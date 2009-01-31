/*
 * Copyright 1998-2003 Sun Microsystems, Inc.  All Rights Reserved.
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
  
#ifdef __SUNPRO_C
#pragma ident   "@(#)mlib_c_ImageBlendTable.h	1.4    99/07/29 SMI"
#endif /* __SUNPRO_C */

/*
 *    These tables are used by C versions of the
 *    mlib_ImageBlend_... functions.
 */

#ifndef MLIB_C_IMAGE_BLEND_TABLE_H
#define MLIB_C_IMAGE_BLEND_TABLE_H

#include "mlib_image.h"

extern const mlib_f32 mlib_c_blend_u8[];
extern const mlib_f32 mlib_U82F32[];
extern const mlib_f32 mlib_c_blend_Q8[];
extern const mlib_f32 mlib_c_blend_u8_sat[];

#endif /* MLIB_C_IMAGEF_BLEND_TABLE_H */

