#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "%W% %E% %U% JVM"
#endif
/*
 * Copyright 1999-2006 Sun Microsystems, Inc.  All Rights Reserved.
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

//  On i486 the frame looks as follows:
//
//  +-----------------------------+---------+----------------------------------------+----------------+-----------
//  | size_arguments-nof_reg_args | 2 words | size_locals-size_arguments+numreg_args | _size_monitors | spilling .
//  +-----------------------------+---------+----------------------------------------+----------------+-----------
//
//  The FPU registers are mapped with their offset from TOS; therefore the
//  status of FPU stack must be updated during code emission.

 public:
  static const int pd_c_runtime_reserved_arg_size;

  enum {
    nof_xmm_regs = pd_nof_xmm_regs_frame_map,
    nof_caller_save_xmm_regs = pd_nof_caller_save_xmm_regs_frame_map,
    first_available_sp_in_frame = 0,
    frame_pad_in_bytes = 8,
    nof_reg_args = 2
  };

 private:
  static LIR_Opr      _caller_save_xmm_regs [nof_caller_save_xmm_regs];

  static XMMRegister _xmm_regs[nof_xmm_regs];

 public:
  static LIR_Opr receiver_opr;
  
  static LIR_Opr rsi_opr;
  static LIR_Opr rdi_opr;
  static LIR_Opr rbx_opr;
  static LIR_Opr rax_opr;
  static LIR_Opr rdx_opr;
  static LIR_Opr rcx_opr;
  static LIR_Opr rsp_opr;
  static LIR_Opr rbp_opr;

  static LIR_Opr rsi_oop_opr;
  static LIR_Opr rdi_oop_opr;
  static LIR_Opr rbx_oop_opr;
  static LIR_Opr rax_oop_opr;
  static LIR_Opr rdx_oop_opr;
  static LIR_Opr rcx_oop_opr;

  static LIR_Opr rax_rdx_long_opr;
  static LIR_Opr rbx_rcx_long_opr;
  static LIR_Opr fpu0_float_opr;
  static LIR_Opr fpu0_double_opr;
  static LIR_Opr xmm0_float_opr;
  static LIR_Opr xmm0_double_opr;

  static LIR_Opr as_long_opr(Register r, Register r2) {
    return LIR_OprFact::double_cpu(cpu_reg2rnr(r), cpu_reg2rnr(r2));
  }

  // VMReg name for spilled physical FPU stack slot n
  static VMReg fpu_regname (int n);

  static XMMRegister nr2xmmreg(int rnr);

  static bool is_caller_save_register (LIR_Opr opr) { return true; }
  static bool is_caller_save_register (Register r) { return true; }

  static LIR_Opr caller_save_xmm_reg_at(int i) {
    assert(i >= 0 && i < nof_caller_save_xmm_regs, "out of bounds");
    return _caller_save_xmm_regs[i];
  }
