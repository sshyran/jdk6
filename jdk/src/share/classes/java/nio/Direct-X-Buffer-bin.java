/*
 * Copyright 2000-2003 Sun Microsystems, Inc.  All Rights Reserved.
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

#warn This file is preprocessed before being compiled

class XXX {

#begin

#if[rw]

    private $type$ get$Type$(long a) {
	if (unaligned) {
	    $memtype$ x = unsafe.get$Memtype$(a);
	    return $fromBits$(nativeByteOrder ? x : Bits.swap(x));
	}
	return Bits.get$Type$(a, bigEndian);
    }

    public $type$ get$Type$() {
	return get$Type$(ix(nextGetIndex($BYTES_PER_VALUE$)));
    }

    public $type$ get$Type$(int i) {
	return get$Type$(ix(checkIndex(i, $BYTES_PER_VALUE$)));
    }

#end[rw]

    private ByteBuffer put$Type$(long a, $type$ x) {
#if[rw]
	if (unaligned) {
	    $memtype$ y = $toBits$(x);
	    unsafe.put$Memtype$(a, (nativeByteOrder ? y : Bits.swap(y)));
	} else {
	    Bits.put$Type$(a, x, bigEndian);
	}
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public ByteBuffer put$Type$($type$ x) {
#if[rw]
	put$Type$(ix(nextPutIndex($BYTES_PER_VALUE$)), x);
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public ByteBuffer put$Type$(int i, $type$ x) {
#if[rw]
	put$Type$(ix(checkIndex(i, $BYTES_PER_VALUE$)), x);
	return this;
#else[rw]
	throw new ReadOnlyBufferException();
#end[rw]
    }

    public $Type$Buffer as$Type$Buffer() {
	int off = this.position();
	int lim = this.limit();
	assert (off <= lim);
	int rem = (off <= lim ? lim - off : 0);

	int size = rem >> $LG_BYTES_PER_VALUE$;
 	if (!unaligned && ((address + off) % $BYTES_PER_VALUE$ != 0)) {
	    return (bigEndian
		    ? ($Type$Buffer)(new ByteBufferAs$Type$Buffer$RW$B(this,
								       -1,
								       0,
								       size,
								       size,
								       off))
		    : ($Type$Buffer)(new ByteBufferAs$Type$Buffer$RW$L(this,
								       -1,
								       0,
								       size,
								       size,
								       off)));
	} else {
	    return (nativeByteOrder
		    ? ($Type$Buffer)(new Direct$Type$Buffer$RW$U(this,
								 -1,
								 0,
								 size,
								 size,
								 off))
		    : ($Type$Buffer)(new Direct$Type$Buffer$RW$S(this,
								 -1,
								 0,
								 size,
								 size,
								 off)));
	}
    }

#end

}
