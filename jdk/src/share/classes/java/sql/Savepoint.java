/*
 * Copyright 2000-2005 Sun Microsystems, Inc.  All Rights Reserved.
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

package java.sql;
 
/**
 * The representation of a savepoint, which is a point within
 * the current transaction that can be referenced from the 
 * <code>Connection.rollback</code> method. When a transaction
 * is rolled back to a savepoint all changes made after that
 * savepoint are undone.
 * <p>
 * Savepoints can be either named or unnamed. Unnamed savepoints
 * are identified by an ID generated by the underlying data source.
 *
 * @since 1.4
 */

public interface Savepoint {

    /**
     * Retrieves the generated ID for the savepoint that this 
     * <code>Savepoint</code> object represents.
     * @return the numeric ID of this savepoint
     * @exception SQLException if this is a named savepoint
     * @since 1.4
     */
    int getSavepointId() throws SQLException;

    /**
     * Retrieves the name of the savepoint that this <code>Savepoint</code>
     * object represents.
     * @return the name of this savepoint
     * @exception SQLException if this is an un-named savepoint
     * @since 1.4
     */
    String getSavepointName() throws SQLException;
}


