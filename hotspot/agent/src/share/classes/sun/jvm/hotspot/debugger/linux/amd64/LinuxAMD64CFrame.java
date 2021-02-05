/*
 * Copyright 2003 Sun Microsystems, Inc.  All Rights Reserved.
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

package sun.jvm.hotspot.debugger.linux.amd64;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.linux.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.cdbg.basic.*;

final public class LinuxAMD64CFrame extends BasicCFrame {
   public LinuxAMD64CFrame(LinuxDebugger dbg, Address rbp, Address rip) {
      super(dbg.getCDebugger()); 
      this.rbp = rbp;  
      this.rip = rip;
      this.dbg = dbg;
   }

   // override base class impl to avoid ELF parsing
   public ClosestSymbol closestSymbolToPC() {
      // try native lookup in debugger.
      return dbg.lookup(dbg.getAddressValue(pc()));
   }

   public Address pc() {
      return rip;
   }

   public Address localVariableBase() {
      return rbp;
   }

   public CFrame sender() {
      if (rbp == null) {
        return null;
      }

      Address nextRBP = rbp.getAddressAt( 0 * ADDRESS_SIZE);
      if (nextRBP == null) {
        return null;
      }
      Address nextPC  = rbp.getAddressAt( 1 * ADDRESS_SIZE);
      if (nextPC == null) {
        return null;
      }
      return new LinuxAMD64CFrame(dbg, nextRBP, nextPC);
   }

   // package/class internals only
   private static final int ADDRESS_SIZE = 8;
   private Address rip;
   private Address rbp; 
   private LinuxDebugger dbg;
}
