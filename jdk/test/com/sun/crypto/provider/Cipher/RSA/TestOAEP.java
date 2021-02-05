/*
 * Copyright 2003-2007 Sun Microsystems, Inc.  All Rights Reserved.
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

/*
 * @test
 * @bug 4894151
 * @summary encryption/decryption test for OAEP
 * @author Andreas Sterbenz
 */

import java.util.*;

import java.security.*;

import javax.crypto.*;

public class TestOAEP {

    private static Provider cp;

    private static PrivateKey privateKey;

    private static PublicKey publicKey;

    private static Random random = new Random();

    public static void main(String[] args) throws Exception {
	long start = System.currentTimeMillis();
	cp = Security.getProvider("SunJCE");
	System.out.println("Testing provider " + cp.getName() + "...");
	Provider kfp = Security.getProvider("SunRsaSign");
	KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA", kfp);
	kpg.initialize(768);
	KeyPair kp = kpg.generateKeyPair();
	privateKey = kp.getPrivate();
	publicKey = kp.getPublic();

	Cipher.getInstance("RSA/ECB/OAEPwithMD5andMGF1Padding");
	Cipher.getInstance("RSA/ECB/OAEPwithSHA1andMGF1Padding");
	Cipher.getInstance("RSA/ECB/OAEPwithSHA-1andMGF1Padding");
	Cipher.getInstance("RSA/ECB/OAEPwithSHA-256andMGF1Padding");
	Cipher.getInstance("RSA/ECB/OAEPwithSHA-384andMGF1Padding");
	Cipher.getInstance("RSA/ECB/OAEPwithSHA-512andMGF1Padding");

	// basic test using MD5
	testEncryptDecrypt("MD5", 0);
	testEncryptDecrypt("MD5", 16);
	testEncryptDecrypt("MD5", 62);
	try {
	    testEncryptDecrypt("MD5", 63);
	    throw new Exception("Unexpectedly completed call");
	} catch (IllegalBlockSizeException e) {
	    // ok
	    System.out.println(e);
	}

	// basic test using SHA-1
	testEncryptDecrypt("SHA1", 0);
	testEncryptDecrypt("SHA1", 16);
	testEncryptDecrypt("SHA1", 54);
	try {
	    testEncryptDecrypt("SHA1", 55);
	    throw new Exception("Unexpectedly completed call");
	} catch (IllegalBlockSizeException e) {
	    // ok
	    System.out.println(e);
	}
	// tests alias works
	testEncryptDecrypt("SHA-1", 16);

	// basic test using SHA-256
	testEncryptDecrypt("SHA-256", 0);
	testEncryptDecrypt("SHA-256", 16);
	testEncryptDecrypt("SHA-256", 30);
	try {
	    testEncryptDecrypt("SHA-256", 31);
	    throw new Exception("Unexpectedly completed call");
	} catch (IllegalBlockSizeException e) {
	    // ok
	    System.out.println(e);
	}

	// 768 bit key too short for OAEP with 64 byte digest
	try {
	    testEncryptDecrypt("SHA-512", 1);
	    throw new Exception("Unexpectedly completed call");
	} catch (InvalidKeyException e) {
	    // ok
	    System.out.println(e);
	}

	Cipher c;
	byte[] enc;
	byte[] data = new byte[16];
	random.nextBytes(data);

	try {
	    c = Cipher.getInstance("RSA/ECB/OAEPwithFOOandMGF1Padding", cp);
	    throw new Exception("Unexpectedly completed call");
	} catch (NoSuchPaddingException e) {
	    // ok
	    System.out.println(e);
	}

	c = Cipher.getInstance("RSA/ECB/OAEPwithMD5andMGF1Padding", cp);
	// cannot "sign" using OAEP
	try {
	    c.init(Cipher.ENCRYPT_MODE, privateKey);
	    throw new Exception("Unexpectedly completed call");
	} catch (InvalidKeyException e) {
	    // ok
	    System.out.println(e);
	}

	// cannot "verify" using OAEP
	try {
	    c.init(Cipher.DECRYPT_MODE, publicKey);
	    throw new Exception("Unexpectedly completed call");
	} catch (InvalidKeyException e) {
	    // ok
	    System.out.println(e);
	}

	// decryption failure
	c.init(Cipher.DECRYPT_MODE, privateKey);
	try {
	    c.doFinal(data);
	    throw new Exception("Unexpectedly completed call");
	} catch (BadPaddingException e) {
	    // ok
	    System.out.println(e);
	}

	// wrong hash length
	c.init(Cipher.ENCRYPT_MODE, publicKey);
	enc = c.doFinal(data);
	c = Cipher.getInstance("RSA/ECB/OAEPwithSHA1andMGF1Padding", cp);
	c.init(Cipher.DECRYPT_MODE, privateKey);
	try {
	    c.doFinal(enc);
	    throw new Exception("Unexpectedly completed call");
	} catch (BadPaddingException e) {
	    // ok
	    System.out.println(e);
	}

/* MD2 not supported with OAEP
	// wrong hash value
	c = Cipher.getInstance("RSA/ECB/OAEPwithMD2andMGF1Padding", cp);
	c.init(Cipher.DECRYPT_MODE, privateKey);
	try {
	    c.doFinal(enc);
	    throw new Exception("Unexpectedly completed call");
	} catch (BadPaddingException e) {
	    // ok
	    System.out.println(e);
	}
*/

	// wrong padding type
	c = Cipher.getInstance("RSA/ECB/PKCS1Padding", cp);
	c.init(Cipher.ENCRYPT_MODE, publicKey);
	enc = c.doFinal(data);
	c = Cipher.getInstance("RSA/ECB/OAEPwithSHA1andMGF1Padding", cp);
	c.init(Cipher.DECRYPT_MODE, privateKey);
	try {
	    c.doFinal(enc);
	    throw new Exception("Unexpectedly completed call");
	} catch (BadPaddingException e) {
	    // ok
	    System.out.println(e);
	}

	long stop = System.currentTimeMillis();
	System.out.println("Done (" + (stop - start) + " ms).");
    }

    private static void testEncryptDecrypt(String hashAlg, int dataLength) throws Exception {
	System.out.println("Testing OAEP with hash " + hashAlg + ", " + dataLength + " bytes");
	Cipher c = Cipher.getInstance("RSA/ECB/OAEPwith" + hashAlg + "andMGF1Padding", cp);
	c.init(Cipher.ENCRYPT_MODE, publicKey);
	byte[] data = new byte[dataLength];
	byte[] enc = c.doFinal(data);
	c.init(Cipher.DECRYPT_MODE, privateKey);
	byte[] dec = c.doFinal(enc);
	if (Arrays.equals(data, dec) == false) {
	    throw new Exception("Data does not match");
	}
    }
}
