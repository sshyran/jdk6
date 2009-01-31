/* 
 * Copyright (c) 2007 Sun Microsystems, Inc.  All Rights Reserved.
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
 * @bug 6355009
 * @summary Make sure not to have too many causes for MissingResourceException
 */

import java.util.ResourceBundle;
import java.util.MissingResourceException;

public final strictfp class Bug6355009 {
    private final ResourceBundle bundle = ResourceBundle.getBundle(Bug6355009.class.getName());
    
    public final static void main(String[] args) {
    	try {
	    new Bug6355009();
	} catch (MissingResourceException e) {
	    Throwable cause = e;
	    int count = 0;
	    while ((cause = cause.getCause()) != null) {
		if (cause instanceof MissingResourceException) {
		    count++;
		}
	    }
	    if (count > 0) {
	    	throw new RuntimeException("too many causes: " + count);
	    }
	}
    }
}
