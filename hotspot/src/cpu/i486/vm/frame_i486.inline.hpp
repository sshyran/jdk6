#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)frame_i486.inline.hpp	1.73 07/05/05 17:04:15 JVM"
#endif
/*
 * Copyright 1997-2007 Sun Microsystems, Inc.  All Rights Reserved.
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

// Inline functions for Intel frames:

// Constructors:

inline frame::frame() { 
  _pc = NULL;
  _sp = NULL;
  _unextended_sp = NULL;
  _fp = NULL;
  _cb = NULL;
  _deopt_state = unknown;
}

inline frame:: frame(jint* sp, jint* fp, address pc) { 
  _sp = sp;
  _unextended_sp = sp;
  _fp = fp;
  _pc = pc;
  assert(pc != NULL, "no pc?");
  _cb = CodeCache::find_blob(pc);
  _deopt_state = not_deoptimized;
  if (_cb != NULL && _cb->is_nmethod() && ((nmethod*)_cb)->is_deopt_pc(_pc)) {
    _pc = (((nmethod*)_cb)->get_original_pc(this));
    _deopt_state = is_deoptimized;
  } else {
    _deopt_state = not_deoptimized;
  }
}

inline frame:: frame(jint* sp, jint* unextended_sp, jint* fp, address pc) { 
  _sp = sp;
  _unextended_sp = unextended_sp;
  _fp = fp;
  _pc = pc;
  assert(pc != NULL, "no pc?");
  _cb = CodeCache::find_blob(pc);
  _deopt_state = not_deoptimized;
  if (_cb != NULL && _cb->is_nmethod() && ((nmethod*)_cb)->is_deopt_pc(_pc)) {
    _pc = (((nmethod*)_cb)->get_original_pc(this));
    _deopt_state = is_deoptimized;
  } else {
    _deopt_state = not_deoptimized;
  }
}

inline frame::frame(jint* sp, jint* fp) {
  _sp = sp;
  _unextended_sp = sp;
  _fp = fp;
  _pc = (address)(sp[-1]);
  assert(_pc != NULL, "no pc?");
  _cb = CodeCache::find_blob(_pc);
  // In case of native stubs, the pc retreived here might be 
  // wrong. (the _last_native_pc will have the right value)
  // So do not put add any asserts on the _pc here.

  // QQQ The above comment is wrong and has been wrong for years. This constructor
  // should (and MUST) not be called in that situation. In the native situation
  // the pc should be supplied to the constructor.
  _deopt_state = not_deoptimized;
  if (_cb != NULL && _cb->is_nmethod() && ((nmethod*)_cb)->is_deopt_pc(_pc)) {
    _pc = (((nmethod*)_cb)->get_original_pc(this));
    _deopt_state = is_deoptimized;
  } else {
    _deopt_state = not_deoptimized;
  }
}

// Accessors

inline bool frame::equal(frame other) const {
  bool ret =  sp() == other.sp()
              && unextended_sp() == other.unextended_sp()
              && fp() == other.fp()
              && pc() == other.pc();
  assert(!ret || ret && cb() == other.cb() && _deopt_state == other._deopt_state, "inconsistent construction");
  return ret;
}

// Return unique id for this frame. The id must have a value where we can distinguish
// identity and younger/older relationship. NULL represents an invalid (incomparable)
// frame.
inline intptr_t* frame::id(void) const { return unextended_sp(); }

// Relationals on frames based 
// Return true if the frame is younger (more recent activation) than the frame represented by id
inline bool frame::is_younger(intptr_t* id) const { assert(this->id() != NULL && id != NULL, "NULL frame id");
					            return this->id() < id ; }

// Return true if the frame is older (less recent activation) than the frame represented by id
inline bool frame::is_older(intptr_t* id) const   { assert(this->id() != NULL && id != NULL, "NULL frame id");
					            return this->id() > id ; }



inline intptr_t* frame::link() const              { return (intptr_t*) *(intptr_t **)addr_at(link_offset); }
inline void      frame::set_link(intptr_t* addr)  { *(intptr_t **)addr_at(link_offset) = addr; }


inline intptr_t* frame::unextended_sp() const     { return _unextended_sp; }

inline intptr_t* frame::entry_frame_argument_at(int offset) const {
  // Since an entry frame always calls the interpreter first,
  // the format of the call is always compatible with the interpreter.
  return interpreter_frame_tos_at(offset);
}

// Return address:

inline address* frame::sender_pc_addr()      const { return (address*) addr_at( return_addr_offset); }
inline address  frame::sender_pc()           const { return *sender_pc_addr(); }

// return address of param, zero origin index.
inline address* frame::native_param_addr(int idx) const { return (address*) addr_at( native_frame_initial_param_offset+idx); }

inline intptr_t*    frame::sender_sp()        const { return            addr_at(   sender_sp_offset); }

inline int frame::pd_oop_map_offset_adjustment() const {
  return 0;
}

inline intptr_t** frame::interpreter_frame_locals_addr() const { 
  return (intptr_t**)addr_at(interpreter_frame_locals_offset); 
}

inline intptr_t* frame::interpreter_frame_last_sp() const {
  return *(intptr_t**)addr_at(interpreter_frame_last_sp_offset); 
}

inline intptr_t* frame::interpreter_frame_bcx_addr() const {
  return (intptr_t*)addr_at(interpreter_frame_bcx_offset);
}


inline intptr_t* frame::interpreter_frame_mdx_addr() const {
  return (intptr_t*)addr_at(interpreter_frame_mdx_offset);
}


inline int frame::interpreter_frame_monitor_size() {
  return BasicObjectLock::size();
}


// expression stack
// (the max_stack arguments are used by the GC; see class FrameClosure)

inline intptr_t* frame::interpreter_frame_expression_stack() const {
  intptr_t* monitor_end = (intptr_t*) interpreter_frame_monitor_end();
  return monitor_end-1; 
}


inline jint frame::interpreter_frame_expression_stack_direction() { return -1; }

// top of expression stack
inline intptr_t* frame::interpreter_frame_tos_address() const {
  intptr_t* last_sp = interpreter_frame_last_sp();
  if (last_sp == NULL ) {
    return sp();
  } else {
    // sp() may have been extended by an i2c
    assert(last_sp < fp() && last_sp >= sp(), "bad tos");
    return last_sp;
  }
}


// Method

inline methodOop* frame::interpreter_frame_method_addr() const { 
  return (methodOop*)addr_at(interpreter_frame_method_offset);
}


// Constant pool cache

inline constantPoolCacheOop* frame::interpreter_frame_cache_addr() const {
  return (constantPoolCacheOop*)addr_at(interpreter_frame_cache_offset);
}


// Entry frames

inline JavaCallWrapper* frame::entry_frame_call_wrapper() const { 
 return (JavaCallWrapper*)at(entry_frame_call_wrapper_offset); 
}


// Compiled frames

inline int frame::local_offset_for_compiler(int local_index, int nof_args, int max_nof_locals, int max_nof_monitors) {
  return (nof_args - local_index + (local_index < nof_args ? 1: -1)); 
}

inline int frame::monitor_offset_for_compiler(int local_index, int nof_args, int max_nof_locals, int max_nof_monitors) {
  return local_offset_for_compiler(local_index, nof_args, max_nof_locals, max_nof_monitors);
}

inline int frame::min_local_offset_for_compiler(int nof_args, int max_nof_locals, int max_nof_monitors) {
  return (nof_args - (max_nof_locals + max_nof_monitors*2) - 1);
}

inline bool frame::volatile_across_calls(Register reg) {
  return true;
}



inline oop frame::saved_oop_result(RegisterMap* map) const       { 
  return *((oop*) map->location(eax->as_VMReg()));
}

inline void frame::set_saved_oop_result(RegisterMap* map, oop obj) {
  *((oop*) map->location(eax->as_VMReg())) = obj;
}
