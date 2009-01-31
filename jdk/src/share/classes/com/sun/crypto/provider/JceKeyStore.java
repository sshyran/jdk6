/*
 * Copyright 1998-2007 Sun Microsystems, Inc.  All Rights Reserved.
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

package com.sun.crypto.provider;

import java.io.*;
import java.util.*;
import java.security.DigestInputStream;
import java.security.DigestOutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.Key;
import java.security.PrivateKey;
import java.security.KeyStoreSpi;
import java.security.KeyStoreException;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.spec.InvalidKeySpecException;
import javax.crypto.SealedObject;

/**
 * This class provides the keystore implementation referred to as "jceks".
 * This implementation strongly protects the keystore private keys using
 * triple-DES, where the triple-DES encryption/decryption key is derived from
 * the user's password.
 * The encrypted private keys are stored in the keystore in a standard format,
 * namely the <code>EncryptedPrivateKeyInfo</code> format defined in PKCS #8.
 *
 * @author Jan Luehe
 *
 *
 * @see java.security.KeyStoreSpi
 */

public final class JceKeyStore extends KeyStoreSpi {

    private static final int JCEKS_MAGIC = 0xcececece;
    private static final int JKS_MAGIC = 0xfeedfeed;
    private static final int VERSION_1 = 0x01;
    private static final int VERSION_2 = 0x02;

    // Private key and supporting certificate chain
    private static final class PrivateKeyEntry {
	Date date; // the creation date of this entry
	byte[] protectedKey;
	Certificate chain[];
    };

    // Secret key
    private static final class SecretKeyEntry {
	Date date; // the creation date of this entry
	SealedObject sealedKey;
    }

    // Trusted certificate
    private static final class TrustedCertEntry {
	Date date; // the creation date of this entry
	Certificate cert;
    };

    /**
     * Private keys and certificates are stored in a hashtable.
     * Hash entries are keyed by alias names.
     */
    private Hashtable entries = new Hashtable();

    /**
     * Returns the key associated with the given alias, using the given
     * password to recover it.
     *
     * @param alias the alias name
     * @param password the password for recovering the key
     *
     * @return the requested key, or null if the given alias does not exist
     * or does not identify a <i>key entry</i>.
     *
     * @exception NoSuchAlgorithmException if the algorithm for recovering the
     * key cannot be found
     * @exception UnrecoverableKeyException if the key cannot be recovered
     * (e.g., the given password is wrong).
     */
    public Key engineGetKey(String alias, char[] password)
	throws NoSuchAlgorithmException, UnrecoverableKeyException
    {
	Key key = null;

	Object entry = entries.get(alias.toLowerCase());

	if (!((entry instanceof PrivateKeyEntry) || 
	      (entry instanceof SecretKeyEntry))) {
	    return null;
	}

	KeyProtector keyProtector = new KeyProtector(password);

	if (entry instanceof PrivateKeyEntry) {
	    byte[] encrBytes = ((PrivateKeyEntry)entry).protectedKey;
	    EncryptedPrivateKeyInfo encrInfo;
	    try {
		encrInfo = new EncryptedPrivateKeyInfo(encrBytes);
	    } catch (IOException ioe) {
		throw new UnrecoverableKeyException("Private key not stored "
						    + "as PKCS #8 " +
						    "EncryptedPrivateKeyInfo");
	    }
	    key = keyProtector.recover(encrInfo);
	} else {
	    key =
		keyProtector.unseal(((SecretKeyEntry)entry).sealedKey);
	}

	return key;
    }

    /**
     * Returns the certificate chain associated with the given alias.
     *
     * @param alias the alias name
     *
     * @return the certificate chain (ordered with the user's certificate first
     * and the root certificate authority last), or null if the given alias
     * does not exist or does not contain a certificate chain (i.e., the given 
     * alias identifies either a <i>trusted certificate entry</i> or a
     * <i>key entry</i> without a certificate chain).
     */
    public Certificate[] engineGetCertificateChain(String alias)
    {
	Certificate[] chain = null;

	Object entry = entries.get(alias.toLowerCase());

	if ((entry instanceof PrivateKeyEntry) 
	    && (((PrivateKeyEntry)entry).chain != null)) {
	    chain = (Certificate[])((PrivateKeyEntry)entry).chain.clone();
	}

	return chain;
    }

    /**
     * Returns the certificate associated with the given alias.
     *
     * <p>If the given alias name identifies a
     * <i>trusted certificate entry</i>, the certificate associated with that
     * entry is returned. If the given alias name identifies a
     * <i>key entry</i>, the first element of the certificate chain of that
     * entry is returned, or null if that entry does not have a certificate
     * chain.
     *
     * @param alias the alias name
     *
     * @return the certificate, or null if the given alias does not exist or
     * does not contain a certificate.
     */
    public Certificate engineGetCertificate(String alias) {
	Certificate cert = null;

	Object entry = entries.get(alias.toLowerCase());

	if (entry != null) {
	    if (entry instanceof TrustedCertEntry) {
		cert = ((TrustedCertEntry)entry).cert;
	    } else if ((entry instanceof PrivateKeyEntry) &&
		       (((PrivateKeyEntry)entry).chain != null)) {
		cert = ((PrivateKeyEntry)entry).chain[0];
	    }
	}

	return cert;
    }	

    /**
     * Returns the creation date of the entry identified by the given alias.
     *
     * @param alias the alias name
     *
     * @return the creation date of this entry, or null if the given alias does
     * not exist
     */
    public Date engineGetCreationDate(String alias) {
	Date date = null;

	Object entry = entries.get(alias.toLowerCase());

	if (entry != null) {
	    // We have to create a new instance of java.util.Date because
	    // dates are not immutable
	    if (entry instanceof TrustedCertEntry) {
		date = new Date(((TrustedCertEntry)entry).date.getTime());
	    } else if (entry instanceof PrivateKeyEntry) {
		date = new Date(((PrivateKeyEntry)entry).date.getTime());
	    } else {
		date = new Date(((SecretKeyEntry)entry).date.getTime());
	    }
	}

	return date;
    }

    /**
     * Assigns the given key to the given alias, protecting it with the given
     * password.
     *
     * <p>If the given key is of type <code>java.security.PrivateKey</code>,
     * it must be accompanied by a certificate chain certifying the
     * corresponding public key.
     *
     * <p>If the given alias already exists, the keystore information
     * associated with it is overridden by the given key (and possibly
     * certificate chain).
     *
     * @param alias the alias name
     * @param key the key to be associated with the alias
     * @param password the password to protect the key
     * @param chain the certificate chain for the corresponding public
     * key (only required if the given key is of type
     * <code>java.security.PrivateKey</code>).
     *
     * @exception KeyStoreException if the given key cannot be protected, or
     * this operation fails for some other reason
     */
    public void engineSetKeyEntry(String alias, Key key, char[] password,
				  Certificate[] chain)
	throws KeyStoreException
    {
	synchronized(entries) {
	    try {
		KeyProtector keyProtector = new KeyProtector(password);

		if (key instanceof PrivateKey) {
		    PrivateKeyEntry entry = new PrivateKeyEntry();
		    entry.date = new Date();

		    // protect the private key
		    entry.protectedKey = keyProtector.protect((PrivateKey)key);

		    // clone the chain
		    if ((chain != null) &&
			(chain.length !=0)) {
			entry.chain = (Certificate[])chain.clone();
		    } else {
			entry.chain = null;
		    }

		    // store the entry
		    entries.put(alias.toLowerCase(), entry);
		
		} else {
		    SecretKeyEntry entry = new SecretKeyEntry();
		    entry.date = new Date();
		    
		    // seal and store the key
		    entry.sealedKey = keyProtector.seal(key);
		    entries.put(alias.toLowerCase(), entry);
		}

	    } catch (Exception e) {
		throw new KeyStoreException(e.getMessage());
	    }
	}
    }

    /**
     * Assigns the given key (that has already been protected) to the given
     * alias.
     * 
     * <p>If the protected key is of type
     * <code>java.security.PrivateKey</code>,
     * it must be accompanied by a certificate chain certifying the
     * corresponding public key.
     *
     * <p>If the given alias already exists, the keystore information
     * associated with it is overridden by the given key (and possibly
     * certificate chain).
     *
     * @param alias the alias name
     * @param key the key (in protected format) to be associated with the alias
     * @param chain the certificate chain for the corresponding public
     * key (only useful if the protected key is of type
     * <code>java.security.PrivateKey</code>).
     *
     * @exception KeyStoreException if this operation fails.
     */
    public void engineSetKeyEntry(String alias, byte[] key,
				  Certificate[] chain)
	throws KeyStoreException
    {
	synchronized(entries) {
	    // We assume it's a private key, because there is no standard
	    // (ASN.1) encoding format for wrapped secret keys
	    PrivateKeyEntry entry = new PrivateKeyEntry();
	    entry.date = new Date();

	    entry.protectedKey = (byte[])key.clone();
	    if ((chain != null) &&
		(chain.length != 0)) {
		entry.chain = (Certificate[])chain.clone();
	    } else {
		entry.chain = null;
	    }

	    entries.put(alias.toLowerCase(), entry);
	}
    }

    /**
     * Assigns the given certificate to the given alias.
     *
     * <p>If the given alias already exists in this keystore and identifies a
     * <i>trusted certificate entry</i>, the certificate associated with it is
     * overridden by the given certificate.
     *
     * @param alias the alias name
     * @param cert the certificate
     *
     * @exception KeyStoreException if the given alias already exists and does
     * not identify a <i>trusted certificate entry</i>, or this operation
     * fails for some other reason.
     */
    public void engineSetCertificateEntry(String alias, Certificate cert)
	throws KeyStoreException
    {
	synchronized(entries) {

	    Object entry = entries.get(alias.toLowerCase());
	    if (entry != null) {
		if (entry instanceof PrivateKeyEntry) {
		    throw new KeyStoreException("Cannot overwrite own "
						+ "certificate");
		} else if (entry instanceof SecretKeyEntry) {
		    throw new KeyStoreException("Cannot overwrite secret key");
		}
	    }

	    TrustedCertEntry trustedCertEntry = new TrustedCertEntry();
	    trustedCertEntry.cert = cert;
	    trustedCertEntry.date = new Date();
	    entries.put(alias.toLowerCase(), trustedCertEntry);
	}
    }

    /**
     * Deletes the entry identified by the given alias from this keystore.
     *
     * @param alias the alias name
     *
     * @exception KeyStoreException if the entry cannot be removed.
     */
    public void engineDeleteEntry(String alias)
	throws KeyStoreException
    {
	synchronized(entries) {
	    entries.remove(alias.toLowerCase());
	}
    }

    /**
     * Lists all the alias names of this keystore.
     *
     * @return enumeration of the alias names
     */
    public Enumeration engineAliases() {
	return entries.keys();
    }

    /**
     * Checks if the given alias exists in this keystore.
     *
     * @param alias the alias name
     *
     * @return true if the alias exists, false otherwise
     */
    public boolean engineContainsAlias(String alias) {
	return entries.containsKey(alias.toLowerCase());
    }

    /**
     * Retrieves the number of entries in this keystore.
     *
     * @return the number of entries in this keystore
     */
    public int engineSize() {
	return entries.size();
    }

    /**
     * Returns true if the entry identified by the given alias is a
     * <i>key entry</i>, and false otherwise.
     *
     * @return true if the entry identified by the given alias is a
     * <i>key entry</i>, false otherwise.
     */
    public boolean engineIsKeyEntry(String alias) {
	boolean isKey = false;

	Object entry = entries.get(alias.toLowerCase());
	if ((entry instanceof PrivateKeyEntry)
	    || (entry instanceof SecretKeyEntry)) {
	    isKey = true;
	}

	return isKey;
    }

    /**
     * Returns true if the entry identified by the given alias is a
     * <i>trusted certificate entry</i>, and false otherwise.
     *
     * @return true if the entry identified by the given alias is a
     * <i>trusted certificate entry</i>, false otherwise.
     */
    public boolean engineIsCertificateEntry(String alias) {
	boolean isCert = false;
	Object entry = entries.get(alias.toLowerCase());
	if (entry instanceof TrustedCertEntry) {
	    isCert = true;
	}
	return isCert;
    }

    /**
     * Returns the (alias) name of the first keystore entry whose certificate
     * matches the given certificate.
     *
     * <p>This method attempts to match the given certificate with each
     * keystore entry. If the entry being considered
     * is a <i>trusted certificate entry</i>, the given certificate is
     * compared to that entry's certificate. If the entry being considered is
     * a <i>key entry</i>, the given certificate is compared to the first
     * element of that entry's certificate chain (if a chain exists).
     *
     * @param cert the certificate to match with.
     *
     * @return the (alias) name of the first entry with matching certificate,
     * or null if no such entry exists in this keystore.
     */
    public String engineGetCertificateAlias(Certificate cert) {
	Certificate certElem;

	Enumeration e = entries.keys();
	while (e.hasMoreElements()) {
	    String alias = (String)e.nextElement();
	    Object entry = entries.get(alias);
	    if (entry instanceof TrustedCertEntry) {
		certElem = ((TrustedCertEntry)entry).cert;
	    } else if ((entry instanceof PrivateKeyEntry) &&
		       (((PrivateKeyEntry)entry).chain != null)) {
		certElem = ((PrivateKeyEntry)entry).chain[0];
	    } else {
		continue;
	    }
	    if (certElem.equals(cert)) {
		return alias;
	    }
	}
	return null;
    }

    /**
     * Stores this keystore to the given output stream, and protects its
     * integrity with the given password.
     *
     * @param stream the output stream to which this keystore is written.
     * @param password the password to generate the keystore integrity check
     *
     * @exception IOException if there was an I/O problem with data
     * @exception NoSuchAlgorithmException if the appropriate data integrity
     * algorithm could not be found
     * @exception CertificateException if any of the certificates included in
     * the keystore data could not be stored
     */
    public void engineStore(OutputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	synchronized(entries) {
	    /*
	     * KEYSTORE FORMAT:
	     *
	     * Magic number (big-endian integer),
	     * Version of this file format (big-endian integer),
	     *
	     * Count (big-endian integer),
	     * followed by "count" instances of either:
	     *
	     *     {
	     *      tag=1 (big-endian integer)
	     *      alias (UTF string)
	     *      timestamp
	     *	    encrypted private-key info according to PKCS #8
	     *          (integer length followed by encoding)
	     *	    cert chain (integer count followed by certs;
	     *          for each cert: type UTF string, followed by integer
	     *              length, followed by encoding)
	     *     }
	     *
	     * or:
	     *
	     *     {
	     *      tag=2 (big-endian integer)
	     *      alias (UTF string)
	     *      timestamp
	     *      cert (type UTF string, followed by integer length,
	     *          followed by encoding)
	     *     }
	     *
	     * or:
	     *
	     *     {
	     *      tag=3 (big-endian integer)
	     *      alias (UTF string)
	     *      timestamp
	     *      sealed secret key (in serialized form)
	     *     }
	     *
	     * ended by a keyed SHA1 hash (bytes only) of
	     *     { password + whitener + preceding body }
	     */
	    
	    // password is mandatory when storing
	    if (password == null) {
		throw new IllegalArgumentException("password can't be null");
	    }

	    byte[] encoded; // the certificate encoding

	    MessageDigest md = getPreKeyedHash(password);
	    DataOutputStream dos
		= new DataOutputStream(new DigestOutputStream(stream, md));
	    // NOTE: don't pass dos to oos at this point or it'll corrupt
	    // the keystore!!!
	    ObjectOutputStream oos = null; 
	    try {
		dos.writeInt(JCEKS_MAGIC);
		dos.writeInt(VERSION_2); // always write the latest version
		
		dos.writeInt(entries.size());
		
		Enumeration e = entries.keys();
		while (e.hasMoreElements()) {
		    
		    String alias = (String)e.nextElement();
		    Object entry = entries.get(alias);
		    
		    if (entry instanceof PrivateKeyEntry) {
			
			PrivateKeyEntry pentry = (PrivateKeyEntry)entry;
			
			// write PrivateKeyEntry tag
			dos.writeInt(1);
			
			// write the alias
			dos.writeUTF(alias);

			// write the (entry creation) date
			dos.writeLong(pentry.date.getTime());
			
			// write the protected private key
			dos.writeInt(pentry.protectedKey.length);
			dos.write(pentry.protectedKey);
			
			// write the certificate chain
			int chainLen;
			if (pentry.chain == null) {
			    chainLen = 0;
			} else {
			    chainLen = pentry.chain.length;
			}
			dos.writeInt(chainLen);
			for (int i = 0; i < chainLen; i++) {
			    encoded = pentry.chain[i].getEncoded();
			    dos.writeUTF(pentry.chain[i].getType());
			    dos.writeInt(encoded.length);
			    dos.write(encoded);
			}
			
		    } else if (entry instanceof TrustedCertEntry) {
			
			// write TrustedCertEntry tag
			dos.writeInt(2);
			
			// write the alias
			dos.writeUTF(alias);
			
			// write the (entry creation) date
			dos.writeLong(((TrustedCertEntry)entry).date.getTime());
			
			// write the trusted certificate
			encoded = ((TrustedCertEntry)entry).cert.getEncoded();
			dos.writeUTF(((TrustedCertEntry)entry).cert.getType());
			dos.writeInt(encoded.length);    
			dos.write(encoded);
			
		    } else {
			
			// write SecretKeyEntry tag
			dos.writeInt(3);
			
			// write the alias
			dos.writeUTF(alias);
			
			// write the (entry creation) date
			dos.writeLong(((SecretKeyEntry)entry).date.getTime());
			
			// write the sealed key
			oos = new ObjectOutputStream(dos);
			oos.writeObject(((SecretKeyEntry)entry).sealedKey);
			// NOTE: don't close oos here since we are still
			// using dos!!!
		    }
		}
		
		/*
		 * Write the keyed hash which is used to detect tampering with
		 * the keystore (such as deleting or modifying key or
		 * certificate entries).
		 */
		byte digest[] = md.digest();
		
		dos.write(digest);
		dos.flush();
	    } finally {
		if (oos != null) {
		    oos.close();
		} else {
		    dos.close();
		}
	    }
	}
    }

    /**
     * Loads the keystore from the given input stream.
     *
     * <p>If a password is given, it is used to check the integrity of the
     * keystore data. Otherwise, the integrity of the keystore is not checked.
     *
     * @param stream the input stream from which the keystore is loaded
     * @param password the (optional) password used to check the integrity of
     * the keystore.
     *
     * @exception IOException if there is an I/O or format problem with the
     * keystore data
     * @exception NoSuchAlgorithmException if the algorithm used to check
     * the integrity of the keystore cannot be found
     * @exception CertificateException if any of the certificates in the
     * keystore could not be loaded
     */
    public void engineLoad(InputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	synchronized(entries) {
	    DataInputStream dis;
	    MessageDigest md = null;
	    CertificateFactory cf = null;
	    Hashtable cfs = null;
	    ByteArrayInputStream bais = null;
	    byte[] encoded = null;

	    if (stream == null)
		return;

	    if (password != null) {
		md = getPreKeyedHash(password);
		dis = new DataInputStream(new DigestInputStream(stream, md));
	    } else {
		dis = new DataInputStream(stream);
	    }
	    // NOTE: don't pass dis to ois at this point or it'll fail to load
	    // the keystore!!!
	    ObjectInputStream ois = null;

	    try { 
		// Body format: see store method
	     
		int xMagic = dis.readInt();
		int xVersion = dis.readInt();
		
		// Accept the following keystore implementations:
		// - JCEKS (this implementation), versions 1 and 2
		// - JKS (Sun's keystore implementation in JDK 1.2),
		//   versions 1 and 2
		if (((xMagic != JCEKS_MAGIC) && (xMagic != JKS_MAGIC)) ||
		    ((xVersion != VERSION_1) && (xVersion != VERSION_2))) {
		    throw new IOException("Invalid keystore format");
		}

		if (xVersion == VERSION_1) {
		    cf = CertificateFactory.getInstance("X509");
		} else {
		    // version 2
		    cfs = new Hashtable(3);
		}
		
		entries.clear();
		int count = dis.readInt();
		
		for (int i = 0; i < count; i++) {
		    int tag;
		    String alias;
		    
		    tag = dis.readInt();
		    
		    if (tag == 1) { // private-key entry
			
			PrivateKeyEntry entry = new PrivateKeyEntry();
			
			// read the alias
			alias = dis.readUTF();
			
			// read the (entry creation) date
			entry.date = new Date(dis.readLong());
			
			// read the private key
			try {
			    entry.protectedKey = new byte[dis.readInt()];
			} catch (OutOfMemoryError e) {
			    throw new IOException("Keysize too big");
			}
			dis.readFully(entry.protectedKey);
			
			// read the certificate chain
			int numOfCerts = dis.readInt();
			try {
			    if (numOfCerts > 0) {
				entry.chain = new Certificate[numOfCerts];
			    }
			} catch (OutOfMemoryError e) {
			    throw new IOException("Too many certificates in "
						  + "chain");
			}
			for (int j = 0; j < numOfCerts; j++) {
			    if (xVersion == 2) {
				// read the certificate type, and instantiate a
				// certificate factory of that type (reuse
				// existing factory if possible)
				String certType = dis.readUTF();
				if (cfs.containsKey(certType)) {
				// reuse certificate factory
				    cf = (CertificateFactory)cfs.get(certType);
				} else {
				// create new certificate factory
				    cf = CertificateFactory.getInstance(certType);
				// store the certificate factory so we can
				// reuse it later
				    cfs.put(certType, cf);
				}
			    }
			    // instantiate the certificate
			    try {
				encoded = new byte[dis.readInt()];
			    } catch (OutOfMemoryError e) {
				throw new IOException("Certificate too big");
			    }
			    dis.readFully(encoded);
			    bais = new ByteArrayInputStream(encoded);
			    entry.chain[j] = cf.generateCertificate(bais);
			}
			
			// Add the entry to the list
			entries.put(alias, entry);
			
		    } else if (tag == 2) { // trusted certificate entry
			
			TrustedCertEntry entry = new TrustedCertEntry();
			
			// read the alias
			alias = dis.readUTF();
			
			// read the (entry creation) date
			entry.date = new Date(dis.readLong());
			
			// read the trusted certificate
			if (xVersion == 2) {
			    // read the certificate type, and instantiate a
			    // certificate factory of that type (reuse
			    // existing factory if possible)
			    String certType = dis.readUTF();
			    if (cfs.containsKey(certType)) {
				// reuse certificate factory
				cf = (CertificateFactory)cfs.get(certType);
			    } else {
				// create new certificate factory
				cf = CertificateFactory.getInstance(certType);
				// store the certificate factory so we can
				// reuse it later
				cfs.put(certType, cf);
			    }
			}
			try {
			    encoded = new byte[dis.readInt()];
			} catch (OutOfMemoryError e) {
			    throw new IOException("Certificate too big");
			}
			dis.readFully(encoded);
			bais = new ByteArrayInputStream(encoded);
			entry.cert = cf.generateCertificate(bais);
			
			// Add the entry to the list
			entries.put(alias, entry);
			
		    } else if (tag == 3) { // secret-key entry
			
			SecretKeyEntry entry = new SecretKeyEntry();
			
			// read the alias
			alias = dis.readUTF();
			
			// read the (entry creation) date
			entry.date = new Date(dis.readLong());
			
			// read the sealed key
			try {
			    ois = new ObjectInputStream(dis);
			    entry.sealedKey = (SealedObject)ois.readObject();
			    // NOTE: don't close ois here since we are still
			    // using dis!!!
			} catch (ClassNotFoundException cnfe) {
			    throw new IOException(cnfe.getMessage());
			}
			
			// Add the entry to the list
			entries.put(alias, entry);
			
		    } else {
			throw new IOException("Unrecognized keystore entry");
		    }
		}
		
		/*
		 * If a password has been provided, we check the keyed digest
		 * at the end. If this check fails, the store has been tampered
		 * with
		 */
		if (password != null) {
		    byte computed[], actual[];
		    computed = md.digest();
		    actual = new byte[computed.length];
		    dis.readFully(actual);
		    for (int i = 0; i < computed.length; i++) {
			if (computed[i] != actual[i]) {
			    throw new IOException("Keystore was tampered with, or "
						  + "password was incorrect");
			}
		    }
		}
	    }  finally {
		if (ois != null) {
		    ois.close();
		} else {
		    dis.close();
		}
	    }
	}
    }

    /**
     * To guard against tampering with the keystore, we append a keyed
     * hash with a bit of whitener.
     */
    private MessageDigest getPreKeyedHash(char[] password)
    throws NoSuchAlgorithmException, UnsupportedEncodingException {
	int i, j;

	MessageDigest md = MessageDigest.getInstance("SHA");
	byte[] passwdBytes = new byte[password.length * 2];
	for (i=0, j=0; i<password.length; i++) {
	    passwdBytes[j++] = (byte)(password[i] >> 8);
	    passwdBytes[j++] = (byte)password[i];
	}
	md.update(passwdBytes);
	for (i=0; i<passwdBytes.length; i++)
	    passwdBytes[i] = 0;
	md.update("Mighty Aphrodite".getBytes("UTF8"));
	return md;
    }
}
