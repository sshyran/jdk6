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

/* @test
 * @bug 4418673
 * @summary verify that the value of the java.rmi.server.RMIClassLoaderSpi
 * system property is interpreted to name the class to instantiate for the
 * RMIClassLoader provider.
 * @author Peter Jones
 *
 * @library ../../../testlibrary
 * @build TestLibrary
 * @build Property
 * @build ServiceConfiguration
 * @build TestProvider
 * @run main/othervm/policy=security.policy Property
 */

public class Property {
    public static void main(String[] args) throws Exception {

	ServiceConfiguration.installServiceConfigurationFile();

	System.setProperty(
	    "java.rmi.server.RMIClassLoaderSpi", "TestProvider");

	TestProvider.exerciseTestProvider(
	    TestProvider.loadClassReturn,
	    TestProvider.loadProxyClassReturn,
	    TestProvider.getClassLoaderReturn,
	    TestProvider.getClassAnnotationReturn,
	    TestProvider.invocations);
    }
}
