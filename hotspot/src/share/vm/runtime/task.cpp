#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)task.cpp	1.27 07/05/05 17:06:59 JVM"
#endif
/*
 * Copyright 1997-2007 Sun Microsystems, Inc.  All Rights Reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_task.cpp.incl"

int PeriodicTask::_num_tasks = 0;
PeriodicTask* PeriodicTask::_tasks[PeriodicTask::max_tasks];
#ifndef PRODUCT
elapsedTimer PeriodicTask::_timer;
int PeriodicTask::_intervalHistogram[PeriodicTask::max_interval];
int PeriodicTask::_ticks;

void PeriodicTask::print_intervals() {
  if (ProfilerCheckIntervals) {
    for (int i = 0; i < PeriodicTask::max_interval; i++) {
      int n = _intervalHistogram[i];
      if (n > 0) tty->print_cr("%3d: %5d (%4.1f%%)", i, n, 100.0 * n / _ticks);
    }
  }
}
#endif

void PeriodicTask::real_time_tick(size_t delay_time) {
#ifndef PRODUCT
  if (ProfilerCheckIntervals) {
    _ticks++;
    _timer.stop();
    int ms = (int)(_timer.seconds() * 1000.0);
    _timer.reset();
    _timer.start();
    if (ms >= PeriodicTask::max_interval) ms = PeriodicTask::max_interval - 1;
    _intervalHistogram[ms]++;
  }
#endif
  int orig_num_tasks = _num_tasks;
  for(int index = 0; index < _num_tasks; index++) {
    _tasks[index]->execute_if_pending(delay_time);
    if (_num_tasks < orig_num_tasks) { // task dis-enrolled itself
      index--;  // re-do current slot as it has changed
      orig_num_tasks = _num_tasks;
    }
  }
}


PeriodicTask::PeriodicTask(size_t interval_time) :
  _counter(0), _interval(interval_time) {
  assert(is_init_completed(), "Periodic tasks should not start during VM initialization");
  // Sanity check the interval time
  assert(_interval >= PeriodicTask::min_interval &&
         _interval <= PeriodicTask::max_interval &&
         _interval %  PeriodicTask::interval_gran == 0,
              "improper PeriodicTask interval time");
}

PeriodicTask::~PeriodicTask() {
  if (is_enrolled())
    disenroll();
}

bool PeriodicTask::is_enrolled() const {
  for(int index = 0; index < _num_tasks; index++) 
    if (_tasks[index] == this) return true;
  return false;
}

void PeriodicTask::enroll() {
  assert(WatcherThread::watcher_thread() == NULL, "dynamic enrollment of tasks not yet supported");

  if (_num_tasks == PeriodicTask::max_tasks)
    fatal("Overflow in PeriodicTask table");
  _tasks[_num_tasks++] = this;
}

void PeriodicTask::disenroll() {
  assert(WatcherThread::watcher_thread() == NULL ||
         Thread::current() == WatcherThread::watcher_thread(),
         "dynamic disenrollment currently only handled from WatcherThread from within task() method");

  int index;
  for(index = 0; index < _num_tasks && _tasks[index] != this; index++);
  if (index == _num_tasks) return;
  _num_tasks--;
  for (; index < _num_tasks; index++) {
    _tasks[index] = _tasks[index+1];
  }
}

TimeMillisUpdateTask* TimeMillisUpdateTask::_task = NULL;

void TimeMillisUpdateTask::task() {
  os::update_global_time();
}

void TimeMillisUpdateTask::engage() {
  assert(_task == NULL, "init twice?");
  os::update_global_time(); // initial update
  os::enable_global_time();
  _task = new TimeMillisUpdateTask(CacheTimeMillisGranularity);
  _task->enroll();
}

void TimeMillisUpdateTask::disengage() {
  assert(_task != NULL, "uninit twice?");
  os::disable_global_time();
  _task->disenroll();
  delete _task;
  _task = NULL;
}
