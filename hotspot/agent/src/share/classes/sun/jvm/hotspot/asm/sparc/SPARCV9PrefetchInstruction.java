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

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9PrefetchInstruction extends SPARCInstruction 
                    implements SPARCV9Instruction {
    final private SPARCRegisterIndirectAddress addr;
    final private int prefetchFcn;
    final private String description;

    public static final int PREFETCH_MANY_READS  = 0;
    public static final int PREFETCH_ONE_READ    = 1;
    public static final int PREFETCH_MANY_WRITES = 2;
    public static final int PREFETCH_ONE_WRITE   = 3;
    public static final int PREFETCH_PAGE        = 4;
  
    public SPARCV9PrefetchInstruction(String name, SPARCRegisterIndirectAddress addr, int prefetchFcn) {
        super(name);
        this.addr = addr;
        this.prefetchFcn = prefetchFcn;
        description = initDescription();
    }

    private String initDescription() {
        StringBuffer buf = new StringBuffer();
        buf.append(getName());
        buf.append(spaces);
        buf.append(addr.toString());
        buf.append(comma);
        buf.append(prefetchFcn);
        return buf.toString();
    }

    public int getPrefetchFunction() {
        return prefetchFcn;
    }

    public SPARCRegisterIndirectAddress getPrefetchAddress() {
        return addr;
    }

    public String asString(long currentPc, SymbolFinder symFinder) {
        return description;
    }
}
