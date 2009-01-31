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

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.*;


public class X86Helper implements CPUHelper {
   public Disassembler createDisassembler(long startPc, byte[] code) {
       return new X86Disassembler(startPc, code);
   }

   public Register getIntegerRegister(int num) {
      return X86Registers.getRegister32(num);
   }

   public Register getFloatRegister(int num) {
      return X86FloatRegisters.getRegister(num);
   }

   public Register getStackPointer() {
      return X86Registers.ESP;
   }

   public Register getFramePointer() {
      return X86Registers.EBP;
   }
}