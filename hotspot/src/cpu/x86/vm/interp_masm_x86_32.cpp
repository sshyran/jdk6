#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interp_masm_x86_32.cpp	1.172 07/09/17 09:26:18 JVM"
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

#include "incls/_precompiled.incl"
#include "incls/_interp_masm_x86_32.cpp.incl"


// Implementation of InterpreterMacroAssembler
#ifdef CC_INTERP
void InterpreterMacroAssembler::get_method(Register reg) {
  movl(reg, Address(rbp, -(sizeof(BytecodeInterpreter) + 2 * wordSize)));
  movl(reg, Address(reg, byte_offset_of(BytecodeInterpreter, _method)));
}
#endif // CC_INTERP


#ifndef CC_INTERP
void InterpreterMacroAssembler::call_VM_leaf_base(
  address entry_point,
  int     number_of_arguments
) {
  // interpreter specific
  //
  // Note: No need to save/restore bcp & locals (rsi & rdi) pointer
  //       since these are callee saved registers and no blocking/
  //       GC can happen in leaf calls.
  // Further Note: DO NOT save/restore bcp/locals. If a caller has
  // already saved them so that it can use rsi/rdi as temporaries
  // then a save/restore here will DESTROY the copy the caller
  // saved! There used to be a save_bcp() that only happened in
  // the ASSERT path (no restore_bcp). Which caused bizarre failures
  // when jvm built with ASSERTs.
#ifdef ASSERT
  { Label L;
    cmpl(Address(rbp, frame::interpreter_frame_last_sp_offset * wordSize), NULL_WORD);
    jcc(Assembler::equal, L);
    stop("InterpreterMacroAssembler::call_VM_leaf_base: last_sp != NULL");
    bind(L);
  }
#endif
  // super call
  MacroAssembler::call_VM_leaf_base(entry_point, number_of_arguments);
  // interpreter specific

  // Used to ASSERT that rsi/rdi were equal to frame's bcp/locals
  // but since they may not have been saved (and we don't want to
  // save them here (see note above) the assert is invalid.
}


void InterpreterMacroAssembler::call_VM_base(
  Register oop_result,
  Register java_thread,
  Register last_java_sp,
  address  entry_point,
  int      number_of_arguments,
  bool     check_exceptions
) {
#ifdef ASSERT
  { Label L;
    cmpl(Address(rbp, frame::interpreter_frame_last_sp_offset * wordSize), NULL_WORD);
    jcc(Assembler::equal, L);
    stop("InterpreterMacroAssembler::call_VM_base: last_sp != NULL");
    bind(L);
  }
#endif /* ASSERT */
  // interpreter specific
  //
  // Note: Could avoid restoring locals ptr (callee saved) - however doesn't
  //       really make a difference for these runtime calls, since they are
  //       slow anyway. Btw., bcp must be saved/restored since it may change
  //       due to GC.
  assert(java_thread == noreg , "not expecting a precomputed java thread");
  save_bcp();
  // super call
  MacroAssembler::call_VM_base(oop_result, java_thread, last_java_sp, entry_point, number_of_arguments, check_exceptions);
  // interpreter specific
  restore_bcp();
  restore_locals();
}


void InterpreterMacroAssembler::check_and_handle_popframe(Register java_thread) {
  if (JvmtiExport::can_pop_frame()) {
    Label L;
    // Initiate popframe handling only if it is not already being processed.  If the flag
    // has the popframe_processing bit set, it means that this code is called *during* popframe
    // handling - we don't want to reenter.
    Register pop_cond = java_thread;  // Not clear if any other register is available...
    movl(pop_cond, Address(java_thread, JavaThread::popframe_condition_offset()));
    testl(pop_cond, JavaThread::popframe_pending_bit);
    jcc(Assembler::zero, L);
    testl(pop_cond, JavaThread::popframe_processing_bit);
    jcc(Assembler::notZero, L);   
    // Call Interpreter::remove_activation_preserving_args_entry() to get the
    // address of the same-named entrypoint in the generated interpreter code.
    call_VM_leaf(CAST_FROM_FN_PTR(address, Interpreter::remove_activation_preserving_args_entry));
    jmp(rax);
    bind(L);
    get_thread(java_thread);
  }
}


void InterpreterMacroAssembler::load_earlyret_value(TosState state) {
  get_thread(rcx);
  movl(rcx, Address(rcx, JavaThread::jvmti_thread_state_offset()));
  const Address tos_addr (rcx, JvmtiThreadState::earlyret_tos_offset());
  const Address oop_addr (rcx, JvmtiThreadState::earlyret_oop_offset());
  const Address val_addr (rcx, JvmtiThreadState::earlyret_value_offset());
  const Address val_addr1(rcx, JvmtiThreadState::earlyret_value_offset()
                             + in_ByteSize(wordSize));
  switch (state) {
    case atos: movl(rax, oop_addr);
               movl(oop_addr, NULL_WORD);
               verify_oop(rax, state);                break;
    case ltos: movl(rdx, val_addr1);               // fall through
    case btos:                                     // fall through
    case ctos:                                     // fall through
    case stos:                                     // fall through
    case itos: movl(rax, val_addr);                   break;
    case ftos: fld_s(val_addr);                       break;
    case dtos: fld_d(val_addr);                       break;
    case vtos: /* nothing to do */                    break;
    default  : ShouldNotReachHere();
  }
  // Clean up tos value in the thread object
  movl(tos_addr,  (int) ilgl);
  movl(val_addr,  NULL_WORD);
  movl(val_addr1, NULL_WORD);
}


void InterpreterMacroAssembler::check_and_handle_earlyret(Register java_thread) {
  if (JvmtiExport::can_force_early_return()) {
    Label L;
    Register tmp = java_thread;
    movl(tmp, Address(tmp, JavaThread::jvmti_thread_state_offset()));
    testl(tmp, tmp);
    jcc(Assembler::zero, L); // if (thread->jvmti_thread_state() == NULL) exit;

    // Initiate earlyret handling only if it is not already being processed.
    // If the flag has the earlyret_processing bit set, it means that this code
    // is called *during* earlyret handling - we don't want to reenter.
    movl(tmp, Address(tmp, JvmtiThreadState::earlyret_state_offset()));
    cmpl(tmp, JvmtiThreadState::earlyret_pending);
    jcc(Assembler::notEqual, L);

    // Call Interpreter::remove_activation_early_entry() to get the address of the
    // same-named entrypoint in the generated interpreter code.
    get_thread(java_thread);
    movl(tmp, Address(java_thread, JavaThread::jvmti_thread_state_offset()));
    pushl(Address(tmp, JvmtiThreadState::earlyret_tos_offset()));
    call_VM_leaf(CAST_FROM_FN_PTR(address, Interpreter::remove_activation_early_entry), 1);
    jmp(rax);
    bind(L);
    get_thread(java_thread);
  }
}


void InterpreterMacroAssembler::get_unsigned_2_byte_index_at_bcp(Register reg, int bcp_offset) {
  assert(bcp_offset >= 0, "bcp is still pointing to start of bytecode");
  movl(reg, Address(rsi, bcp_offset));
  bswap(reg);
  shrl(reg, 16);
}


void InterpreterMacroAssembler::get_cache_and_index_at_bcp(Register cache, Register index, int bcp_offset) {
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert(cache != index, "must use different registers");
  load_unsigned_word(index, Address(rsi, bcp_offset));
  movl(cache, Address(rbp, frame::interpreter_frame_cache_offset * wordSize));
  assert(sizeof(ConstantPoolCacheEntry) == 4*wordSize, "adjust code below");
  shll(index, 2); // convert from field index to ConstantPoolCacheEntry index
}


void InterpreterMacroAssembler::get_cache_entry_pointer_at_bcp(Register cache, Register tmp, int bcp_offset) {
  assert(bcp_offset > 0, "bcp is still pointing to start of bytecode");
  assert(cache != tmp, "must use different register");
  load_unsigned_word(tmp, Address(rsi, bcp_offset));
  assert(sizeof(ConstantPoolCacheEntry) == 4*wordSize, "adjust code below");
                               // convert from field index to ConstantPoolCacheEntry index
                               // and from word offset to byte offset
  shll(tmp, 2 + LogBytesPerWord);
  movl(cache, Address(rbp, frame::interpreter_frame_cache_offset * wordSize));
                               // skip past the header
  addl(cache, in_bytes(constantPoolCacheOopDesc::base_offset()));
  addl(cache, tmp);            // construct pointer to cache entry
}


  // Generate a subtype check: branch to ok_is_subtype if sub_klass is
  // a subtype of super_klass.  EAX holds the super_klass.  Blows ECX.
  // Resets EDI to locals.  Register sub_klass cannot be any of the above.
void InterpreterMacroAssembler::gen_subtype_check( Register Rsub_klass, Label &ok_is_subtype ) {
  assert( Rsub_klass != rax, "rax, holds superklass" );
  assert( Rsub_klass != rcx, "rcx holds 2ndary super array length" );
  assert( Rsub_klass != rdi, "rdi holds 2ndary super array scan ptr" );
  Label not_subtype, loop;

  // Profile the not-null value's klass.
  profile_typecheck(rcx, Rsub_klass, rdi); // blows rcx, rdi

  // Load the super-klass's check offset into ECX
  movl( rcx, Address(rax, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes() ) );
  // Load from the sub-klass's super-class display list, or a 1-word cache of
  // the secondary superclass list, or a failing value with a sentinel offset
  // if the super-klass is an interface or exceptionally deep in the Java
  // hierarchy and we have to scan the secondary superclass list the hard way.
  // See if we get an immediate positive hit
  cmpl( rax, Address(Rsub_klass,rcx,Address::times_1) );
  jcc( Assembler::equal,ok_is_subtype );

  // Check for immediate negative hit
  cmpl( rcx, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes() );
  jcc( Assembler::notEqual, not_subtype );
  // Check for self
  cmpl( Rsub_klass, rax );
  jcc( Assembler::equal, ok_is_subtype );

  // Now do a linear scan of the secondary super-klass chain.
  movl( rdi, Address(Rsub_klass, sizeof(oopDesc) + Klass::secondary_supers_offset_in_bytes()) );
  // EDI holds the objArrayOop of secondary supers.
  movl( rcx, Address(rdi, arrayOopDesc::length_offset_in_bytes()));// Load the array length
  // Skip to start of data; also clear Z flag incase ECX is zero
  addl( rdi, arrayOopDesc::base_offset_in_bytes(T_OBJECT) );
  // Scan ECX words at [EDI] for occurance of EAX
  // Set NZ/Z based on last compare
  repne_scan();
  restore_locals();           // Restore EDI; Must not blow flags
  // Not equal?
  jcc( Assembler::notEqual, not_subtype );
  // Must be equal but missed in cache.  Update cache.
  movl( Address(Rsub_klass, sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes()), rax );
  jmp( ok_is_subtype );

  bind(not_subtype);
  profile_typecheck_failed(rcx); // blows rcx
}

void InterpreterMacroAssembler::f2ieee() {
  if (IEEEPrecision) {
    fstp_s(Address(rsp, 0));
    fld_s(Address(rsp, 0));
  }
}


void InterpreterMacroAssembler::d2ieee() {
  if (IEEEPrecision) {
    fstp_d(Address(rsp, 0));
    fld_d(Address(rsp, 0));
  }
}
#endif // CC_INTERP

// Java Expression Stack

#ifdef ASSERT
void InterpreterMacroAssembler::verify_stack_tag(frame::Tag t) {
  if (TaggedStackInterpreter) {
    Label okay;
    cmpl(Address(rsp, wordSize), (int)t);
    jcc(Assembler::equal, okay);
    // Also compare if the stack value is zero, then the tag might
    // not have been set coming from deopt.
    cmpl(Address(rsp, 0), 0);
    jcc(Assembler::equal, okay);
    stop("Java Expression stack tag value is bad");
    bind(okay);
  }
}
#endif // ASSERT

void InterpreterMacroAssembler::pop_ptr(Register r) {
  debug_only(verify_stack_tag(frame::TagReference));
  popl(r);
  if (TaggedStackInterpreter) addl(rsp, 1 * wordSize);
}

void InterpreterMacroAssembler::pop_ptr(Register r, Register tag) {
  popl(r);
  // Tag may not be reference for jsr, can be returnAddress
  if (TaggedStackInterpreter) popl(tag);
}

void InterpreterMacroAssembler::pop_i(Register r) {
  debug_only(verify_stack_tag(frame::TagValue));
  popl(r);
  if (TaggedStackInterpreter) addl(rsp, 1 * wordSize);
}

void InterpreterMacroAssembler::pop_l(Register lo, Register hi) {
  debug_only(verify_stack_tag(frame::TagValue));
  popl(lo);
  if (TaggedStackInterpreter) addl(rsp, 1 * wordSize);
  debug_only(verify_stack_tag(frame::TagValue));
  popl(hi);
  if (TaggedStackInterpreter) addl(rsp, 1 * wordSize);
}

void InterpreterMacroAssembler::pop_f() {
  debug_only(verify_stack_tag(frame::TagValue));
  fld_s(Address(rsp, 0));
  addl(rsp, 1 * wordSize);
  if (TaggedStackInterpreter) addl(rsp, 1 * wordSize);
}

void InterpreterMacroAssembler::pop_d() {
  // Write double to stack contiguously and load into ST0
  pop_dtos_to_rsp();
  fld_d(Address(rsp, 0));
  addl(rsp, 2 * wordSize);
}


// Pop the top of the java expression stack to execution stack (which
// happens to be the same place).
void InterpreterMacroAssembler::pop_dtos_to_rsp() {
  if (TaggedStackInterpreter) {
    // Pop double value into scratch registers
    debug_only(verify_stack_tag(frame::TagValue));
    popl(rax);
    addl(rsp, 1* wordSize);
    debug_only(verify_stack_tag(frame::TagValue));
    popl(rdx);
    addl(rsp, 1* wordSize);
    pushl(rdx);
    pushl(rax);
  }
}

void InterpreterMacroAssembler::pop_ftos_to_rsp() {
  if (TaggedStackInterpreter) {
    debug_only(verify_stack_tag(frame::TagValue));
    popl(rax);
    addl(rsp, 1 * wordSize);
    pushl(rax);  // ftos is at rsp
  }
}

void InterpreterMacroAssembler::pop(TosState state) {
  switch (state) {
    case atos: pop_ptr(rax);                                 break;
    case btos:						     // fall through
    case ctos:						     // fall through
    case stos:						     // fall through
    case itos: pop_i(rax);                                   break;
    case ltos: pop_l(rax, rdx);                              break;
    case ftos: pop_f();                                      break;
    case dtos: pop_d();                                      break;
    case vtos: /* nothing to do */                           break;
    default  : ShouldNotReachHere();
  }
  verify_oop(rax, state);
}

void InterpreterMacroAssembler::push_ptr(Register r) {
  if (TaggedStackInterpreter) pushl(frame::TagReference);
  pushl(r);
}

void InterpreterMacroAssembler::push_ptr(Register r, Register tag) {
  if (TaggedStackInterpreter) pushl(tag);  // tag first
  pushl(r);
}

void InterpreterMacroAssembler::push_i(Register r) {
  if (TaggedStackInterpreter) pushl(frame::TagValue);
  pushl(r);
}

void InterpreterMacroAssembler::push_l(Register lo, Register hi) {
  if (TaggedStackInterpreter) pushl(frame::TagValue);
  pushl(hi);
  if (TaggedStackInterpreter) pushl(frame::TagValue);
  pushl(lo);
}

void InterpreterMacroAssembler::push_f() {
  if (TaggedStackInterpreter) pushl(frame::TagValue);
  // Do not schedule for no AGI! Never write beyond rsp!
  subl(rsp, 1 * wordSize);
  fstp_s(Address(rsp, 0));
}

void InterpreterMacroAssembler::push_d(Register r) {
  if (TaggedStackInterpreter) {
    // Double values are stored as:
    //   tag
    //   high
    //   tag
    //   low
    pushl(frame::TagValue);
    subl(rsp, 3 * wordSize);
    fstp_d(Address(rsp, 0));
    // move high word up to slot n-1
    movl(r, Address(rsp, 1*wordSize));
    movl(Address(rsp, 2*wordSize), r);
    // move tag
    movl(Address(rsp, 1*wordSize), frame::TagValue);
  } else {
    // Do not schedule for no AGI! Never write beyond rsp!
    subl(rsp, 2 * wordSize);
    fstp_d(Address(rsp, 0));
  }
}


void InterpreterMacroAssembler::push(TosState state) {
  verify_oop(rax, state);
  switch (state) {
    case atos: push_ptr(rax); break;
    case btos:						     // fall through
    case ctos:						     // fall through
    case stos:						     // fall through
    case itos: push_i(rax);                                    break;
    case ltos: push_l(rax, rdx);                               break;
    case ftos: push_f();                                       break;
    case dtos: push_d(rax);                                    break;
    case vtos: /* nothing to do */                             break;
    default  : ShouldNotReachHere();
  }
}

#ifndef CC_INTERP

// Tagged stack helpers for swap and dup
void InterpreterMacroAssembler::load_ptr_and_tag(int n, Register val,
                                                 Register tag) {
  movl(val, Address(rsp, Interpreter::expr_offset_in_bytes(n)));
  if (TaggedStackInterpreter) {
    movl(tag, Address(rsp, Interpreter::expr_tag_offset_in_bytes(n)));
  }
}

void InterpreterMacroAssembler::store_ptr_and_tag(int n, Register val,
                                                  Register tag) {
  movl(Address(rsp, Interpreter::expr_offset_in_bytes(n)), val);
  if (TaggedStackInterpreter) {
    movl(Address(rsp, Interpreter::expr_tag_offset_in_bytes(n)), tag);
  }
}


// Tagged local support
void InterpreterMacroAssembler::tag_local(frame::Tag tag, int n) {
  if (TaggedStackInterpreter) {
    if (tag == frame::TagCategory2) {
      movl(Address(rdi, Interpreter::local_tag_offset_in_bytes(n+1)), (int)frame::TagValue);
      movl(Address(rdi, Interpreter::local_tag_offset_in_bytes(n)), (int)frame::TagValue);
    } else {
      movl(Address(rdi, Interpreter::local_tag_offset_in_bytes(n)), (int)tag);
    }
  }
}

void InterpreterMacroAssembler::tag_local(frame::Tag tag, Register idx) {
  if (TaggedStackInterpreter) {
    if (tag == frame::TagCategory2) {
      movl(Address(rdi, idx, Interpreter::stackElementScale(),
                  Interpreter::local_tag_offset_in_bytes(1)), (int)frame::TagValue);
      movl(Address(rdi, idx, Interpreter::stackElementScale(),
                  Interpreter::local_tag_offset_in_bytes(0)), (int)frame::TagValue);
    } else {
      movl(Address(rdi, idx, Interpreter::stackElementScale(),
                             Interpreter::local_tag_offset_in_bytes(0)), (int)tag);
    }
  }
}

void InterpreterMacroAssembler::tag_local(Register tag, Register idx) {
  if (TaggedStackInterpreter) {
    // can only be TagValue or TagReference
    movl(Address(rdi, idx, Interpreter::stackElementScale(),
                           Interpreter::local_tag_offset_in_bytes(0)), tag);
  }
}


void InterpreterMacroAssembler::tag_local(Register tag, int n) {
  if (TaggedStackInterpreter) {
    // can only be TagValue or TagReference
    movl(Address(rdi, Interpreter::local_tag_offset_in_bytes(n)), tag);
  }
}
 
#ifdef ASSERT
void InterpreterMacroAssembler::verify_local_tag(frame::Tag tag, int n) {
  if (TaggedStackInterpreter) {
     frame::Tag t = tag;
    if (tag == frame::TagCategory2) {
      Label nbl;
      t = frame::TagValue;  // change to what is stored in locals
      cmpl(Address(rdi, Interpreter::local_tag_offset_in_bytes(n+1)), (int)t);
      jcc(Assembler::equal, nbl);
      stop("Local tag is bad for long/double");
      bind(nbl);
    }
    Label notBad;
    cmpl(Address(rdi, Interpreter::local_tag_offset_in_bytes(n)), (int)t);
    jcc(Assembler::equal, notBad);
    // Also compare if the local value is zero, then the tag might
    // not have been set coming from deopt.
    cmpl(Address(rdi, Interpreter::local_offset_in_bytes(n)), 0);
    jcc(Assembler::equal, notBad);
    stop("Local tag is bad");
    bind(notBad);
  }
}

void InterpreterMacroAssembler::verify_local_tag(frame::Tag tag, Register idx) {
  if (TaggedStackInterpreter) {
    frame::Tag t = tag;
    if (tag == frame::TagCategory2) {
      Label nbl;
      t = frame::TagValue;  // change to what is stored in locals
      cmpl(Address(rdi, idx, Interpreter::stackElementScale(),
                  Interpreter::local_tag_offset_in_bytes(1)), (int)t);
      jcc(Assembler::equal, nbl);
      stop("Local tag is bad for long/double");
      bind(nbl);
    }
    Label notBad;
    cmpl(Address(rdi, idx, Interpreter::stackElementScale(),
                  Interpreter::local_tag_offset_in_bytes(0)), (int)t);
    jcc(Assembler::equal, notBad);
    // Also compare if the local value is zero, then the tag might
    // not have been set coming from deopt.
    cmpl(Address(rdi, idx, Interpreter::stackElementScale(),
                  Interpreter::local_offset_in_bytes(0)), 0);
    jcc(Assembler::equal, notBad);
    stop("Local tag is bad");
    bind(notBad);
    
  }
}
#endif // ASSERT

void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point) {
  MacroAssembler::call_VM_leaf_base(entry_point, 0);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point, Register arg_1) {
  pushl(arg_1);  
  MacroAssembler::call_VM_leaf_base(entry_point, 1);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point, Register arg_1, Register arg_2) {
  pushl(arg_2);
  pushl(arg_1);
  MacroAssembler::call_VM_leaf_base(entry_point, 2);
}


void InterpreterMacroAssembler::super_call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3) {
  pushl(arg_3);
  pushl(arg_2);
  pushl(arg_1);
  MacroAssembler::call_VM_leaf_base(entry_point, 3);
}


// Jump to from_interpreted entry of a call unless single stepping is possible
// in this thread in which case we must call the i2i entry
void InterpreterMacroAssembler::jump_from_interpreted(Register method, Register temp) {
  // set sender sp
  leal(rsi, Address(rsp, wordSize));
  // record last_sp
  movl(Address(rbp, frame::interpreter_frame_last_sp_offset * wordSize), rsi);

  if (JvmtiExport::can_post_interpreter_events()) {
    Label run_compiled_code;
    // JVMTI events, such as single-stepping, are implemented partly by avoiding running
    // compiled code in threads for which the event is enabled.  Check here for
    // interp_only_mode if these events CAN be enabled.
    get_thread(temp);
    // interp_only is an int, on little endian it is sufficient to test the byte only
    // Is a cmpl faster (ce
    cmpb(Address(temp, JavaThread::interp_only_mode_offset()), 0);
    jcc(Assembler::zero, run_compiled_code);
    jmp(Address(method, methodOopDesc::interpreter_entry_offset()));
    bind(run_compiled_code);
  }

  jmp(Address(method, methodOopDesc::from_interpreted_offset()));

}


// The following two routines provide a hook so that an implementation
// can schedule the dispatch in two parts.  Intel does not do this.
void InterpreterMacroAssembler::dispatch_prolog(TosState state, int step) {
  // Nothing Intel-specific to be done here.
}

void InterpreterMacroAssembler::dispatch_epilog(TosState state, int step) {
  dispatch_next(state, step);
}

void InterpreterMacroAssembler::dispatch_base(TosState state, address* table,
                                              bool verifyoop) {
  verify_FPU(1, state);
  if (VerifyActivationFrameSize) {
    Label L;
    movl(rcx, rbp);
    subl(rcx, rsp);
    int min_frame_size = (frame::link_offset - frame::interpreter_frame_initial_sp_offset) * wordSize;
    cmpl(rcx, min_frame_size);
    jcc(Assembler::greaterEqual, L);
    stop("broken stack frame");
    bind(L);
  }
  if (verifyoop) verify_oop(rax, state);
  Address index(noreg, rbx, Address::times_4);
  ExternalAddress tbl((address)table);
  ArrayAddress dispatch(tbl, index);
  jump(dispatch);
}


void InterpreterMacroAssembler::dispatch_only(TosState state) {
  dispatch_base(state, Interpreter::dispatch_table(state));
}


void InterpreterMacroAssembler::dispatch_only_normal(TosState state) {
  dispatch_base(state, Interpreter::normal_table(state));
}

void InterpreterMacroAssembler::dispatch_only_noverify(TosState state) {
  dispatch_base(state, Interpreter::normal_table(state), false);
}


void InterpreterMacroAssembler::dispatch_next(TosState state, int step) {
  // load next bytecode (load before advancing rsi to prevent AGI)
  load_unsigned_byte(rbx, Address(rsi, step));
  // advance rsi
  increment(rsi, step);
  dispatch_base(state, Interpreter::dispatch_table(state));
}


void InterpreterMacroAssembler::dispatch_via(TosState state, address* table) {
  // load current bytecode
  load_unsigned_byte(rbx, Address(rsi, 0));
  dispatch_base(state, table);
}

// remove activation
//
// Unlock the receiver if this is a synchronized method.
// Unlock any Java monitors from syncronized blocks.
// Remove the activation from the stack.
//
// If there are locked Java monitors
//    If throw_monitor_exception
//       throws IllegalMonitorStateException
//    Else if install_monitor_exception
//       installs IllegalMonitorStateException
//    Else
//       no error processing
void InterpreterMacroAssembler::remove_activation(TosState state, Register ret_addr,
                                                  bool throw_monitor_exception,
                                                  bool install_monitor_exception,
                                                  bool notify_jvmdi) {
  // Note: Registers rax, rdx and FPU ST(0) may be in use for the result
  // check if synchronized method  
  Label unlocked, unlock, no_unlock;

  get_thread(rcx);
  const Address do_not_unlock_if_synchronized(rcx,
    in_bytes(JavaThread::do_not_unlock_if_synchronized_offset()));

  movbool(rbx, do_not_unlock_if_synchronized);
  movl(rdi,rbx);
  movbool(do_not_unlock_if_synchronized, false); // reset the flag

  movl(rbx, Address(rbp, frame::interpreter_frame_method_offset * wordSize)); // get method access flags
  movl(rcx, Address(rbx, methodOopDesc::access_flags_offset()));

  testl(rcx, JVM_ACC_SYNCHRONIZED);
  jcc(Assembler::zero, unlocked);
 
  // Don't unlock anything if the _do_not_unlock_if_synchronized flag
  // is set.
  movl(rcx,rdi);
  testbool(rcx);
  jcc(Assembler::notZero, no_unlock);

  // unlock monitor
  push(state);                                   // save result
    
  // BasicObjectLock will be first in list, since this is a synchronized method. However, need
  // to check that the object has not been unlocked by an explicit monitorexit bytecode.  
  const Address monitor(rbp, frame::interpreter_frame_initial_sp_offset * wordSize - (int)sizeof(BasicObjectLock));
  leal  (rdx, monitor);                          // address of first monitor
  
  movl  (rax, Address(rdx, BasicObjectLock::obj_offset_in_bytes()));
  testl (rax, rax);
  jcc   (Assembler::notZero, unlock);
                                  
  pop(state);
  if (throw_monitor_exception) {
    empty_FPU_stack();  // remove possible return value from FPU-stack, otherwise stack could overflow

    // Entry already unlocked, need to throw exception
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
    should_not_reach_here();
  } else {
    // Monitor already unlocked during a stack unroll. 
    // If requested, install an illegal_monitor_state_exception.
    // Continue with stack unrolling.
    if (install_monitor_exception) {
      empty_FPU_stack();  // remove possible return value from FPU-stack, otherwise stack could overflow
      call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::new_illegal_monitor_state_exception));
    }
    jmp(unlocked);
  }

  bind(unlock);  
  unlock_object(rdx);              
  pop(state);

  // Check that for block-structured locking (i.e., that all locked objects has been unlocked)  
  bind(unlocked);  

  // rax, rdx: Might contain return value

  // Check that all monitors are unlocked
  {
    Label loop, exception, entry, restart;
    const int entry_size               = frame::interpreter_frame_monitor_size()           * wordSize;
    const Address monitor_block_top(rbp, frame::interpreter_frame_monitor_block_top_offset * wordSize);
    const Address monitor_block_bot(rbp, frame::interpreter_frame_initial_sp_offset        * wordSize);
    
    bind(restart);
    movl(rcx, monitor_block_top);             // points to current entry, starting with top-most entry
    leal(rbx, monitor_block_bot);             // points to word before bottom of monitor block
    jmp(entry);
          
    // Entry already locked, need to throw exception
    bind(exception); 

    if (throw_monitor_exception) {
      empty_FPU_stack();  // remove possible return value from FPU-stack, otherwise stack could overflow

      // Throw exception      
      call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::throw_illegal_monitor_state_exception));
      should_not_reach_here();
    } else {
      // Stack unrolling. Unlock object and install illegal_monitor_exception
      // Unlock does not block, so don't have to worry about the frame

      push(state);
      movl(rdx, rcx);
      unlock_object(rdx);
      pop(state);
      
      if (install_monitor_exception) {
        empty_FPU_stack();  // remove possible return value from FPU-stack, otherwise stack could overflow
        call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::new_illegal_monitor_state_exception));
      }

      jmp(restart);
    }
  
    bind(loop);
    cmpl(Address(rcx, BasicObjectLock::obj_offset_in_bytes()), NULL_WORD);  // check if current entry is used
    jcc(Assembler::notEqual, exception);
          
    addl(rcx, entry_size);                       // otherwise advance to next entry
    bind(entry);
    cmpl(rcx, rbx);                              // check if bottom reached
    jcc(Assembler::notEqual, loop);              // if not at bottom then check this entry      
  }        

  bind(no_unlock);

  // jvmti support
  if (notify_jvmdi) {
    notify_method_exit(state, NotifyJVMTI);     // preserve TOSCA
  } else {
    notify_method_exit(state, SkipNotifyJVMTI); // preserve TOSCA
  }

  // remove activation
  movl(rbx, Address(rbp, frame::interpreter_frame_sender_sp_offset * wordSize)); // get sender sp
  leave();                                     // remove frame anchor
  popl(ret_addr);                              // get return address
  movl(rsp, rbx);                              // set sp to sender sp
  if (UseSSE) {
    // float and double are returned in xmm register in SSE-mode
    if (state == ftos && UseSSE >= 1) {
      subl(rsp, wordSize);
      fstp_s(Address(rsp, 0));
      movflt(xmm0, Address(rsp, 0));
      addl(rsp, wordSize);
    } else if (state == dtos && UseSSE >= 2) {
      subl(rsp, 2*wordSize);
      fstp_d(Address(rsp, 0));
      movdbl(xmm0, Address(rsp, 0));
      addl(rsp, 2*wordSize);
    }
  }
}

#endif /* !CC_INTERP */


// Lock object
//
// Argument: rdx : Points to BasicObjectLock to be used for locking. Must
// be initialized with object to lock
void InterpreterMacroAssembler::lock_object(Register lock_reg) {
  assert(lock_reg == rdx, "The argument is only for looks. It must be rdx");

  if (UseHeavyMonitors) {
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), lock_reg);
  } else {

    Label done;

    const Register swap_reg = rax;  // Must use rax, for cmpxchg instruction
    const Register obj_reg  = rcx;  // Will contain the oop

    const int obj_offset = BasicObjectLock::obj_offset_in_bytes();
    const int lock_offset = BasicObjectLock::lock_offset_in_bytes ();
    const int mark_offset = lock_offset + BasicLock::displaced_header_offset_in_bytes(); 

    Label slow_case;
    
    // Load object pointer into obj_reg %rcx
    movl(obj_reg, Address(lock_reg, obj_offset));
    
    if (UseBiasedLocking) {
      // Note: we use noreg for the temporary register since it's hard
      // to come up with a free register on all incoming code paths
      biased_locking_enter(lock_reg, obj_reg, swap_reg, noreg, false, done, &slow_case);
    }

    // Load immediate 1 into swap_reg %rax,
    movl(swap_reg, 1);

    // Load (object->mark() | 1) into swap_reg %rax,
    orl(swap_reg, Address(obj_reg, 0));

    // Save (object->mark() | 1) into BasicLock's displaced header
    movl(Address(lock_reg, mark_offset), swap_reg);

    assert(lock_offset == 0, "displached header must be first word in BasicObjectLock");
    if (os::is_MP()) {
      lock();
    }    
    cmpxchg(lock_reg, Address(obj_reg, 0));    
    if (PrintBiasedLockingStatistics) {
      cond_inc32(Assembler::zero,
                 ExternalAddress((address) BiasedLocking::fast_path_entry_count_addr()));
    }
    jcc(Assembler::zero, done);

    // Test if the oopMark is an obvious stack pointer, i.e.,
    //  1) (mark & 3) == 0, and
    //  2) rsp <= mark < mark + os::pagesize()
    //
    // These 3 tests can be done by evaluating the following 
    // expression: ((mark - rsp) & (3 - os::vm_page_size())),
    // assuming both stack pointer and pagesize have their
    // least significant 2 bits clear.
    // NOTE: the oopMark is in swap_reg %rax, as the result of cmpxchg
    subl(swap_reg, rsp);
    andl(swap_reg, 3 - os::vm_page_size());

    // Save the test result, for recursive case, the result is zero
    movl(Address(lock_reg, mark_offset), swap_reg);

    if (PrintBiasedLockingStatistics) {
      cond_inc32(Assembler::zero,
                 ExternalAddress((address) BiasedLocking::fast_path_entry_count_addr()));
    }
    jcc(Assembler::zero, done);

    bind(slow_case);

    // Call the runtime routine for slow case
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorenter), lock_reg);

    bind(done);
  }   
}


// Unlocks an object. Used in monitorexit bytecode and remove_activation.
//
// Argument: rdx : Points to BasicObjectLock structure for lock
// Throw an IllegalMonitorException if object is not locked by current thread
// 
// Uses: rax, rbx, rcx, rdx
void InterpreterMacroAssembler::unlock_object(Register lock_reg) {
  assert(lock_reg == rdx, "The argument is only for looks. It must be rdx");

  if (UseHeavyMonitors) {
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), lock_reg);
  } else {
    Label done;

    const Register swap_reg   = rax;  // Must use rax, for cmpxchg instruction
    const Register header_reg = rbx;  // Will contain the old oopMark
    const Register obj_reg    = rcx;  // Will contain the oop

    save_bcp(); // Save in case of exception

    // Convert from BasicObjectLock structure to object and BasicLock structure
    // Store the BasicLock address into %rax,
    leal(swap_reg, Address(lock_reg, BasicObjectLock::lock_offset_in_bytes()));

    // Load oop into obj_reg(%rcx)
    movl(obj_reg, Address(lock_reg, BasicObjectLock::obj_offset_in_bytes ()));

    // Free entry
    movl(Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()), NULL_WORD);

    if (UseBiasedLocking) {
      biased_locking_exit(obj_reg, header_reg, done);
    }

    // Load the old header from BasicLock structure
    movl(header_reg, Address(swap_reg, BasicLock::displaced_header_offset_in_bytes()));

    // Test for recursion
    testl(header_reg, header_reg);

    // zero for recursive case
    jcc(Assembler::zero, done);
    
    // Atomic swap back the old header
    if (os::is_MP()) lock();
    cmpxchg(header_reg, Address(obj_reg, 0));

    // zero for recursive case
    jcc(Assembler::zero, done);

    // Call the runtime routine for slow case.
    movl(Address(lock_reg, BasicObjectLock::obj_offset_in_bytes()), obj_reg); // restore obj
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::monitorexit), lock_reg);

    bind(done);

    restore_bcp();
  }
}


#ifndef CC_INTERP

// Test ImethodDataPtr.  If it is null, continue at the specified label
void InterpreterMacroAssembler::test_method_data_pointer(Register mdp, Label& zero_continue) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  movl(mdp, Address(rbp, frame::interpreter_frame_mdx_offset * wordSize));
  testl(mdp, mdp);
  jcc(Assembler::zero, zero_continue);
}


// Set the method data pointer for the current bcp.
void InterpreterMacroAssembler::set_method_data_pointer_for_bcp() {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Label zero_continue;
  pushl(rax);
  pushl(rbx);

  get_method(rbx);
  // Test MDO to avoid the call if it is NULL.
  movl(rax, Address(rbx, in_bytes(methodOopDesc::method_data_offset())));
  testl(rax, rax);
  jcc(Assembler::zero, zero_continue);

  // rbx,: method
  // rsi: bcp
  call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::bcp_to_di), rbx, rsi);
  // rax,: mdi

  movl(rbx, Address(rbx, in_bytes(methodOopDesc::method_data_offset())));
  testl(rbx, rbx);
  jcc(Assembler::zero, zero_continue);
  addl(rbx, in_bytes(methodDataOopDesc::data_offset()));
  addl(rbx, rax);
  movl(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), rbx);

  bind(zero_continue);
  popl(rbx);
  popl(rax);
}

void InterpreterMacroAssembler::verify_method_data_pointer() {
  assert(ProfileInterpreter, "must be profiling interpreter");
#ifdef ASSERT
  Label verify_continue;
  pushl(rax);
  pushl(rbx);
  pushl(rcx);
  pushl(rdx);
  test_method_data_pointer(rcx, verify_continue); // If mdp is zero, continue
  get_method(rbx);

  // If the mdp is valid, it will point to a DataLayout header which is
  // consistent with the bcp.  The converse is highly probable also.
  load_unsigned_word(rdx, Address(rcx, in_bytes(DataLayout::bci_offset())));
  addl(rdx, Address(rbx, methodOopDesc::const_offset()));
  leal(rdx, Address(rdx, constMethodOopDesc::codes_offset()));
  cmpl(rdx, rsi);
  jcc(Assembler::equal, verify_continue);
  // rbx,: method
  // rsi: bcp
  // rcx: mdp
  call_VM_leaf(CAST_FROM_FN_PTR(address, InterpreterRuntime::verify_mdp), rbx, rsi, rcx);
  bind(verify_continue);
  popl(rdx);
  popl(rcx);
  popl(rbx);
  popl(rax);
#endif // ASSERT
}


void InterpreterMacroAssembler::set_mdp_data_at(Register mdp_in, int constant, Register value) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address data(mdp_in, constant);
  movl(data, value);
}


void InterpreterMacroAssembler::increment_mdp_data_at(Register mdp_in,
                                                      int constant,
                                                      bool decrement) {
  // Counter address
  Address data(mdp_in, constant);

  increment_mdp_data_at(data, decrement);
}


void InterpreterMacroAssembler::increment_mdp_data_at(Address data,
                                                      bool decrement) {

  assert( DataLayout::counter_increment==1, "flow-free idiom only works with 1" );
  assert(ProfileInterpreter, "must be profiling interpreter");

  if (decrement) {
    // Decrement the register.  Set condition codes.
    addl(data, -DataLayout::counter_increment);
    // If the decrement causes the counter to overflow, stay negative
    Label L;
    jcc(Assembler::negative, L);
    addl(data, DataLayout::counter_increment);
    bind(L);
  } else {
    assert(DataLayout::counter_increment == 1,
           "flow-free idiom only works with 1");
    // Increment the register.  Set carry flag.
    addl(data, DataLayout::counter_increment);
    // If the increment causes the counter to overflow, pull back by 1.
    sbbl(data, 0);
  }
}


void InterpreterMacroAssembler::increment_mdp_data_at(Register mdp_in,
                                                      Register reg,
                                                      int constant,
                                                      bool decrement) {
  Address data(mdp_in, reg, Address::times_1, constant);

  increment_mdp_data_at(data, decrement);
}


void InterpreterMacroAssembler::set_mdp_flag_at(Register mdp_in, int flag_byte_constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  int header_offset = in_bytes(DataLayout::header_offset());
  int header_bits = DataLayout::flag_mask_to_header_mask(flag_byte_constant);
  // Set the flag
  orl(Address(mdp_in, header_offset), header_bits);
}



void InterpreterMacroAssembler::test_mdp_data_at(Register mdp_in,
                                                 int offset,
                                                 Register value,
                                                 Register test_value_out,
                                                 Label& not_equal_continue) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  if (test_value_out == noreg) {
    cmpl(value, Address(mdp_in, offset));
  } else {
    // Put the test value into a register, so caller can use it:
    movl(test_value_out, Address(mdp_in, offset));
    cmpl(test_value_out, value);
  }
  jcc(Assembler::notEqual, not_equal_continue);
}


void InterpreterMacroAssembler::update_mdp_by_offset(Register mdp_in, int offset_of_disp) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address disp_address(mdp_in, offset_of_disp);
  addl(mdp_in,disp_address);
  movl(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_by_offset(Register mdp_in, Register reg, int offset_of_disp) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  Address disp_address(mdp_in, reg, Address::times_1, offset_of_disp);
  addl(mdp_in, disp_address);
  movl(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_by_constant(Register mdp_in, int constant) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  addl(mdp_in, constant);
  movl(Address(rbp, frame::interpreter_frame_mdx_offset * wordSize), mdp_in);
}


void InterpreterMacroAssembler::update_mdp_for_ret(Register return_bci) {
  assert(ProfileInterpreter, "must be profiling interpreter");
  pushl(return_bci);             // save/restore across call_VM
  call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::update_mdp_for_ret), return_bci);
  popl(return_bci);
}


void InterpreterMacroAssembler::profile_taken_branch(Register mdp, Register bumped_count) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    // Otherwise, assign to mdp
    test_method_data_pointer(mdp, profile_continue);

    // We are taking a branch.  Increment the taken count.
    // We inline increment_mdp_data_at to return bumped_count in a register
    //increment_mdp_data_at(mdp, in_bytes(JumpData::taken_offset()));
    Address data(mdp, in_bytes(JumpData::taken_offset()));
    movl(bumped_count,data);
    assert( DataLayout::counter_increment==1, "flow-free idiom only works with 1" );
    addl(bumped_count, DataLayout::counter_increment);
    sbbl(bumped_count, 0);
    movl(data,bumped_count);    // Store back out

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_offset(mdp, in_bytes(JumpData::displacement_offset()));
    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_not_taken_branch(Register mdp) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are taking a branch.  Increment the not taken count.
    increment_mdp_data_at(mdp, in_bytes(BranchData::not_taken_offset()));

    // The method data pointer needs to be updated to correspond to the next bytecode
    update_mdp_by_constant(mdp, in_bytes(BranchData::branch_data_size()));
    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_call(Register mdp) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, in_bytes(CounterData::counter_data_size()));
    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_final_call(Register mdp) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, in_bytes(VirtualCallData::virtual_call_data_size()));
    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_virtual_call(Register receiver, Register mdp, Register reg2) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // We are making a call.  Increment the count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    // Record the receiver type.
    record_klass_in_profile(receiver, mdp, reg2);

    // The method data pointer needs to be updated to reflect the new target.
    update_mdp_by_constant(mdp, 
                           in_bytes(VirtualCallData::
                                    virtual_call_data_size()));
    bind(profile_continue);
  }
}


void InterpreterMacroAssembler::record_klass_in_profile_helper(
                                        Register receiver, Register mdp,
                                        Register reg2,
                                        int start_row, Label& done) {
  int last_row = VirtualCallData::row_limit() - 1;
  assert(start_row <= last_row, "must be work left to do");
  // Test this row for both the receiver and for null.
  // Take any of three different outcomes:
  //   1. found receiver => increment count and goto done
  //   2. found null => keep looking for case 1, maybe allocate this cell
  //   3. found something else => keep looking for cases 1 and 2
  // Case 3 is handled by a recursive call.
  for (int row = start_row; row <= last_row; row++) {
    Label next_test;
    bool test_for_null_also = (row == start_row);

    // See if the receiver is receiver[n].
    int recvr_offset = in_bytes(VirtualCallData::receiver_offset(row));
    test_mdp_data_at(mdp, recvr_offset, receiver,
                     (test_for_null_also ? reg2 : noreg),
                     next_test);
    // (Reg2 now contains the receiver from the CallData.)

    // The receiver is receiver[n].  Increment count[n].
    int count_offset = in_bytes(VirtualCallData::receiver_count_offset(row));
    increment_mdp_data_at(mdp, count_offset);
    jmp(done);
    bind(next_test);

    if (row == start_row) {
      // Failed the equality check on receiver[n]...  Test for null.
      testl(reg2, reg2);
      if (start_row == last_row) {
        // The only thing left to do is handle the null case.
        jcc(Assembler::notZero, done);
        break;
      }
      // Since null is rare, make it be the branch-taken case.
      Label found_null;
      jcc(Assembler::zero, found_null);

      // Put all the "Case 3" tests here.
      record_klass_in_profile_helper(receiver, mdp, reg2, start_row + 1, done);

      // Found a null.  Keep searching for a matching receiver,
      // but remember that this is an empty (unused) slot.
      bind(found_null);
    }
  }

  // In the fall-through case, we found no matching receiver, but we
  // observed the receiver[start_row] is NULL.

  // Fill in the receiver field and increment the count.
  int recvr_offset = in_bytes(VirtualCallData::receiver_offset(start_row));
  set_mdp_data_at(mdp, recvr_offset, receiver);
  int count_offset = in_bytes(VirtualCallData::receiver_count_offset(start_row));
  movl(reg2, DataLayout::counter_increment);
  set_mdp_data_at(mdp, count_offset, reg2);
  jmp(done);
}

void InterpreterMacroAssembler::record_klass_in_profile(Register receiver,
                                                        Register mdp,
                                                        Register reg2) {
  assert(ProfileInterpreter, "must be profiling");
  Label done;

  record_klass_in_profile_helper(receiver, mdp, reg2, 0, done);

  bind (done);
}

void InterpreterMacroAssembler::profile_ret(Register return_bci, Register mdp) {
  if (ProfileInterpreter) {
    Label profile_continue;
    uint row;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Update the total ret count.
    increment_mdp_data_at(mdp, in_bytes(CounterData::count_offset()));

    for (row = 0; row < RetData::row_limit(); row++) {
      Label next_test;

      // See if return_bci is equal to bci[n]:
      test_mdp_data_at(mdp, in_bytes(RetData::bci_offset(row)), return_bci,
                       noreg, next_test);

      // return_bci is equal to bci[n].  Increment the count.
      increment_mdp_data_at(mdp, in_bytes(RetData::bci_count_offset(row)));

      // The method data pointer needs to be updated to reflect the new target.
      update_mdp_by_offset(mdp, in_bytes(RetData::bci_displacement_offset(row)));
      jmp(profile_continue);
      bind(next_test);
    }

    update_mdp_for_ret(return_bci);

    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_null_seen(Register mdp) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // The method data pointer needs to be updated.
    int mdp_delta = in_bytes(BitData::bit_data_size());
    if (TypeProfileCasts) {
      mdp_delta = in_bytes(VirtualCallData::virtual_call_data_size());
    }
    update_mdp_by_constant(mdp, mdp_delta);

    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_typecheck_failed(Register mdp) {
  if (ProfileInterpreter && TypeProfileCasts) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    int count_offset = in_bytes(CounterData::count_offset());
    // Back up the address, since we have already bumped the mdp.
    count_offset -= in_bytes(VirtualCallData::virtual_call_data_size());

    // *Decrement* the counter.  We expect to see zero or small negatives.
    increment_mdp_data_at(mdp, count_offset, true);

    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_typecheck(Register mdp, Register klass, Register reg2)
{
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // The method data pointer needs to be updated.
    int mdp_delta = in_bytes(BitData::bit_data_size());
    if (TypeProfileCasts) {
      mdp_delta = in_bytes(VirtualCallData::virtual_call_data_size());

      // Record the object type.
      record_klass_in_profile(klass, mdp, reg2);
      assert(reg2 == rdi, "we know how to fix this blown reg");
      restore_locals();         // Restore EDI
    }
    update_mdp_by_constant(mdp, mdp_delta);

    bind(profile_continue);
  }
}


void InterpreterMacroAssembler::profile_switch_default(Register mdp) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Update the default case count
    increment_mdp_data_at(mdp, in_bytes(MultiBranchData::default_count_offset()));

    // The method data pointer needs to be updated.
    update_mdp_by_offset(mdp, in_bytes(MultiBranchData::default_displacement_offset()));

    bind (profile_continue);
  }
}


void InterpreterMacroAssembler::profile_switch_case(Register index, Register mdp, Register reg2) {
  if (ProfileInterpreter) {
    Label profile_continue;

    // If no method data exists, go to profile_continue.
    test_method_data_pointer(mdp, profile_continue);

    // Build the base (index * per_case_size_in_bytes()) + case_array_offset_in_bytes()  
    movl(reg2, in_bytes(MultiBranchData::per_case_size()));
    imull(index, reg2);
    addl(index, in_bytes(MultiBranchData::case_array_offset()));

    // Update the case count
    increment_mdp_data_at(mdp, index, in_bytes(MultiBranchData::relative_count_offset()));  

    // The method data pointer needs to be updated.
    update_mdp_by_offset(mdp, index, in_bytes(MultiBranchData::relative_displacement_offset()));

    bind (profile_continue);
  }
}

#endif // !CC_INTERP



void InterpreterMacroAssembler::verify_oop(Register reg, TosState state) {
  if (state == atos) MacroAssembler::verify_oop(reg);
}


#ifndef CC_INTERP
void InterpreterMacroAssembler::verify_FPU(int stack_depth, TosState state) {
  if (state == ftos || state == dtos) MacroAssembler::verify_FPU(stack_depth);
}

#endif /* CC_INTERP */

 
void InterpreterMacroAssembler::notify_method_entry() {
  // Whenever JVMTI is interp_only_mode, method entry/exit events are sent to
  // track stack depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (JvmtiExport::can_post_interpreter_events()) {
    Label L;
    get_thread(rcx);
    movl(rcx, Address(rcx, JavaThread::interp_only_mode_offset()));
    testl(rcx,rcx);
    jcc(Assembler::zero, L);
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_entry));
    bind(L);
  }

  {
    SkipIfEqual skip_if(this, &DTraceMethodProbes, 0);
    get_thread(rcx);
    get_method(rbx);
    call_VM_leaf(
      CAST_FROM_FN_PTR(address, SharedRuntime::dtrace_method_entry), rcx, rbx);
  }
}

 
void InterpreterMacroAssembler::notify_method_exit(
    TosState state, NotifyMethodExitMode mode) {
  // Whenever JVMTI is interp_only_mode, method entry/exit events are sent to
  // track stack depth.  If it is possible to enter interp_only_mode we add
  // the code to check if the event should be sent.
  if (mode == NotifyJVMTI && JvmtiExport::can_post_interpreter_events()) {
    Label L;
    // Note: frame::interpreter_frame_result has a dependency on how the 
    // method result is saved across the call to post_method_exit. If this
    // is changed then the interpreter_frame_result implementation will
    // need to be updated too.

    // For c++ interpreter the result is always stored at a known location in the frame
    // template interpreter will leave it on the top of the stack.
    NOT_CC_INTERP(push(state);)
    get_thread(rcx);
    movl(rcx, Address(rcx, JavaThread::interp_only_mode_offset()));
    testl(rcx,rcx);
    jcc(Assembler::zero, L);
    call_VM(noreg, CAST_FROM_FN_PTR(address, InterpreterRuntime::post_method_exit));
    bind(L);
    NOT_CC_INTERP(pop(state);)
  }

  {
    SkipIfEqual skip_if(this, &DTraceMethodProbes, 0);
    push(state);
    get_thread(rbx);
    get_method(rcx);
    call_VM_leaf(
      CAST_FROM_FN_PTR(address, SharedRuntime::dtrace_method_exit),
      rbx, rcx);
    pop(state);
  }
}