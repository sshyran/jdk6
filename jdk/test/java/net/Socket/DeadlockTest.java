/*
 * Copyright 1999 Sun Microsystems, Inc.  All Rights Reserved.
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
 */

/*
 * @test
 * @bug 4176738
 * @summary Make sure a deadlock situation
 *     would not occur 
 */

import java.net.*;
import java.io.*;

public class DeadlockTest {
    public static void main(String [] argv) throws Exception {

	// Start the server thread
        Thread s1 = new Thread(new ServerThread());
	s1.start(); 
		
	// Sleep to make sure s1 has created a server socket
	Thread.sleep(1000);

	// Start the client thread
	ClientThread ct = new ClientThread();
	Thread c1 = new Thread(ct);
	c1.start();
	
	// Wait for the client thread to finish
	c1.join(40000);

	// If timeout, we assume there is a deadlock
	if (c1.isAlive() == true) {
	    // Close the socket to force the server thread 
	    // terminate too
	    s1.stop();
	    ct.getSock().close();
	    throw new Exception("Takes too long. Dead lock");
	}
    }
}

class ServerThread implements Runnable {

    private static boolean dbg = false;

    ObjectInputStream  in;
    ObjectOutputStream out;

    ServerSocket server;

    Socket sock;

    public ServerThread() throws Exception {
       
    }

    public void ping(int cnt) {
       Message.write(out, new PingMessage(cnt));
    }

    private int cnt = 1;

    public void run() {

	try {
	    if (Thread.currentThread().getName().startsWith("child") == false) {
		server = new ServerSocket(4711);
		sock  = server.accept();

		new Thread(this, "child").start();

		out = new ObjectOutputStream(sock.getOutputStream());
		out.flush();

		if (dbg) System.out.println("*** ping0 ***");
		ping(0);
		if (dbg) System.out.println("*** ping1 ***");
		ping(1);
		if (dbg) System.out.println("*** ping2 ***");
		ping(2);
		if (dbg) System.out.println("*** ping3 ***");
		ping(3);
		if (dbg) System.out.println("*** ping4 ***");
		ping(4);
		if (dbg) System.out.println("*** end ***");
	    }

        } catch (Throwable e) {
	    // If anything goes wrong, just quit.
	} 

	if (Thread.currentThread().getName().startsWith("child")) {
	    try {
          
		in  = new ObjectInputStream(sock.getInputStream());

		while (true) {
		    if (dbg) System.out.println("read " + cnt);
		    Message msg = (Message) in.readObject();
		    if (dbg) System.out.println("read done " + cnt++);
		    switch (msg.code) {
		    case Message.PING: {
			if (true) System.out.println("ping recv'ed");
		    } break;
		    }
		    
		}        

	    } catch (Throwable e) {
		// If anything goes wrong, just quit.	    }
	    }
	}
    }
}

class ClientThread implements Runnable {

    ObjectInputStream  in;
    ObjectOutputStream out;

    Socket sock;

    public ClientThread() throws Exception {
        try {
	    System.out.println("About to create a socket");
	    sock = new Socket(InetAddress.getLocalHost().getHostName(), 4711);
	    System.out.println("connected");
 
	    out = new ObjectOutputStream(sock.getOutputStream());
	    out.flush();
        } catch (Throwable e) {
          System.out.println("client failed with: " + e);
          e.printStackTrace();
          throw new Exception("Unexpected exception");
        }
    }

    public Socket getSock() {
	return sock;
    }

    private int cnt = 1;

    public void run() {
        try {
          in  = new ObjectInputStream(sock.getInputStream());

	  int count = 0;

          while (true) {
	      System.out.println("read " + cnt);
	      Message msg = (Message) in.readObject();
	      System.out.println("read done " + cnt++);
	      switch (msg.code) {
	      case Message.PING: {
		  System.out.println("ping recv'ed");
		  count++;
	      } break;
	      }
	      if (count == 5) {
		  sock.close();
		  break;
	      }
          }
        }  catch (IOException ioe) {
	} catch (Throwable e) {
	    // If anything went wrong, just quit
        }
    }

}

class Message implements java.io.Serializable {

    static final int UNKNOWN = 0;
    static final int PING = 1;

    protected int code;

    public Message() { this.code = UNKNOWN; }

    public Message(int code) { this.code = code; }

    private static int cnt = 1;

    public static void write(ObjectOutput out, Message msg) {
        try {
	    System.out.println("write message " + cnt);
	    out.writeObject(msg);
	    System.out.println("flush message");
	    out.flush();
	    System.out.println("write message done " + cnt++);
        } catch (IOException ioe) {
	    // Ignore the exception
        }
     }
}

class PingMessage extends Message implements java.io.Serializable {

      public PingMessage() {
          code = Message.PING;
      }

      public PingMessage(int cnt)
      {
          code = Message.PING;
          this.cnt = cnt;

          data = new int[50000];
      }

      int cnt;
      int[] data;
}
