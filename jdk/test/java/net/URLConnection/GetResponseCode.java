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
 *
 * @bug 4191815
 * @summary Check that getResponseCode doesn't throw exception if http
 *          respone code is >= 400.
 */
import java.net.*;
import java.io.*;

public class GetResponseCode implements Runnable {

    ServerSocket ss;

    /* 
     * Our "http" server to return a 404
     */
    public void run() {
	try {
	    Socket s = ss.accept();

	    PrintStream out = new PrintStream( 
				 new BufferedOutputStream(
				    s.getOutputStream() ));

	    /* send the header */
	    out.print("HTTP/1.1 404 Not Found\r\n");
	    out.print("Content-Type: text/html; charset=iso-8859-1\r\n");
	    out.print("Connection: close\r\n");
	    out.print("\r\n");
	    out.print("<HTML>");
	    out.print("<HEAD><TITLE>404 Not Found</TITLE></HEAD>");
	    out.print("<BODY>The requested URL was not found.</BODY>");
	    out.print("</HTML>");
	    out.flush();

	    s.close();
	    ss.close();
	} catch (Exception e) { 
	    e.printStackTrace();
	}
    }

    GetResponseCode() throws Exception {

	/* start the server */
	ss = new ServerSocket(0);
	(new Thread(this)).start();

	/* establish http connection to server */
	String uri = "http://localhost:" + 
		     Integer.toString(ss.getLocalPort()) + 
		     "/missing.nothtml";
	URL url = new URL(uri);

	HttpURLConnection http = (HttpURLConnection)url.openConnection();

	int respCode = http.getResponseCode();

	http.disconnect();

    }

    public static void main(String args[]) throws Exception {
	new GetResponseCode();
    }

}
