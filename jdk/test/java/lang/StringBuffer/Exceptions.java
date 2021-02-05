/*
 * Copyright 2002-2005 Sun Microsystems, Inc.  All Rights Reserved.
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
 * @bug 4414306 6248507
 * @summary Verify that exceptions are thrown as expected.
 */

public class Exceptions {
    private static boolean ok = true;

     private static void fail(Throwable ex, String s, Throwable got) {
        ok = false;
        System.err.println("expected "
			   + ex.getClass().getName() + ": " + ex.getMessage()
			   + " for " + s
			   + " got "
			   + got.getClass().getName() + ": " + got.getMessage() 
			   + " - FAILED");
    }

    private static void pass(String s) {
        System.out.println(s + " -- OK");
    }

    private static void tryCatch(String s, Throwable ex, Runnable thunk) {    
        Throwable t = null;
        try {
            thunk.run();
        } catch (Throwable x) {
//          x.printStackTrace();	    
	    if (ex.getClass().isAssignableFrom(x.getClass()))
                t = x;
            else
                x.printStackTrace();
        }
        if ((t == null) && (ex != null))
            fail(ex, s, t);
	
	String msg = (ex == null ? null : ex.getMessage());
	if ((msg != null) && !msg.equals(t.getMessage())) 
	    fail(ex, s, t);
	else
            pass(s);
    }

    public static void main(String [] args) {
        System.out.println("StringBuffer()");
        tryCatch("  no args", null, new Runnable() {
                public void run() {
                    new StringBuffer();
                }});

        System.out.println("StringBuffer(int length)");
        tryCatch("  1", null, new Runnable() {
                public void run() {
                    new StringBuffer(1);
                }});
        tryCatch("  -1", new NegativeArraySizeException(), new Runnable() {
                public void run() {
                    new StringBuffer(-1);
                }});

        System.out.println("StringBuffer(String str)");
	tryCatch("  null", new NullPointerException(), new Runnable() {
                public void run() {
                    new StringBuffer(null);
                }});
        tryCatch("  foo", null, new Runnable() {
                public void run() {
                    new StringBuffer("foo");
                }});

	System.out.println("StringBuffer.replace(int start, int end, String str)");
 	tryCatch("  -1, 2, \" \"",
		 new StringIndexOutOfBoundsException(-1),
		 new Runnable() {
                public void run() {
		    StringBuffer sb = new StringBuffer("hilbert");
		    sb.replace(-1, 2, " ");
		}});

 	tryCatch("  7, 8, \" \"",
		 new StringIndexOutOfBoundsException("start > length()"),
		 new Runnable() {
                public void run() {
		    StringBuffer sb = new StringBuffer("banach");
		    sb.replace(7, 8, " ");
		}});
 	tryCatch("  2, 1, \" \"", 
		 new StringIndexOutOfBoundsException("start > end"),
		 new Runnable() {
                public void run() {
		    StringBuffer sb = new StringBuffer("riemann");
		    sb.replace(2, 1, " ");
		}});

        if (!ok)
            throw new RuntimeException("Some tests FAILED");
        else
            System.out.println("All tests PASSED");
    }
}
