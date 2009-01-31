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

package sun.awt.windows;

import java.awt.Component;
import java.awt.Cursor;

import java.awt.datatransfer.Transferable;

import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.InvalidDnDOperationException;

import java.awt.event.InputEvent;

import java.util.Map;

import sun.awt.dnd.SunDragSourceContextPeer;

/**
 * <p>
 * TBC
 * </p>
 *
 * @version %R%.%L%
 * @since JDK1.2
 *
 */

final class WDragSourceContextPeer extends SunDragSourceContextPeer {
    public void startSecondaryEventLoop(){
        WToolkit.startSecondaryEventLoop();
    }
    public void quitSecondaryEventLoop(){
        WToolkit.quitSecondaryEventLoop();
    }

    private static final WDragSourceContextPeer theInstance = 
        new WDragSourceContextPeer(null);

    /**
     * construct a new WDragSourceContextPeer. package private
     */

    private WDragSourceContextPeer(DragGestureEvent dge) {
    	super(dge);
    }

    static WDragSourceContextPeer createDragSourceContextPeer(DragGestureEvent dge) throws InvalidDnDOperationException {
	theInstance.setTrigger(dge);
        return theInstance;
    }

    protected void startDrag(Transferable trans, 
                             long[] formats, Map formatMap) {

        long nativeCtxtLocal = 0;

        nativeCtxtLocal = createDragSource(getTrigger().getComponent(),
                                           trans,
                                           getTrigger().getTriggerEvent(),
                                           getTrigger().getSourceAsDragGestureRecognizer().getSourceActions(),
                                           formats,
                                           formatMap);
            
        if (nativeCtxtLocal == 0) {
            throw new InvalidDnDOperationException("failed to create native peer");
        }

        setNativeContext(nativeCtxtLocal);

        WDropTargetContextPeer.setCurrentJVMLocalSourceTransferable(trans);
        
        doDragDrop(getNativeContext(), getCursor());
    }

    /**
     * downcall into native code
     */

    native long createDragSource(Component component,
                                 Transferable transferable,
                                 InputEvent nativeTrigger, 
                                 int actions,
                                 long[] formats,
                                 Map formatMap);

    /**
     * downcall into native code
     */

    native void doDragDrop(long nativeCtxt, Cursor cursor);

    protected native void setNativeCursor(long nativeCtxt, Cursor c, int cType);

}


