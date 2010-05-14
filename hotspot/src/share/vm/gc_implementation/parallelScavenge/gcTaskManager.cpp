/*
 * Copyright (c) 2002, 2007, Oracle and/or its affiliates. All rights reserved.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores,
 * CA 94065 USA or visit www.oracle.com if you need additional information or
 * have any questions.
 *
 */

#include "incls/_precompiled.incl"
#include "incls/_gcTaskManager.cpp.incl"

//
// GCTask
//

const char* GCTask::Kind::to_string(kind value) {
  const char* result = "unknown GCTask kind";
  switch (value) {
  default:
    result = "unknown GCTask kind";
    break;
  case unknown_task:
    result = "unknown task";
    break;
  case ordinary_task:
    result = "ordinary task";
    break;
  case barrier_task:
    result = "barrier task";
    break;
  case noop_task:
    result = "noop task";
    break;
  }
  return result;
};

GCTask::GCTask() :
  _kind(Kind::ordinary_task),
  _affinity(GCTaskManager::sentinel_worker()){
  initialize();
}

GCTask::GCTask(Kind::kind kind) :
  _kind(kind),
  _affinity(GCTaskManager::sentinel_worker()) {
  initialize();
}

GCTask::GCTask(uint affinity) :
  _kind(Kind::ordinary_task),
  _affinity(affinity) {
  initialize();
}

GCTask::GCTask(Kind::kind kind, uint affinity) :
  _kind(kind),
  _affinity(affinity) {
  initialize();
}

void GCTask::initialize() {
  _older = NULL;
  _newer = NULL;
}

void GCTask::destruct() {
  assert(older() == NULL, "shouldn't have an older task");
  assert(newer() == NULL, "shouldn't have a newer task");
  // Nothing to do.
}

NOT_PRODUCT(
void GCTask::print(const char* message) const {
  tty->print(INTPTR_FORMAT " <- " INTPTR_FORMAT "(%u) -> " INTPTR_FORMAT,
             newer(), this, affinity(), older());
}
)

//
// GCTaskQueue
//

GCTaskQueue* GCTaskQueue::create() {
  GCTaskQueue* result = new GCTaskQueue(false);
  if (TraceGCTaskQueue) {
    tty->print_cr("GCTaskQueue::create()"
                  " returns " INTPTR_FORMAT, result);
  }
  return result;
}

GCTaskQueue* GCTaskQueue::create_on_c_heap() {
  GCTaskQueue* result = new(ResourceObj::C_HEAP) GCTaskQueue(true);
  if (TraceGCTaskQueue) {
    tty->print_cr("GCTaskQueue::create_on_c_heap()"
                  " returns " INTPTR_FORMAT,
                  result);
  }
  return result;
}

GCTaskQueue::GCTaskQueue(bool on_c_heap) :
  _is_c_heap_obj(on_c_heap) {
  initialize();
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " GCTaskQueue::GCTaskQueue() constructor",
                  this);
  }
}

void GCTaskQueue::destruct() {
  // Nothing to do.
}

void GCTaskQueue::destroy(GCTaskQueue* that) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " GCTaskQueue::destroy()"
                  "  is_c_heap_obj:  %s",
                  that,
                  that->is_c_heap_obj() ? "true" : "false");
  }
  // That instance may have been allocated as a CHeapObj,
  // in which case we have to free it explicitly.
  if (that != NULL) {
    that->destruct();
    assert(that->is_empty(), "should be empty");
    if (that->is_c_heap_obj()) {
      FreeHeap(that);
    }
  }
}

void GCTaskQueue::initialize() {
  set_insert_end(NULL);
  set_remove_end(NULL);
  set_length(0);
}

// Enqueue one task.
void GCTaskQueue::enqueue(GCTask* task) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " GCTaskQueue::enqueue(task: "
                  INTPTR_FORMAT ")",
                  this, task);
    print("before:");
  }
  assert(task != NULL, "shouldn't have null task");
  assert(task->older() == NULL, "shouldn't be on queue");
  assert(task->newer() == NULL, "shouldn't be on queue");
  task->set_newer(NULL);
  task->set_older(insert_end());
  if (is_empty()) {
    set_remove_end(task);
  } else {
    insert_end()->set_newer(task);
  }
  set_insert_end(task);
  increment_length();
  if (TraceGCTaskQueue) {
    print("after:");
  }
}

// Enqueue a whole list of tasks.  Empties the argument list.
void GCTaskQueue::enqueue(GCTaskQueue* list) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " GCTaskQueue::enqueue(list: "
                  INTPTR_FORMAT ")",
                  this);
    print("before:");
    list->print("list:");
  }
  if (list->is_empty()) {
    // Enqueuing the empty list: nothing to do.
    return;
  }
  uint list_length = list->length();
  if (is_empty()) {
    // Enqueuing to empty list: just acquire elements.
    set_insert_end(list->insert_end());
    set_remove_end(list->remove_end());
    set_length(list_length);
  } else {
    // Prepend argument list to our queue.
    list->remove_end()->set_older(insert_end());
    insert_end()->set_newer(list->remove_end());
    set_insert_end(list->insert_end());
    // empty the argument list.
  }
  set_length(length() + list_length);
  list->initialize();
  if (TraceGCTaskQueue) {
    print("after:");
    list->print("list:");
  }
}

// Dequeue one task.
GCTask* GCTaskQueue::dequeue() {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " GCTaskQueue::dequeue()", this);
    print("before:");
  }
  assert(!is_empty(), "shouldn't dequeue from empty list");
  GCTask* result = remove();
  assert(result != NULL, "shouldn't have NULL task");
  if (TraceGCTaskQueue) {
    tty->print_cr("    return: " INTPTR_FORMAT, result);
    print("after:");
  }
  return result;
}

// Dequeue one task, preferring one with affinity.
GCTask* GCTaskQueue::dequeue(uint affinity) {
  if (TraceGCTaskQueue) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " GCTaskQueue::dequeue(%u)", this, affinity);
    print("before:");
  }
  assert(!is_empty(), "shouldn't dequeue from empty list");
  // Look down to the next barrier for a task with this affinity.
  GCTask* result = NULL;
  for (GCTask* element = remove_end();
       element != NULL;
       element = element->newer()) {
    if (element->is_barrier_task()) {
      // Don't consider barrier tasks, nor past them.
      result = NULL;
      break;
    }
    if (element->affinity() == affinity) {
      result = remove(element);
      break;
    }
  }
  // If we didn't find anything with affinity, just take the next task.
  if (result == NULL) {
    result = remove();
  }
  if (TraceGCTaskQueue) {
    tty->print_cr("    return: " INTPTR_FORMAT, result);
    print("after:");
  }
  return result;
}

GCTask* GCTaskQueue::remove() {
  // Dequeue from remove end.
  GCTask* result = remove_end();
  assert(result != NULL, "shouldn't have null task");
  assert(result->older() == NULL, "not the remove_end");
  set_remove_end(result->newer());
  if (remove_end() == NULL) {
    assert(insert_end() == result, "not a singleton");
    set_insert_end(NULL);
  } else {
    remove_end()->set_older(NULL);
  }
  result->set_newer(NULL);
  decrement_length();
  assert(result->newer() == NULL, "shouldn't be on queue");
  assert(result->older() == NULL, "shouldn't be on queue");
  return result;
}

GCTask* GCTaskQueue::remove(GCTask* task) {
  // This is slightly more work, and has slightly fewer asserts
  // than removing from the remove end.
  assert(task != NULL, "shouldn't have null task");
  GCTask* result = task;
  if (result->newer() != NULL) {
    result->newer()->set_older(result->older());
  } else {
    assert(insert_end() == result, "not youngest");
    set_insert_end(result->older());
  }
  if (result->older() != NULL) {
    result->older()->set_newer(result->newer());
  } else {
    assert(remove_end() == result, "not oldest");
    set_remove_end(result->newer());
  }
  result->set_newer(NULL);
  result->set_older(NULL);
  decrement_length();
  return result;
}

NOT_PRODUCT(
void GCTaskQueue::print(const char* message) const {
  tty->print_cr("[" INTPTR_FORMAT "] GCTaskQueue:"
                "  insert_end: " INTPTR_FORMAT
                "  remove_end: " INTPTR_FORMAT
                "  %s",
                this, insert_end(), remove_end(), message);
  for (GCTask* element = insert_end();
       element != NULL;
       element = element->older()) {
    element->print("    ");
    tty->cr();
  }
}
)

//
// SynchronizedGCTaskQueue
//

SynchronizedGCTaskQueue::SynchronizedGCTaskQueue(GCTaskQueue* queue_arg,
                                                 Monitor *       lock_arg) :
  _unsynchronized_queue(queue_arg),
  _lock(lock_arg) {
  assert(unsynchronized_queue() != NULL, "null queue");
  assert(lock() != NULL, "null lock");
}

SynchronizedGCTaskQueue::~SynchronizedGCTaskQueue() {
  // Nothing to do.
}

//
// GCTaskManager
//
GCTaskManager::GCTaskManager(uint workers) :
  _workers(workers),
  _ndc(NULL) {
  initialize();
}

GCTaskManager::GCTaskManager(uint workers, NotifyDoneClosure* ndc) :
  _workers(workers),
  _ndc(ndc) {
  initialize();
}

void GCTaskManager::initialize() {
  if (TraceGCTaskManager) {
    tty->print_cr("GCTaskManager::initialize: workers: %u", workers());
  }
  assert(workers() != 0, "no workers");
  _monitor = new Monitor(Mutex::barrier,                // rank
                         "GCTaskManager monitor",       // name
                         Mutex::_allow_vm_block_flag);  // allow_vm_block
  // The queue for the GCTaskManager must be a CHeapObj.
  GCTaskQueue* unsynchronized_queue = GCTaskQueue::create_on_c_heap();
  _queue = SynchronizedGCTaskQueue::create(unsynchronized_queue, lock());
  _noop_task = NoopGCTask::create_on_c_heap();
  _resource_flag = NEW_C_HEAP_ARRAY(bool, workers());
  {
    // Set up worker threads.
    //     Distribute the workers among the available processors,
    //     unless we were told not to, or if the os doesn't want to.
    uint* processor_assignment = NEW_C_HEAP_ARRAY(uint, workers());
    if (!BindGCTaskThreadsToCPUs ||
        !os::distribute_processes(workers(), processor_assignment)) {
      for (uint a = 0; a < workers(); a += 1) {
        processor_assignment[a] = sentinel_worker();
      }
    }
    _thread = NEW_C_HEAP_ARRAY(GCTaskThread*, workers());
    for (uint t = 0; t < workers(); t += 1) {
      set_thread(t, GCTaskThread::create(this, t, processor_assignment[t]));
    }
    if (TraceGCTaskThread) {
      tty->print("GCTaskManager::initialize: distribution:");
      for (uint t = 0; t < workers(); t += 1) {
        tty->print("  %u", processor_assignment[t]);
      }
      tty->cr();
    }
    FREE_C_HEAP_ARRAY(uint, processor_assignment);
  }
  reset_busy_workers();
  set_unblocked();
  for (uint w = 0; w < workers(); w += 1) {
    set_resource_flag(w, false);
  }
  reset_delivered_tasks();
  reset_completed_tasks();
  reset_noop_tasks();
  reset_barriers();
  reset_emptied_queue();
  for (uint s = 0; s < workers(); s += 1) {
    thread(s)->start();
  }
}

GCTaskManager::~GCTaskManager() {
  assert(busy_workers() == 0, "still have busy workers");
  assert(queue()->is_empty(), "still have queued work");
  NoopGCTask::destroy(_noop_task);
  _noop_task = NULL;
  if (_thread != NULL) {
    for (uint i = 0; i < workers(); i += 1) {
      GCTaskThread::destroy(thread(i));
      set_thread(i, NULL);
    }
    FREE_C_HEAP_ARRAY(GCTaskThread*, _thread);
    _thread = NULL;
  }
  if (_resource_flag != NULL) {
    FREE_C_HEAP_ARRAY(bool, _resource_flag);
    _resource_flag = NULL;
  }
  if (queue() != NULL) {
    GCTaskQueue* unsynchronized_queue = queue()->unsynchronized_queue();
    GCTaskQueue::destroy(unsynchronized_queue);
    SynchronizedGCTaskQueue::destroy(queue());
    _queue = NULL;
  }
  if (monitor() != NULL) {
    delete monitor();
    _monitor = NULL;
  }
}

void GCTaskManager::print_task_time_stamps() {
  for(uint i=0; i<ParallelGCThreads; i++) {
    GCTaskThread* t = thread(i);
    t->print_task_time_stamps();
  }
}

void GCTaskManager::print_threads_on(outputStream* st) {
  uint num_thr = workers();
  for (uint i = 0; i < num_thr; i++) {
    thread(i)->print_on(st);
    st->cr();
  }
}

void GCTaskManager::threads_do(ThreadClosure* tc) {
  assert(tc != NULL, "Null ThreadClosure");
  uint num_thr = workers();
  for (uint i = 0; i < num_thr; i++) {
    tc->do_thread(thread(i));
  }
}

GCTaskThread* GCTaskManager::thread(uint which) {
  assert(which < workers(), "index out of bounds");
  assert(_thread[which] != NULL, "shouldn't have null thread");
  return _thread[which];
}

void GCTaskManager::set_thread(uint which, GCTaskThread* value) {
  assert(which < workers(), "index out of bounds");
  assert(value != NULL, "shouldn't have null thread");
  _thread[which] = value;
}

void GCTaskManager::add_task(GCTask* task) {
  assert(task != NULL, "shouldn't have null task");
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  if (TraceGCTaskManager) {
    tty->print_cr("GCTaskManager::add_task(" INTPTR_FORMAT " [%s])",
                  task, GCTask::Kind::to_string(task->kind()));
  }
  queue()->enqueue(task);
  // Notify with the lock held to avoid missed notifies.
  if (TraceGCTaskManager) {
    tty->print_cr("    GCTaskManager::add_task (%s)->notify_all",
                  monitor()->name());
  }
  (void) monitor()->notify_all();
  // Release monitor().
}

void GCTaskManager::add_list(GCTaskQueue* list) {
  assert(list != NULL, "shouldn't have null task");
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  if (TraceGCTaskManager) {
    tty->print_cr("GCTaskManager::add_list(%u)", list->length());
  }
  queue()->enqueue(list);
  // Notify with the lock held to avoid missed notifies.
  if (TraceGCTaskManager) {
    tty->print_cr("    GCTaskManager::add_list (%s)->notify_all",
                  monitor()->name());
  }
  (void) monitor()->notify_all();
  // Release monitor().
}

GCTask* GCTaskManager::get_task(uint which) {
  GCTask* result = NULL;
  // Grab the queue lock.
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  // Wait while the queue is block or
  // there is nothing to do, except maybe release resources.
  while (is_blocked() ||
         (queue()->is_empty() && !should_release_resources(which))) {
    if (TraceGCTaskManager) {
      tty->print_cr("GCTaskManager::get_task(%u)"
                    "  blocked: %s"
                    "  empty: %s"
                    "  release: %s",
                    which,
                    is_blocked() ? "true" : "false",
                    queue()->is_empty() ? "true" : "false",
                    should_release_resources(which) ? "true" : "false");
      tty->print_cr("    => (%s)->wait()",
                    monitor()->name());
    }
    monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
  }
  // We've reacquired the queue lock here.
  // Figure out which condition caused us to exit the loop above.
  if (!queue()->is_empty()) {
    if (UseGCTaskAffinity) {
      result = queue()->dequeue(which);
    } else {
      result = queue()->dequeue();
    }
    if (result->is_barrier_task()) {
      assert(which != sentinel_worker(),
             "blocker shouldn't be bogus");
      set_blocking_worker(which);
    }
  } else {
    // The queue is empty, but we were woken up.
    // Just hand back a Noop task,
    // in case someone wanted us to release resources, or whatever.
    result = noop_task();
    increment_noop_tasks();
  }
  assert(result != NULL, "shouldn't have null task");
  if (TraceGCTaskManager) {
    tty->print_cr("GCTaskManager::get_task(%u) => " INTPTR_FORMAT " [%s]",
                  which, result, GCTask::Kind::to_string(result->kind()));
    tty->print_cr("     %s", result->name());
  }
  increment_busy_workers();
  increment_delivered_tasks();
  return result;
  // Release monitor().
}

void GCTaskManager::note_completion(uint which) {
  MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
  if (TraceGCTaskManager) {
    tty->print_cr("GCTaskManager::note_completion(%u)", which);
  }
  // If we are blocked, check if the completing thread is the blocker.
  if (blocking_worker() == which) {
    assert(blocking_worker() != sentinel_worker(),
           "blocker shouldn't be bogus");
    increment_barriers();
    set_unblocked();
  }
  increment_completed_tasks();
  uint active = decrement_busy_workers();
  if ((active == 0) && (queue()->is_empty())) {
    increment_emptied_queue();
    if (TraceGCTaskManager) {
      tty->print_cr("    GCTaskManager::note_completion(%u) done", which);
    }
    // Notify client that we are done.
    NotifyDoneClosure* ndc = notify_done_closure();
    if (ndc != NULL) {
      ndc->notify(this);
    }
  }
  if (TraceGCTaskManager) {
    tty->print_cr("    GCTaskManager::note_completion(%u) (%s)->notify_all",
                  which, monitor()->name());
    tty->print_cr("  "
                  "  blocked: %s"
                  "  empty: %s"
                  "  release: %s",
                  is_blocked() ? "true" : "false",
                  queue()->is_empty() ? "true" : "false",
                  should_release_resources(which) ? "true" : "false");
    tty->print_cr("  "
                  "  delivered: %u"
                  "  completed: %u"
                  "  barriers: %u"
                  "  emptied: %u",
                  delivered_tasks(),
                  completed_tasks(),
                  barriers(),
                  emptied_queue());
  }
  // Tell everyone that a task has completed.
  (void) monitor()->notify_all();
  // Release monitor().
}

uint GCTaskManager::increment_busy_workers() {
  assert(queue()->own_lock(), "don't own the lock");
  _busy_workers += 1;
  return _busy_workers;
}

uint GCTaskManager::decrement_busy_workers() {
  assert(queue()->own_lock(), "don't own the lock");
  _busy_workers -= 1;
  return _busy_workers;
}

void GCTaskManager::release_all_resources() {
  // If you want this to be done atomically, do it in a BarrierGCTask.
  for (uint i = 0; i < workers(); i += 1) {
    set_resource_flag(i, true);
  }
}

bool GCTaskManager::should_release_resources(uint which) {
  // This can be done without a lock because each thread reads one element.
  return resource_flag(which);
}

void GCTaskManager::note_release(uint which) {
  // This can be done without a lock because each thread writes one element.
  set_resource_flag(which, false);
}

void GCTaskManager::execute_and_wait(GCTaskQueue* list) {
  WaitForBarrierGCTask* fin = WaitForBarrierGCTask::create();
  list->enqueue(fin);
  add_list(list);
  fin->wait_for();
  // We have to release the barrier tasks!
  WaitForBarrierGCTask::destroy(fin);
}

bool GCTaskManager::resource_flag(uint which) {
  assert(which < workers(), "index out of bounds");
  return _resource_flag[which];
}

void GCTaskManager::set_resource_flag(uint which, bool value) {
  assert(which < workers(), "index out of bounds");
  _resource_flag[which] = value;
}

//
// NoopGCTask
//

NoopGCTask* NoopGCTask::create() {
  NoopGCTask* result = new NoopGCTask(false);
  return result;
}

NoopGCTask* NoopGCTask::create_on_c_heap() {
  NoopGCTask* result = new(ResourceObj::C_HEAP) NoopGCTask(true);
  return result;
}

void NoopGCTask::destroy(NoopGCTask* that) {
  if (that != NULL) {
    that->destruct();
    if (that->is_c_heap_obj()) {
      FreeHeap(that);
    }
  }
}

void NoopGCTask::destruct() {
  // This has to know it's superclass structure, just like the constructor.
  this->GCTask::destruct();
  // Nothing else to do.
}

//
// BarrierGCTask
//

void BarrierGCTask::do_it(GCTaskManager* manager, uint which) {
  // Wait for this to be the only busy worker.
  // ??? I thought of having a StackObj class
  //     whose constructor would grab the lock and come to the barrier,
  //     and whose destructor would release the lock,
  //     but that seems like too much mechanism for two lines of code.
  MutexLockerEx ml(manager->lock(), Mutex::_no_safepoint_check_flag);
  do_it_internal(manager, which);
  // Release manager->lock().
}

void BarrierGCTask::do_it_internal(GCTaskManager* manager, uint which) {
  // Wait for this to be the only busy worker.
  assert(manager->monitor()->owned_by_self(), "don't own the lock");
  assert(manager->is_blocked(), "manager isn't blocked");
  while (manager->busy_workers() > 1) {
    if (TraceGCTaskManager) {
      tty->print_cr("BarrierGCTask::do_it(%u) waiting on %u workers",
                    which, manager->busy_workers());
    }
    manager->monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
  }
}

void BarrierGCTask::destruct() {
  this->GCTask::destruct();
  // Nothing else to do.
}

//
// ReleasingBarrierGCTask
//

void ReleasingBarrierGCTask::do_it(GCTaskManager* manager, uint which) {
  MutexLockerEx ml(manager->lock(), Mutex::_no_safepoint_check_flag);
  do_it_internal(manager, which);
  manager->release_all_resources();
  // Release manager->lock().
}

void ReleasingBarrierGCTask::destruct() {
  this->BarrierGCTask::destruct();
  // Nothing else to do.
}

//
// NotifyingBarrierGCTask
//

void NotifyingBarrierGCTask::do_it(GCTaskManager* manager, uint which) {
  MutexLockerEx ml(manager->lock(), Mutex::_no_safepoint_check_flag);
  do_it_internal(manager, which);
  NotifyDoneClosure* ndc = notify_done_closure();
  if (ndc != NULL) {
    ndc->notify(manager);
  }
  // Release manager->lock().
}

void NotifyingBarrierGCTask::destruct() {
  this->BarrierGCTask::destruct();
  // Nothing else to do.
}

//
// WaitForBarrierGCTask
//
WaitForBarrierGCTask* WaitForBarrierGCTask::create() {
  WaitForBarrierGCTask* result = new WaitForBarrierGCTask(false);
  return result;
}

WaitForBarrierGCTask* WaitForBarrierGCTask::create_on_c_heap() {
  WaitForBarrierGCTask* result = new WaitForBarrierGCTask(true);
  return result;
}

WaitForBarrierGCTask::WaitForBarrierGCTask(bool on_c_heap) :
  _is_c_heap_obj(on_c_heap) {
  _monitor = MonitorSupply::reserve();
  set_should_wait(true);
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " WaitForBarrierGCTask::WaitForBarrierGCTask()"
                  "  monitor: " INTPTR_FORMAT,
                  this, monitor());
  }
}

void WaitForBarrierGCTask::destroy(WaitForBarrierGCTask* that) {
  if (that != NULL) {
    if (TraceGCTaskManager) {
      tty->print_cr("[" INTPTR_FORMAT "]"
                    " WaitForBarrierGCTask::destroy()"
                    "  is_c_heap_obj: %s"
                    "  monitor: " INTPTR_FORMAT,
                    that,
                    that->is_c_heap_obj() ? "true" : "false",
                    that->monitor());
    }
    that->destruct();
    if (that->is_c_heap_obj()) {
      FreeHeap(that);
    }
  }
}

void WaitForBarrierGCTask::destruct() {
  assert(monitor() != NULL, "monitor should not be NULL");
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " WaitForBarrierGCTask::destruct()"
                  "  monitor: " INTPTR_FORMAT,
                  this, monitor());
  }
  this->BarrierGCTask::destruct();
  // Clean up that should be in the destructor,
  // except that ResourceMarks don't call destructors.
   if (monitor() != NULL) {
     MonitorSupply::release(monitor());
  }
  _monitor = (Monitor*) 0xDEAD000F;
}

void WaitForBarrierGCTask::do_it(GCTaskManager* manager, uint which) {
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " WaitForBarrierGCTask::do_it() waiting for idle"
                  "  monitor: " INTPTR_FORMAT,
                  this, monitor());
  }
  {
    // First, wait for the barrier to arrive.
    MutexLockerEx ml(manager->lock(), Mutex::_no_safepoint_check_flag);
    do_it_internal(manager, which);
    // Release manager->lock().
  }
  {
    // Then notify the waiter.
    MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
    set_should_wait(false);
    // Waiter doesn't miss the notify in the wait_for method
    // since it checks the flag after grabbing the monitor.
    if (TraceGCTaskManager) {
      tty->print_cr("[" INTPTR_FORMAT "]"
                    " WaitForBarrierGCTask::do_it()"
                    "  [" INTPTR_FORMAT "] (%s)->notify_all()",
                    this, monitor(), monitor()->name());
    }
    monitor()->notify_all();
    // Release monitor().
  }
}

void WaitForBarrierGCTask::wait_for() {
  if (TraceGCTaskManager) {
    tty->print_cr("[" INTPTR_FORMAT "]"
                  " WaitForBarrierGCTask::wait_for()"
      "  should_wait: %s",
      this, should_wait() ? "true" : "false");
  }
  {
    // Grab the lock and check again.
    MutexLockerEx ml(monitor(), Mutex::_no_safepoint_check_flag);
    while (should_wait()) {
      if (TraceGCTaskManager) {
        tty->print_cr("[" INTPTR_FORMAT "]"
                      " WaitForBarrierGCTask::wait_for()"
          "  [" INTPTR_FORMAT "] (%s)->wait()",
          this, monitor(), monitor()->name());
      }
      monitor()->wait(Mutex::_no_safepoint_check_flag, 0);
    }
    // Reset the flag in case someone reuses this task.
    set_should_wait(true);
    if (TraceGCTaskManager) {
      tty->print_cr("[" INTPTR_FORMAT "]"
                    " WaitForBarrierGCTask::wait_for() returns"
        "  should_wait: %s",
        this, should_wait() ? "true" : "false");
    }
    // Release monitor().
  }
}

Mutex*                   MonitorSupply::_lock     = NULL;
GrowableArray<Monitor*>* MonitorSupply::_freelist = NULL;

Monitor* MonitorSupply::reserve() {
  Monitor* result = NULL;
  // Lazy initialization: possible race.
  if (lock() == NULL) {
    _lock = new Mutex(Mutex::barrier,                  // rank
                      "MonitorSupply mutex",           // name
                      Mutex::_allow_vm_block_flag);    // allow_vm_block
  }
  {
    MutexLockerEx ml(lock());
    // Lazy initialization.
    if (freelist() == NULL) {
      _freelist =
        new(ResourceObj::C_HEAP) GrowableArray<Monitor*>(ParallelGCThreads,
                                                         true);
    }
    if (! freelist()->is_empty()) {
      result = freelist()->pop();
    } else {
      result = new Monitor(Mutex::barrier,                  // rank
                           "MonitorSupply monitor",         // name
                           Mutex::_allow_vm_block_flag);    // allow_vm_block
    }
    guarantee(result != NULL, "shouldn't return NULL");
    assert(!result->is_locked(), "shouldn't be locked");
    // release lock().
  }
  return result;
}

void MonitorSupply::release(Monitor* instance) {
  assert(instance != NULL, "shouldn't release NULL");
  assert(!instance->is_locked(), "shouldn't be locked");
  {
    MutexLockerEx ml(lock());
    freelist()->push(instance);
    // release lock().
  }
}
