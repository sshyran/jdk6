#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "%W% %E% %U% JVM"
#endif
/*
 * Copyright 1998-2007 Sun Microsystems, Inc.  All Rights Reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_vm_version.cpp.incl"

const char* Abstract_VM_Version::_s_vm_release = Abstract_VM_Version::vm_release();
const char* Abstract_VM_Version::_s_internal_vm_info_string = Abstract_VM_Version::internal_vm_info_string();
bool Abstract_VM_Version::_supports_cx8 = false;
unsigned int Abstract_VM_Version::_logical_processors_per_package = 1U;

#ifndef HOTSPOT_RELEASE_VERSION
  #error HOTSPOT_RELEASE_VERSION must be defined
#endif
#ifndef JRE_RELEASE_VERSION
  #error JRE_RELEASE_VERSION must be defined
#endif
#ifndef HOTSPOT_BUILD_TARGET
  #error HOTSPOT_BUILD_TARGET must be defined
#endif

#ifdef PRODUCT
  #define VM_RELEASE HOTSPOT_RELEASE_VERSION
#else
  #define VM_RELEASE HOTSPOT_RELEASE_VERSION "-" HOTSPOT_BUILD_TARGET
#endif

// HOTSPOT_RELEASE_VERSION must follow the release version naming convention 
// <major_ver>.<minor_ver>-b<nn>[-<identifier>][-<debug_target>]
int Abstract_VM_Version::_vm_major_version = 0;
int Abstract_VM_Version::_vm_minor_version = 0;
int Abstract_VM_Version::_vm_build_number = 0;
bool Abstract_VM_Version::_initialized = false;

void Abstract_VM_Version::initialize() {
  if (_initialized) {
    return;
  }
  char* vm_version = os::strdup(HOTSPOT_RELEASE_VERSION);

  // Expecting the next vm_version format: 
  // <major_ver>.<minor_ver>-b<nn>[-<identifier>]
  char* vm_major_ver = vm_version;
  assert(isdigit(vm_major_ver[0]),"wrong vm major version number");
  char* vm_minor_ver = strchr(vm_major_ver, '.');
  assert(vm_minor_ver != NULL && isdigit(vm_minor_ver[1]),"wrong vm minor version number");
  vm_minor_ver[0] = '\0'; // terminate vm_major_ver
  vm_minor_ver += 1;
  char* vm_build_num = strchr(vm_minor_ver, '-');
  assert(vm_build_num != NULL && vm_build_num[1] == 'b' && isdigit(vm_build_num[2]),"wrong vm build number");
  vm_build_num[0] = '\0'; // terminate vm_minor_ver
  vm_build_num += 2;

  _vm_major_version = atoi(vm_major_ver); 
  _vm_minor_version = atoi(vm_minor_ver); 
  _vm_build_number  = atoi(vm_build_num);
 
  os::free(vm_version);
  _initialized = true;
}

#if defined(_LP64)
  #define VMLP "64-Bit "
#else
  #define VMLP ""
#endif

#ifdef KERNEL
  #define VMTYPE "Kernel"
#else // KERNEL
#ifdef TIERED
  #define VMTYPE "Server"
#else
  #define VMTYPE COMPILER1_PRESENT("Client")   \
                 COMPILER2_PRESENT("Server")   
#endif // TIERED
#endif // KERNEL

#ifndef HOTSPOT_VM_DISTRO
  #error HOTSPOT_VM_DISTRO must be defined
#endif
#define VMNAME HOTSPOT_VM_DISTRO " " VMLP VMTYPE " VM"

const char* Abstract_VM_Version::vm_name() {
  return VMNAME;
}


const char* Abstract_VM_Version::vm_vendor() {
#ifdef VENDOR
  return XSTR(VENDOR);
#else
  return "Sun Microsystems Inc.";
#endif
}


const char* Abstract_VM_Version::vm_info_string() {
  switch (Arguments::mode()) {
    case Arguments::_int:
      return UseSharedSpaces ? "interpreted mode, sharing" : "interpreted mode";
    case Arguments::_mixed:
      return UseSharedSpaces ? "mixed mode, sharing"       :  "mixed mode";
    case Arguments::_comp:
      return UseSharedSpaces ? "compiled mode, sharing"    : "compiled mode";
  };
  ShouldNotReachHere();
  return "";
}

// NOTE: do *not* use stringStream. this function is called by 
//       fatal error handler. if the crash is in native thread,
//       stringStream cannot get resource allocated and will SEGV.
const char* Abstract_VM_Version::vm_release() {
  return VM_RELEASE;
}

#define OS       LINUX_ONLY("linux")             \
                 WINDOWS_ONLY("windows")         \
                 SOLARIS_ONLY("solaris")

#define CPU      IA32_ONLY("x86")                \
                 IA64_ONLY("ia64")               \
                 AMD64_ONLY("amd64")             \
                 SPARC_ONLY("sparc")

const char *Abstract_VM_Version::vm_platform_string() {
  return OS "-" CPU;
}

const char* Abstract_VM_Version::internal_vm_info_string() {
  #ifndef HOTSPOT_BUILD_USER
    #define HOTSPOT_BUILD_USER unknown
  #endif

  #ifndef HOTSPOT_BUILD_COMPILER
    #ifdef _MSC_VER
      #if   _MSC_VER == 1100
        #define HOTSPOT_BUILD_COMPILER "MS VC++ 5.0"
      #elif _MSC_VER == 1200
        #define HOTSPOT_BUILD_COMPILER "MS VC++ 6.0"
      #elif _MSC_VER == 1310
        #define HOTSPOT_BUILD_COMPILER "MS VC++ 7.1"
      #elif _MSC_VER == 1400
        #define HOTSPOT_BUILD_COMPILER "MS VC++ 8.0"
      #else
        #define HOTSPOT_BUILD_COMPILER "unknown MS VC++:" XSTR(_MSC_VER)
      #endif
    #elif defined(__SUNPRO_CC)
      #if   __SUNPRO_CC == 0x420
        #define HOTSPOT_BUILD_COMPILER "Workshop 4.2"
      #elif __SUNPRO_CC == 0x500
        #define HOTSPOT_BUILD_COMPILER "Workshop 5.0 compat=" XSTR(__SUNPRO_CC_COMPAT)
      #elif __SUNPRO_CC == 0x520
        #define HOTSPOT_BUILD_COMPILER "Workshop 5.2 compat=" XSTR(__SUNPRO_CC_COMPAT)
      #elif __SUNPRO_CC == 0x580
        #define HOTSPOT_BUILD_COMPILER "Workshop 5.8"
      #elif __SUNPRO_CC == 0x590
        #define HOTSPOT_BUILD_COMPILER "Workshop 5.9"
      #else
        #define HOTSPOT_BUILD_COMPILER "unknown Workshop:" XSTR(__SUNPRO_CC)
      #endif
    #elif defined(__GNUC__)
        #define HOTSPOT_BUILD_COMPILER "gcc " __VERSION__
    #else
      #define HOTSPOT_BUILD_COMPILER "unknown compiler"
    #endif
  #endif


  return VMNAME " (" VM_RELEASE ") for " OS "-" CPU
         " JRE (" JRE_RELEASE_VERSION "), built on " __DATE__ " " __TIME__ 
         " by " XSTR(HOTSPOT_BUILD_USER) " with " HOTSPOT_BUILD_COMPILER;
}

unsigned int Abstract_VM_Version::jvm_version() {
  return ((Abstract_VM_Version::vm_major_version() & 0xFF) << 24) |
         ((Abstract_VM_Version::vm_minor_version() & 0xFF) << 16) |
         (Abstract_VM_Version::vm_build_number() & 0xFF);
}


void VM_Version_init() {
  VM_Version::initialize();

#ifndef PRODUCT
  if (PrintMiscellaneous && Verbose) {
    os::print_cpu_info(tty);
  }
#endif
}
