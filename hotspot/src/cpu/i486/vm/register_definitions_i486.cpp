#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "%W% %E% %U% JVM"
#endif
/*
 * Copyright 2002-2005 Sun Microsystems, Inc.  All Rights Reserved.
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

// make sure the defines don't screw up the declarations later on in this file
#define DONT_USE_REGISTER_DEFINES

#include "incls/_precompiled.incl"
#ifndef AMD64
#include "incls/_register_definitions_i486.cpp.incl"
#else
#include "incls/_register_definitions_amd64.cpp.incl"
#endif // AMD64

REGISTER_DEFINITION(Register, noreg);
REGISTER_DEFINITION(Register, rax);
REGISTER_DEFINITION(Register, rcx);
REGISTER_DEFINITION(Register, rdx);
REGISTER_DEFINITION(Register, rbx);
REGISTER_DEFINITION(Register, rsp);
REGISTER_DEFINITION(Register, rbp);
REGISTER_DEFINITION(Register, rsi);
REGISTER_DEFINITION(Register, rdi);
#ifdef AMD64
REGISTER_DEFINITION(Register, r8);
REGISTER_DEFINITION(Register, r9);
REGISTER_DEFINITION(Register, r10);
REGISTER_DEFINITION(Register, r11);
REGISTER_DEFINITION(Register, r12);
REGISTER_DEFINITION(Register, r13);
REGISTER_DEFINITION(Register, r14);
REGISTER_DEFINITION(Register, r15);
#endif // AMD64

REGISTER_DEFINITION(XMMRegister, xmm0 );
REGISTER_DEFINITION(XMMRegister, xmm1 );
REGISTER_DEFINITION(XMMRegister, xmm2 );
REGISTER_DEFINITION(XMMRegister, xmm3 );
REGISTER_DEFINITION(XMMRegister, xmm4 );
REGISTER_DEFINITION(XMMRegister, xmm5 );
REGISTER_DEFINITION(XMMRegister, xmm6 );
REGISTER_DEFINITION(XMMRegister, xmm7 );
#ifdef AMD64
REGISTER_DEFINITION(XMMRegister, xmm8);
REGISTER_DEFINITION(XMMRegister, xmm9);
REGISTER_DEFINITION(XMMRegister, xmm10);
REGISTER_DEFINITION(XMMRegister, xmm11);
REGISTER_DEFINITION(XMMRegister, xmm12);
REGISTER_DEFINITION(XMMRegister, xmm13);
REGISTER_DEFINITION(XMMRegister, xmm14);
REGISTER_DEFINITION(XMMRegister, xmm15);

REGISTER_DEFINITION(Register, c_rarg0);
REGISTER_DEFINITION(Register, c_rarg1);
REGISTER_DEFINITION(Register, c_rarg2);
REGISTER_DEFINITION(Register, c_rarg3);

REGISTER_DEFINITION(XMMRegister, c_farg0);
REGISTER_DEFINITION(XMMRegister, c_farg1);
REGISTER_DEFINITION(XMMRegister, c_farg2);
REGISTER_DEFINITION(XMMRegister, c_farg3);

// Non windows OS's have a few more argument registers
#ifndef _WIN64
REGISTER_DEFINITION(Register, c_rarg4);
REGISTER_DEFINITION(Register, c_rarg5);

REGISTER_DEFINITION(XMMRegister, c_farg4);
REGISTER_DEFINITION(XMMRegister, c_farg5);
REGISTER_DEFINITION(XMMRegister, c_farg6);
REGISTER_DEFINITION(XMMRegister, c_farg7);
#endif /* _WIN64 */

REGISTER_DEFINITION(Register, j_rarg0);
REGISTER_DEFINITION(Register, j_rarg1);
REGISTER_DEFINITION(Register, j_rarg2);
REGISTER_DEFINITION(Register, j_rarg3);
REGISTER_DEFINITION(Register, j_rarg4);
REGISTER_DEFINITION(Register, j_rarg5);

REGISTER_DEFINITION(XMMRegister, j_farg0);
REGISTER_DEFINITION(XMMRegister, j_farg1);
REGISTER_DEFINITION(XMMRegister, j_farg2);
REGISTER_DEFINITION(XMMRegister, j_farg3);
REGISTER_DEFINITION(XMMRegister, j_farg4);
REGISTER_DEFINITION(XMMRegister, j_farg5);
REGISTER_DEFINITION(XMMRegister, j_farg6);
REGISTER_DEFINITION(XMMRegister, j_farg7);

REGISTER_DEFINITION(Register, rscratch1);
REGISTER_DEFINITION(Register, rscratch2);

REGISTER_DEFINITION(Register, r15_thread);
#endif // AMD64

REGISTER_DEFINITION(MMXRegister, mmx0 );
REGISTER_DEFINITION(MMXRegister, mmx1 );
REGISTER_DEFINITION(MMXRegister, mmx2 );
REGISTER_DEFINITION(MMXRegister, mmx3 );
REGISTER_DEFINITION(MMXRegister, mmx4 );
REGISTER_DEFINITION(MMXRegister, mmx5 );
REGISTER_DEFINITION(MMXRegister, mmx6 );
REGISTER_DEFINITION(MMXRegister, mmx7 );
