#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmThread.hpp	1.40 07/05/05 17:07:03 JVM"
#endif
/*
 * Copyright 1998-2006 Sun Microsystems, Inc.  All Rights Reserved.
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

//
// Prioritized queue of VM operations.
//
// Encapsulates both queue management and
// and priority policy
//
class VMOperationQueue : public CHeapObj {
 private:
  enum Priorities {
     SafepointPriority, // Highest priority (operation executed at a safepoint)
     MediumPriority,    // Medium priority
     nof_priorities 
  };

  // We maintain a doubled linked list, with explicit count.
  int           _queue_length[nof_priorities];
  int           _queue_counter;
  VM_Operation* _queue       [nof_priorities];  
  // we also allow the vmThread to register the ops it has drained so we
  // can scan them from oops_do
  VM_Operation* _drain_list;

  // Double-linked non-empty list insert.
  void insert(VM_Operation* q,VM_Operation* n);
  void unlink(VM_Operation* q);

  // Basic queue manipulation
  bool queue_empty                (int prio);
  void queue_add_front            (int prio, VM_Operation *op);
  void queue_add_back             (int prio, VM_Operation *op);
  VM_Operation* queue_remove_front(int prio);  
  void queue_oops_do(int queue, OopClosure* f);
  void drain_list_oops_do(OopClosure* f);
  VM_Operation* queue_drain(int prio);
  // lock-free query: may return the wrong answer but must not break
  bool queue_peek(int prio) { return _queue_length[prio] > 0; }

 public:
  VMOperationQueue();

  // Highlevel operations. Encapsulates policy
  bool add(VM_Operation *op);
  VM_Operation* remove_next();                        // Returns next or null
  VM_Operation* remove_next_at_safepoint_priority()   { return queue_remove_front(SafepointPriority); }
  VM_Operation* drain_at_safepoint_priority() { return queue_drain(SafepointPriority); }
  void set_drain_list(VM_Operation* list) { _drain_list = list; }
  bool peek_at_safepoint_priority() { return queue_peek(SafepointPriority); }

  // GC support
  void oops_do(OopClosure* f);

  void verify_queue(int prio) PRODUCT_RETURN;
};


//
// A single VMThread (the primordial thread) spawns all other threads
// and is itself used by other threads to offload heavy vm operations
// like scavenge, garbage_collect etc.
//

class VMThread: public Thread {
 private:
  static ThreadPriority _current_priority;

  static bool _should_terminate;
  static bool _terminated;
  static Monitor * _terminate_lock;
  static PerfCounter* _perf_accumulated_vm_operation_time;

  void evaluate_operation(VM_Operation* op);
 public:
  // Constructor
  VMThread();

  // Tester
  bool is_VM_thread() const                      { return true; }
  bool is_GC_thread() const                      { return true; }

  char* name() const { return (char*)"VM Thread"; }
  
  // The ever running loop for the VMThread
  void loop();

  // Called to stop the VM thread
  static void wait_for_vm_thread_exit();
  static bool should_terminate()                  { return _should_terminate; }
  static bool is_terminated()                     { return _terminated == true; }

  // Execution of vm operation
  static void execute(VM_Operation* op);

  // Returns the current vm operation if any.
  static VM_Operation* vm_operation()             { return _cur_vm_operation;   }  

  // Returns the single instance of VMThread.
  static VMThread* vm_thread()                    { return _vm_thread; }

  // GC support
  void oops_do(OopClosure* f);

  // Debugging
  void print_on(outputStream* st) const;
  void print() const				  { print_on(tty); }
  void verify();

  // Performance measurement
  static PerfCounter* perf_accumulated_vm_operation_time()               { return _perf_accumulated_vm_operation_time; }

  // Entry for starting vm thread
  virtual void run();

  // Creations/Destructions
  static void create();
  static void destroy();

 private:   
  // VM_Operation support  
  static VM_Operation*     _cur_vm_operation;   // Current VM operation
  static VMOperationQueue* _vm_queue;           // Queue (w/ policy) of VM operations
  
  // Pointer to single-instance of VM thread
  static VMThread*     _vm_thread;
};

