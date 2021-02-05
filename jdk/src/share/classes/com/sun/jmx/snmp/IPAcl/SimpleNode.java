/*
 * Copyright 1997-2007 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
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


/* Generated By:JJTree: Do not edit this line. SimpleNode.java */

package com.sun.jmx.snmp.IPAcl;

import java.net.InetAddress;
import java.util.Hashtable;
import java.util.Vector;

class SimpleNode implements Node {
    protected Node parent;
    protected Node[] children;
    protected int id;
    protected Parser parser;

    public SimpleNode(int i) {
        id = i;
    }

    public SimpleNode(Parser p, int i) {
        this(i);
        parser = p;
    }

    public static Node jjtCreate(int id) {
        return new SimpleNode(id);
    }

    public static Node jjtCreate(Parser p, int id) {
        return new SimpleNode(p, id);
    }

    public void jjtOpen() {
    }

    public void jjtClose() {
    }

    public void jjtSetParent(Node n) { parent = n; }
    public Node jjtGetParent() { return parent; }

    public void jjtAddChild(Node n, int i) {
        if (children == null) {
            children = new Node[i + 1];
        } else if (i >= children.length) {
            Node c[] = new Node[i + 1];
            System.arraycopy(children, 0, c, 0, children.length);
            children = c;
        }
        children[i] = n;
    }

    public Node jjtGetChild(int i) {
        return children[i];
    }

    public int jjtGetNumChildren() {
        return (children == null) ? 0 : children.length;
    }

    /*
      SR. Extend the SimpleNode definition
    */

    /**
     * Build the Trap entries from the syntactic tree.
     */
    public void buildTrapEntries(Hashtable<InetAddress, Vector<String>> dest) {
        if (children != null) {
            for (int i = 0; i < children.length; ++i) {
                SimpleNode n = (SimpleNode)children[i];
                if (n != null) {
                    n.buildTrapEntries(dest);
                }
            } /* end of loop */
        }
    }
    /**
     * Build the Inform entries from the syntactic tree.
     */
    public void buildInformEntries(Hashtable<InetAddress, Vector<String>> dest) {
        if (children != null) {
            for (int i = 0; i < children.length; ++i) {
                SimpleNode n = (SimpleNode)children[i];
                if (n != null) {
                    n.buildInformEntries(dest);
                }
            } /* end of loop */
        }
    }

    /**
     * Build the Acl entries from the syntactic tree.
     */
    public void buildAclEntries(PrincipalImpl owner, AclImpl acl) {
        if (children != null) {
            for (int i = 0; i < children.length; ++i) {
                SimpleNode n = (SimpleNode)children[i];
                if (n != null) {
                    n.buildAclEntries(owner, acl);
                }
            } /* end of loop */
        }
    }

    /* END SR */

    /* You can override these two methods in subclasses of SimpleNode to
       customize the way the node appears when the tree is dumped.  If
       your output uses more than one line you should override
       toString(String), otherwise overriding toString() is probably all
       you need to do. */

    public String toString() { return ParserTreeConstants.jjtNodeName[id]; }
    public String toString(String prefix) { return prefix + toString(); }

    /* Override this method if you want to customize how the node dumps
       out its children. */

    public void dump(String prefix) {
        if (children != null) {
            for (int i = 0; i < children.length; ++i) {
                SimpleNode n = (SimpleNode)children[i];
                if (n != null) {
                    n.dump(prefix + " ");
                }
            }
        }
    }
}

