/*
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
 *
 */

/*
 * @(#)AlternateSubstSubtables.h	1.5 06/12/13
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#ifndef __ALTERNATESUBSTITUTIONSUBTABLES_H
#define __ALTERNATESUBSTITUTIONSUBTABLES_H

#include "LETypes.h"
#include "LEGlyphFilter.h"
#include "OpenTypeTables.h"
#include "GlyphSubstitutionTables.h"
#include "GlyphIterator.h"

struct AlternateSetTable
{
    le_uint16 glyphCount;
    TTGlyphID alternateArray[ANY_NUMBER];
};

struct AlternateSubstitutionSubtable : GlyphSubstitutionSubtable
{
    le_uint16 alternateSetCount;
    Offset    alternateSetTableOffsetArray[ANY_NUMBER];

    le_uint32 process(GlyphIterator *glyphIterator, const LEGlyphFilter *filter = NULL) const;
};

#endif
