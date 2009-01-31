#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)chaitin_solaris.cpp	1.17 07/05/05 17:04:40 JVM"
#endif
/*
 * Copyright 1999-2001 Sun Microsystems, Inc.  All Rights Reserved.
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
#include "incls/_chaitin_solaris.cpp.incl"

void PhaseRegAlloc::pd_preallocate_hook() {
  // no action
}

#ifdef ASSERT
void PhaseRegAlloc::pd_postallocate_verify_hook() {
  // no action
}
#endif


//Reconciliation History
// 1.1 99/02/12 15:35:26 chaitin_win32.cpp
// 1.2 99/02/18 15:38:56 chaitin_win32.cpp
// 1.4 99/03/09 10:37:48 chaitin_win32.cpp
// 1.6 99/03/25 11:07:44 chaitin_win32.cpp
// 1.8 99/06/22 16:38:58 chaitin_win32.cpp
//End