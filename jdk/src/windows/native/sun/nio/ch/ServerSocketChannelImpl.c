/*
 * Copyright (c) 2000, 2002, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * This file has been modified by Azul Systems, Inc. in 2014. These
 * modifications are Copyright (c) 2014 Azul Systems, Inc., and are made
 * available on the same license terms set forth above. 
 */

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"

#include "nio.h"
#include "nio_util.h"
#include "net_util.h"

#include "sun_nio_ch_ServerSocketChannelImpl.h"


static jfieldID fd_fdID;        /* java.io.FileDescriptor.fd */
static jclass isa_class;        /* java.net.InetSocketAddress */
static jmethodID isa_ctorID;    /* InetSocketAddress(InetAddress, int) */
static jclass ia_class;         /* java.net.InetAddress */
static jmethodID ia_ctorID;     /* InetAddress() */


/**************************************************************
 * static method to store field IDs in initializers
 */

JNIEXPORT void JNICALL
Java_sun_nio_ch_ServerSocketChannelImpl_initIDs(JNIEnv *env, jclass cls)
{
    cls = (*env)->FindClass(env, "java/io/FileDescriptor");
    fd_fdID = (*env)->GetFieldID(env, cls, "fd", "I");

    cls = (*env)->FindClass(env, "java/net/InetSocketAddress");
    isa_class = (*env)->NewGlobalRef(env, cls);
    isa_ctorID = (*env)->GetMethodID(env, cls, "<init>",
                                     "(Ljava/net/InetAddress;I)V");

    cls = (*env)->FindClass(env, "java/net/Inet4Address");
    ia_class = (*env)->NewGlobalRef(env, cls);
    ia_ctorID = (*env)->GetMethodID(env, cls, "<init>","()V");
}

JNIEXPORT void JNICALL
Java_sun_nio_ch_ServerSocketChannelImpl_listen(JNIEnv *env, jclass cl,
                                               jobject fdo, jint backlog)
{
    if (listen(fdval(env,fdo), backlog) == SOCKET_ERROR) {
        NET_ThrowNew(env, WSAGetLastError(), "listen");
    }
}

JNIEXPORT jint JNICALL
Java_sun_nio_ch_ServerSocketChannelImpl_accept0(JNIEnv *env, jobject this,
                                                jobject ssfdo, jobject newfdo,
                                                jobjectArray isaa)
{
    jint ssfd = (*env)->GetIntField(env, ssfdo, fd_fdID);
    jint newfd;
    struct sockaddr_in sa;
    jobject remote_ia = 0;
    jobject isa;
    jobject ia;
    int addrlen = sizeof(sa);

    memset((char *)&sa, 0, sizeof(sa));
    newfd = (jint)accept(ssfd, (struct sockaddr *)&sa, &addrlen);
    if (newfd == INVALID_SOCKET) {
        int theErr = (jint)WSAGetLastError();
        if (theErr == WSAEWOULDBLOCK) {
            return IOS_UNAVAILABLE;
        }
        JNU_ThrowIOExceptionWithLastError(env, "Accept failed");
        return IOS_THROWN;
    }
    (*env)->SetIntField(env, newfdo, fd_fdID, newfd);

    ia = (*env)->NewObject(env, ia_class, ia_ctorID);
    setInetAddress_addr(env, ia, ntohl(sa.sin_addr.s_addr));
    setInetAddress_family(env, ia, sa.sin_family);

    isa = (*env)->NewObject(env, isa_class, isa_ctorID, ia,
                            ntohs(sa.sin_port));
    (*env)->SetObjectArrayElement(env, isaa, 0, isa);
    return 1;
}
