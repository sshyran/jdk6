#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cInterpreter.inline.hpp	1.9 07/05/05 17:05:37 JVM"
#endif
/*
 * Copyright 2002 Sun Microsystems, Inc.  All Rights Reserved.
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

// This file holds platform-independant bodies of inline functions for the C++ based interpreter

#ifdef CC_INTERP

#ifdef ASSERT
extern "C" { typedef void (*verify_oop_fn_t)(oop, const char *);};
#define VERIFY_OOP(o) \
	/*{ verify_oop_fn_t verify_oop_entry = \
            *StubRoutines::verify_oop_subroutine_entry_address(); \
          if (verify_oop_entry) { \
	     (*verify_oop_entry)((o), "Not an oop!"); \
	  } \
	}*/
#else
#define VERIFY_OOP(o)
#endif

// Platform dependent data manipulation
# include "incls/_cInterpreter_pd.inline.hpp.incl"
#endif // CC_INTERP