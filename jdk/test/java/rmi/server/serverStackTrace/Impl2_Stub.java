/* 
 * Copyright 2001 Sun Microsystems, Inc.  All Rights Reserved.
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

// Stub class generated by rmic, do not edit.
// Contents subject to change without notice.

public final class Impl2_Stub
    extends java.rmi.server.RemoteStub
    implements Pong
{
    private static final long serialVersionUID = 2;
    
    private static java.lang.reflect.Method $method_pong_0;
    
    static {
	try {
	    $method_pong_0 = Pong.class.getMethod("pong", new java.lang.Class[] {});
	} catch (java.lang.NoSuchMethodException e) {
	    throw new java.lang.NoSuchMethodError(
		"stub class initialization failed");
	}
    }
    
    // constructors
    public Impl2_Stub(java.rmi.server.RemoteRef ref) {
	super(ref);
    }
    
    // methods from remote interfaces
    
    // implementation of pong()
    public void pong()
	throws PongException, java.rmi.RemoteException
    {
	try {
	    ref.invoke(this, $method_pong_0, null, -5941237182396893426L);
	} catch (java.lang.RuntimeException e) {
	    throw e;
	} catch (java.rmi.RemoteException e) {
	    throw e;
	} catch (PongException e) {
	    throw e;
	} catch (java.lang.Exception e) {
	    throw new java.rmi.UnexpectedException("undeclared checked exception", e);
	}
    }
}
