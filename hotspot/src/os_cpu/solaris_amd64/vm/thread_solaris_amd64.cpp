#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_solaris_amd64.cpp	1.8 07/05/05 17:04:51 JVM"
#endif
/*
 * Copyright 2004-2005 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *  
 */

#include "incls/_precompiled.incl"
#include "incls/_thread_solaris_amd64.cpp.incl"

// For Forte Analyzer AsyncGetCallTrace profiling support - thread is
// currently interrupted by SIGPROF
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
  void* ucontext, bool isInJava) {

  assert(Thread::current() == this, "caller must be current thread");
  assert(this->is_Java_thread(), "must be JavaThread");

  JavaThread* jt = (JavaThread *)this;

  // If we have a last_Java_frame, then we should use it even if
  // isInJava == true.  It should be more reliable than ucontext info.
  if (jt->has_last_Java_frame()) {
    *fr_addr = jt->pd_last_frame();
    return true;
  }

  // At this point, we don't have a last_Java_frame, so
  // we try to glean some information out of the ucontext
  // if we were running Java code when SIGPROF came in.
  if (isInJava) {
    ucontext_t* uc = (ucontext_t*) ucontext;

    intptr_t* ret_fp;
    intptr_t* ret_sp;
    ExtendedPC addr = os::Solaris::fetch_frame_from_ucontext(this, uc,
      &ret_sp, &ret_fp);
    if (addr.pc() == NULL || ret_sp == NULL ) {
      // ucontext wasn't useful
      return false;
    }

    frame ret_frame(ret_sp, ret_fp, addr.pc());
    if (!ret_frame.safe_for_sender(jt)) {
#ifdef COMPILER2
      // C2 uses ebp as a general register see if NULL fp helps
      frame ret_frame2(ret_sp, NULL, addr.pc());
      if (!ret_frame2.safe_for_sender(jt)) {
        // nothing else to try if the frame isn't good
        return false;
      }
      ret_frame = ret_frame2;
#else
      // nothing else to try if the frame isn't good
      return false;
#endif /* COMPILER2 */
    }
    *fr_addr = ret_frame;
    return true;
  }

  // nothing else to try
  return false;
}