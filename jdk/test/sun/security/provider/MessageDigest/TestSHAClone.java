/*
 * Copyright 2002-2003 Sun Microsystems, Inc.  All Rights Reserved.
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

/**
 * @test
 * @bug 4775971
 * @summary test the clone implementation of SHA, SHA-256, 
 *          SHA-384, SHA-512 MessageDigest implementation.
 */
import java.security.*;
import java.util.*;

public class TestSHAClone {

    private static final String[] ALGOS = {
	"SHA", "SHA-256", "SHA-512", "SHA-384"
    };

    private static byte[] input1 = {
	(byte)0x1, (byte)0x2,  (byte)0x3
    };

    private static byte[] input2 = {
	(byte)0x4, (byte)0x5,  (byte)0x6
    };
    
    private MessageDigest md;

    private TestSHAClone(String algo, Provider p) throws Exception {
	md = MessageDigest.getInstance(algo, p);
    }

    private void run() throws Exception {
	md.update(input1);
	MessageDigest md2 = (MessageDigest) md.clone();
	md.update(input2);
	md2.update(input2);
	if (!Arrays.equals(md.digest(), md2.digest())) {
	    throw new Exception(md.getAlgorithm() + ": comparison failed");
	} else {
	    System.out.println(md.getAlgorithm() + ": passed");
	}
    }


    public static void main(String[] argv) throws Exception {
	Provider p = Security.getProvider("SUN");
	for (int i=0; i<ALGOS.length; i++) {
	    TestSHAClone test = new TestSHAClone(ALGOS[i], p);
	    test.run();	    
	} 
    }
}
