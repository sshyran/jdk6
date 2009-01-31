/*
 * @(#)InvocableCallable.java	1.3 07/05/05 17:03:43
 *
 * Copyright 2007 Sun Microsystems, Inc.  All Rights Reserved.
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
 */

package sun.jvm.hotspot.utilities.soql;

import javax.script.Invocable;
import javax.script.ScriptException;

/**
 * This Callable implementation invokes a script
 * function of given name when called. If the target
 * object is non-null, script "method" is invoked, else
 * a "global" script function is invoked.
 */
public class InvocableCallable implements Callable {
  private Object target;
  private String name;
  private Invocable invocable;
  
  public InvocableCallable(Object target, String name, 
    Invocable invocable) {
    this.target = target;
    this.name = name;
    this.invocable = invocable;
  }
  
  public Object call(Object[] args) throws ScriptException {
    try {
      if (target == null) {      
        return invocable.invokeFunction(name, args);
      } else {
        return invocable.invokeMethod(target, name, args);
      }
    } catch (NoSuchMethodException nme) {
      throw new ScriptException(nme);
    }
  }
}