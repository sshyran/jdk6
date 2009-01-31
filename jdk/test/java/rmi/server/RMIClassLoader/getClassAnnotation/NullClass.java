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
 */

/* @test
 * @bug 4518927
 * @summary RMIClassLoader.getClassAnnotation() needs to specify behavior
 * for null class
 * @author Ann Wollrath
 *
 * @library ../../../testlibrary
 * @build TestLibrary
 * @build NullClass
 * @run main/othervm NullClass
 */

import java.rmi.server.RMIClassLoader;

public class NullClass {

    public static void main(String[] args) {

	System.err.println("\nRegression test for bug 4518927\n");

	try {
	    System.err.println("getting class annotation for null class...");
	    String annotation = RMIClassLoader.getClassAnnotation(null);
	    throw new RuntimeException(
		"TEST FAILED: NullPointerException not caught!");
	} catch (NullPointerException e) {
	    System.err.println("TEST PASSED: NullPointerException caught");
	} catch (Exception e) {
	    TestLibrary.bomb(e);
	}
    }
}
