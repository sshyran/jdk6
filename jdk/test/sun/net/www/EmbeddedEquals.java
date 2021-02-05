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

/**
 * @test
 * @bug 4456995
 * @summary  HeaderParser misinterprets a '=' character embedded in value
 */

import sun.net.www.HeaderParser;

public class EmbeddedEquals {
    static String test = "WWW-Authenticate: Digest realm=\"testrealm\","+
                      "nonce=\"Ovqrpw==b20ff3b0ea3f3a18f1d6293331edaafdb98f5bef\", algorithm=MD5,"+
                      "domain=\"http://bakedbean.ireland/authtest/\", qop=\"auth\"";

    public static void main (String args[]) {
	HeaderParser hp = new HeaderParser (test);
	String r1 = hp.findValue ("nonce");
	if (r1 == null || !r1.equals ("Ovqrpw==b20ff3b0ea3f3a18f1d6293331edaafdb98f5bef")) {
	    throw new RuntimeException ("first findValue returned wrong result: " + r1);
	}
	r1 = hp.findValue ("AlGoRiThm");
	if (r1 == null || !r1.equals ("MD5")) {
	    throw new RuntimeException ("second findValue returned wrong result: " + r1);
	}
    }
}
	
