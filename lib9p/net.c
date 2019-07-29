/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Corporation, Sean Hinchee
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/*
	This file holds the network connectivity utilities which are idiomatic to 9p/plan9
*/
#include "9p.h"
#include <stdlib.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

Proto str2proto(Rune *r);

// Connect to a server using a dialstring in the form proto!hostname!port
Conn*
dial(Rune *dialstr)
{
	Conn* c = parsedialstr(dialstr);

	WSADATA wsaData;
	int iResult;

	c->sock = INVALID_SOCKET;

	ADDRINFOW  *result = NULL,
		*ptr = NULL,
		hints;

	// Init winsock2
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(iResult != NO_ERROR) {
		fprint(stderr, L"err: dial WinSock2 startup failed with error # %d\n", iResult);
		return closeconn(c);
	}

	// Switch based on protocol we're using
	if(c->proto == tcp) {
		// Some serious dragons here
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = GetAddrInfoW(c->addr, c->port, &hints, &result);
		if(iResult != 0) {
			fprint(stderr, L"err: dial address resolution failed with error # %d\n", iResult);
			return closeconn(c);
		}

		// Attempt to connect to an address until one succeeds
		for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET
			c->sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
			if(c->sock == INVALID_SOCKET) {
				fprint(stderr, L"err: dial socket creation failed with error # %ld\n", WSAGetLastError());
				return closeconn(c);
			}

			// Connect to server - go to the next IP in the list if unsuccessful
			iResult = connect(c->sock, ptr->ai_addr, (int)ptr->ai_addrlen);
			if(iResult == SOCKET_ERROR) {
				closesocket(c->sock);
				c->sock = INVALID_SOCKET;
				continue;
			}
			break;
		}

		FreeAddrInfoW(result);

		if(c->sock == INVALID_SOCKET) {
			fprint(stderr, L"err: dial got INVALID_SOCKET for socket creation %ld\n", WSAGetLastError());
			return closeconn(c);
		}

	} else {
		sysfatal(L"no protocols implemented outside of tcp");
	}

	return c;
}

// Write to a network connection from byte[] buf
int
cwrite(Conn *c, byte *buf, int len)
{
	int n;
	n = send(c->sock, buf, len, 0);

	if(n == 0)
		closeconn(c);

	if(n < 0) {
		closeconn(c);
		int err = WSAGetLastError();
		if(err == 0)
			fprint(stderr, L"err: write failed - connection closed gracefully by the other party\n");
		else
			fprint(stderr, L"err: connection write failed with size %d error # %d\n", n, err);
	}

	return n;
}

// Read from a network connection into a byte[] buf
int
cread(Conn *c, byte *buf, int len)
{
	int n;
	n = recv(c->sock, buf, len, 0);

	if(n == 0)
		closeconn(c);

	if(n < 0) {
		closeconn(c);
		int err = WSAGetLastError();
		if(err == 0)
			fprint(stderr, L"err: read failed - connection closed gracefully by the other party\n");
		else
			fprint(stderr, L"err: connection read failed with size %d error # %d\n", n, err);
	}

	return n;
}

// Write to a network connection from Rune[] buf
// Len should be the desired total bytes written, in a char[]
int
cwritestr(Conn *c, Rune *buf, int len)
{
	int n;
	char *cbuf = ecalloc(len, sizeof(char));

	runes2bytes(buf, cbuf, len);

	n = cwrite(c, cbuf, len);
	free(cbuf);

	return n;
}

// Read from a network connection into Rune[] buf
// Len should be the desired total bytes read, in a char[]
int
creadstr(Conn *c, Rune *buf, int len)
{
	int n;
	char *cbuf = ecalloc(len, sizeof(char));

	n = cread(c, cbuf, len);

	bytes2runes(cbuf, buf);
	free(cbuf);

	return n;
}

// Close an open connection
Conn*
closeconn(Conn *c)
{
	if(c == nil)
		return nil;

	closesocket(c->sock);

	// Cleans up winsock stuff(?)
	// This might be too lethal if we have multiple connections?
	WSACleanup();

	free(c);
	return nil;
}

// Parse a dialstring in the form proto!hostname!port ;; See: http://man.postnix.pw/9front/6/ndb
Conn*
parsedialstr(Rune *dialstr)
{
	if(dialstr == nil)
		return nil;

	Conn* c = emalloc(sizeof(Conn));

	c->proto = tcp;
	runestrncpy(c->port, BUFSIZE, L"564");	// Default 9p fileserver port
	runestrncpy(c->addr, BUFSIZE, dialstr);

	// Max fields is 3
	Rune **fields = ecalloc(3, sizeof(Rune*));

	int nfields = tokenize(dialstr, fields, 3, L"!");

	Proto p;
	switch(nfields) {
	case 1:
		// The form does not include !, probably just the hostname and the default port
		break;
	case 2:
		// The form is either host!port or proto!host
		p = str2proto(fields[0]);
		if(p < maxproto) {
			// Valid protocol field; form is proto!host
			c->proto = p;
			runestrncpy(c->addr, BUFSIZE, fields[1]);
		} else {
			// Form is host!port (tcp is default)
			runestrncpy(c->addr, BUFSIZE, fields[0]);
			runestrncpy(c->port, BUFSIZE, fields[1]);
		}
		break;
	case 3:
		// The form is proto!host!port
		c->proto = str2proto(fields[0]);	// Error is maxproto
		runestrncpy(c->addr, BUFSIZE, fields[1]);
		//c->port = rune2int(fields[2]);			// Error is 0 (probably)
		runestrncpy(c->port, BUFSIZE, fields[2]);

		break;
	default:
		fprint(stderr, L"err: invalid dial string");
		runestrncpy(c->addr, BUFSIZE, L"err: invalid dial string");
	}

	// May be unsafe
	for(int i = 0; i < nfields; i++)
		free(fields[i]);

	free(fields);

	return c;
}

// Convert a string like "tcp" to its equivalent Proto value ;; maxproto is an error
Proto
str2proto(Rune *r)
{
	if(runestrcmp(r, L"unix") == 0)
		return unix;

	if(runestrcmp(r, L"tcp") == 0)
		return tcp;

	return maxproto;
}
