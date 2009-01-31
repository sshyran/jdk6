#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)location.cpp	1.40 07/05/05 17:05:21 JVM"
#endif
/*
 * Copyright 1997-2006 Sun Microsystems, Inc.  All Rights Reserved.
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
 *  
 */

#include "incls/_precompiled.incl"
#include "incls/_location.cpp.incl"

void Location::print_on(outputStream* st) const {
  if(type() == invalid && !legal_offset_in_bytes(offset() * BytesPerInt)) {
    // product of Location::invalid_loc() or Location::Location().
    switch (where()) {
    case on_stack:     st->print("empty");    break;
    case in_register:  st->print("invalid");  break;
    }
    return;
  }
  switch (where()) {
  case on_stack:    st->print("stack[%d]", stack_offset());    break;
  case in_register: st->print("reg %s [%d]", reg()->name(), register_number()); break;
  default:          st->print("Wrong location where %d", where());
  }
  switch (type()) {
  case normal:                                 break;
  case oop:          st->print(",oop");        break;
  case int_in_long:  st->print(",int");        break;
  case lng:          st->print(",long");       break;
  case float_in_dbl: st->print(",float");      break;
  case dbl:          st->print(",double");     break;
  case addr:         st->print(",address");    break;
  default:           st->print("Wrong location type %d", type());
  }
}


Location::Location(DebugInfoReadStream* stream) {
  _value = (uint16_t) stream->read_int();
}


void Location::write_on(DebugInfoWriteStream* stream) {
  stream->write_int(_value & 0x0000FFFF);
}


// Valid argument to Location::new_stk_loc()?
bool Location::legal_offset_in_bytes(int offset_in_bytes) {
  if ((offset_in_bytes % BytesPerInt) != 0)  return false;
  return (offset_in_bytes / BytesPerInt) < (OFFSET_MASK >> OFFSET_SHIFT);
}
