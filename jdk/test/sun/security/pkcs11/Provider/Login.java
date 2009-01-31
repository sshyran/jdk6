/*
 * Copyright 2003-2004 Sun Microsystems, Inc.  All Rights Reserved.
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

import java.io.*;
import java.util.*;
import java.lang.reflect.*;
import java.security.*;
import javax.security.auth.callback.*;

import javax.security.auth.Subject;
import javax.security.auth.login.FailedLoginException;

public class Login extends PKCS11Test {

    private static final String KS_TYPE = "PKCS11";
    private static char[] password;

    public static void main(String[] args) throws Exception {
	main(new Login());
    }

    public void main(Provider p) throws Exception {

	int testnum = 1;

        KeyStore ks = KeyStore.getInstance(KS_TYPE, p);

        // check instance
        if (ks.getProvider() instanceof java.security.AuthProvider) {
            System.out.println("keystore provider instance of AuthProvider");
            System.out.println("test " + testnum++ + " passed");
        } else {
            throw new SecurityException("did not get AuthProvider KeyStore");
        }

        AuthProvider ap = (AuthProvider)ks.getProvider();
        try {

            // test app-provided callback
            System.out.println("*** enter [foo] as the password ***");
	    password = new char[] { 'f', 'o', 'o' };

            ap.login(new Subject(), new PasswordCallbackHandler());
	    ap.logout();
            throw new SecurityException("test failed, expected LoginException");
	} catch (FailedLoginException fle) {
            System.out.println("test " + testnum++ + " passed");
        }

        try {

            // test default callback
            System.out.println("*** enter [foo] as the password ***");
	    password = new char[] { 'f', 'o', 'o' };

            Security.setProperty("auth.login.defaultCallbackHandler",
                "Login$PasswordCallbackHandler");
            ap.login(new Subject(), null);
	    ap.logout();
            throw new SecurityException("test failed, expected LoginException");
        } catch (FailedLoginException fle) {
            System.out.println("test " + testnum++ + " passed");
        }

        // test provider-set callback
        System.out.println("*** enter test12 (correct) password ***");
	password = new char[] { 't', 'e', 's', 't', '1', '2' };

        Security.setProperty("auth.login.defaultCallbackHandler", "");
        ap.setCallbackHandler
                (new com.sun.security.auth.callback.DialogCallbackHandler());
        ap.setCallbackHandler(new PasswordCallbackHandler());
        ap.login(new Subject(), null);
        System.out.println("test " + testnum++ + " passed");

	// test user already logged in
        ap.setCallbackHandler(null);
        ap.login(new Subject(), null);
        System.out.println("test " + testnum++ + " passed");

	// logout
	ap.logout();

	// call KeyStore.load with a NULL password, and get prompted for PIN
        ap.setCallbackHandler(new PasswordCallbackHandler());
	ks.load(null, (char[])null);
        System.out.println("test " + testnum++ + " passed");
    }

    public static class PasswordCallbackHandler implements CallbackHandler {
        public void handle(Callback[] callbacks)
                throws IOException, UnsupportedCallbackException {
            if (!(callbacks[0] instanceof PasswordCallback)) {
                throw new UnsupportedCallbackException(callbacks[0]);
            }
            PasswordCallback pc = (PasswordCallback)callbacks[0];
            pc.setPassword(Login.password);
        }
    }
}
