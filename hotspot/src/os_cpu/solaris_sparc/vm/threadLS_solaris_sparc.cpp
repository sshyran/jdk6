#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_solaris_sparc.cpp	1.16 07/05/05 17:04:53 JVM"
#endif
/*
 * Copyright 1998-2003 Sun Microsystems, Inc.  All Rights Reserved.
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

// Provides an entry point we can link against and
// a buffer we can emit code into. The buffer is
// filled by ThreadLocalStorage::generate_code_for_get_thread
// and called from ThreadLocalStorage::thread()

#include "incls/_precompiled.incl"
#include "incls/_threadLS_solaris_sparc.cpp.incl"
#include <sys/systeminfo.h>

// The portable TLS mechanism (get_thread_via_cache) is enough on SPARC.
// There is no need for hand-assembling a special function.
void ThreadLocalStorage::generate_code_for_get_thread() {
}

void ThreadLocalStorage::set_thread_in_slot (Thread * self) {}

extern "C" Thread* get_thread() {
  return ThreadLocalStorage::thread();
}