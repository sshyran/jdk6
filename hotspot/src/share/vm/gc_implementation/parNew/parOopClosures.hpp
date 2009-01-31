#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)parOopClosures.hpp	1.1 07/05/16 10:51:44 JVM"
#endif
/*
 * Copyright (c) 2007 Sun Microsystems, Inc.  All Rights Reserved.
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

// Closures for ParNewGeneration

class ParScanThreadState;
class ParNewGeneration;
typedef class OopTaskQueueSet ObjToScanQueueSet;
class ParallelTaskTerminator;

class ParScanClosure: public OopsInGenClosure {
protected:
  ParScanThreadState* _par_scan_state;
  ParNewGeneration* _g;
  HeapWord* _boundary;
  void do_oop_work(oop* p,
			  bool gc_barrier,
			  bool root_scan);

  void par_do_barrier(oop* p);

public:
  ParScanClosure(ParNewGeneration* g, ParScanThreadState* par_scan_state);
};

class ParScanWithBarrierClosure: public ParScanClosure {
public:
  void do_oop(oop* p)    { do_oop_work(p, true, false); }
  void do_oop_nv(oop* p) { do_oop_work(p, true, false); }
  ParScanWithBarrierClosure(ParNewGeneration* g,
			    ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
};

class ParScanWithoutBarrierClosure: public ParScanClosure {
public:
  ParScanWithoutBarrierClosure(ParNewGeneration* g,
			       ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p)    { do_oop_work(p, false, false); }
  void do_oop_nv(oop* p) { do_oop_work(p, false, false); }
};

class ParRootScanWithBarrierTwoGensClosure: public ParScanClosure {
public:
  ParRootScanWithBarrierTwoGensClosure(ParNewGeneration* g,
				       ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p) { do_oop_work(p, true, true); }
};

class ParRootScanWithoutBarrierClosure: public ParScanClosure {
public:
  ParRootScanWithoutBarrierClosure(ParNewGeneration* g,
				   ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p) { do_oop_work(p, false, true); }
};

class ParScanWeakRefClosure: public ScanWeakRefClosure {
protected:
  ParScanThreadState* _par_scan_state;
public:
  ParScanWeakRefClosure(ParNewGeneration* g,
                        ParScanThreadState* par_scan_state);
  void do_oop(oop* p);
  void do_oop_nv(oop* p);
};

class ParEvacuateFollowersClosure: public VoidClosure {
  ParScanThreadState* _par_scan_state;
  ParScanThreadState* par_scan_state() { return _par_scan_state; }

  // We want to preserve the specific types here (rather than "OopClosure") 
  // for later de-virtualization of do_oop calls.
  ParScanWithoutBarrierClosure* _to_space_closure;
  ParScanWithoutBarrierClosure* to_space_closure() {
    return _to_space_closure;
  }
  ParRootScanWithoutBarrierClosure* _to_space_root_closure;
  ParRootScanWithoutBarrierClosure* to_space_root_closure() {
    return _to_space_root_closure;
  }

  ParScanWithBarrierClosure* _old_gen_closure;
  ParScanWithBarrierClosure* old_gen_closure () {
    return _old_gen_closure;
  }
  ParRootScanWithBarrierTwoGensClosure* _old_gen_root_closure;
  ParRootScanWithBarrierTwoGensClosure* old_gen_root_closure () {
    return _old_gen_root_closure;
  }

  ParNewGeneration* _par_gen;
  ParNewGeneration* par_gen() { return _par_gen; }
  
  ObjToScanQueueSet*  _task_queues;
  ObjToScanQueueSet*  task_queues() { return _task_queues; }

  ParallelTaskTerminator* _terminator;
  ParallelTaskTerminator* terminator() { return _terminator; }

public:
  ParEvacuateFollowersClosure(
    ParScanThreadState* par_scan_state_,
    ParScanWithoutBarrierClosure* to_space_closure_,
    ParScanWithBarrierClosure* old_gen_closure_,
    ParRootScanWithoutBarrierClosure* to_space_root_closure_,
    ParNewGeneration* par_gen_,
    ParRootScanWithBarrierTwoGensClosure* old_gen_root_closure_,
    ObjToScanQueueSet* task_queues_,
    ParallelTaskTerminator* terminator_);
  void do_void();
};