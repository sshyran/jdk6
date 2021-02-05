/*
 * Copyright 2000 Sun Microsystems, Inc.  All Rights Reserved.
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
 * @bug 4328748 
 * @summary AbstractMap's clone() method is implemented to 
 *  reset AbstractMap's private fields after super.clone()
 *                                
 * @author Konstantin Kladko
 */

import java.util.*;

public class AbstractMapClone extends AbstractMap implements Cloneable {
    
    private Map map = new HashMap();
    
    public Set entrySet() {
        return map.entrySet();
    }
    
    public Object put(Object key, Object value) {
        return map.put(key, value);
    }
    
    public Object clone() {
        AbstractMapClone clone = null;
        try {
        clone = (AbstractMapClone)super.clone();
        } catch (CloneNotSupportedException e) {
        }
        clone.map = (Map)((HashMap)map).clone();
        return clone;
    }
    
    public static void main(String[] args) {
        AbstractMapClone m1 = new AbstractMapClone();
        m1.put("1", "1");
        Set k1 = m1.keySet();
        AbstractMapClone m2 = (AbstractMapClone)m1.clone();
        Set k2 = m2.keySet();
        m2.put("2","2");
        if (k1.equals(k2)) {
            throw new RuntimeException("AbstractMap.clone() failed.");
        }
    }
}
