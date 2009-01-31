/*
 * Portions Copyright 2000-2003 Sun Microsystems, Inc.  All Rights Reserved.
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

/*
 * (C) Copyright IBM Corp. 2000 - 2003 - All Rights Reserved
 *
 * The original version of this source code and documentation is
 * copyrighted and owned by IBM. These materials are provided
 * under terms of a License Agreement between IBM and Sun.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class Bidi */

#ifndef _Included_Bidi
#define _Included_Bidi
#ifdef __cplusplus
extern "C" {
#endif
#undef Bidi_DIR_LTR
#define Bidi_DIR_LTR 0L
#undef Bidi_DIR_RTL
#define Bidi_DIR_RTL 1L
#undef Bidi_DIR_DEFAULT_LTR
#define Bidi_DIR_DEFAULT_LTR -2L
#undef Bidi_DIR_DEFAULT_RTL
#define Bidi_DIR_DEFAULT_RTL -1L
#undef Bidi_DIR_MIXED
#define Bidi_DIR_MIXED -1L
#undef Bidi_DIR_MIN
#define Bidi_DIR_MIN -2L
#undef Bidi_DIR_MAX
#define Bidi_DIR_MAX 1L

JNIEXPORT jint JNICALL Java_java_text_Bidi_nativeGetDirectionCode
  (JNIEnv *, jclass, jint);

JNIEXPORT void JNICALL Java_java_text_Bidi_nativeBidiChars
  (JNIEnv *, jclass, jobject, jcharArray, jint, jbyteArray, jint, jint, jint);

#ifdef __cplusplus
}
#endif
#endif

