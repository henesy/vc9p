/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Corporation, Sean Hinchee
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/*
	This library serves as a provider of functions and data structures to enable the implementation of 9p clients and servers in Visual C natively on Windows
*/
#pragma once

// Link with winsock2
#pragma comment(lib, "Ws2_32.lib")

#include <wchar.h>
#include <errno.h>
#include <WinSock2.h>

// VS recommended this
#define WIN32_LEAN_AND_MEAN

/* Miscellaneous 9-isms */
#define nil		((void*)0)	// s/null/nil/g
#define BUFSIZE	256			// Buffer size for misc small text buffers

// 9-style utf8 strings
typedef wchar_t				Rune;

// Types in 9-style
typedef unsigned int		uint;
typedef unsigned long		ulong;	
typedef unsigned long long	uvlong;	// Use this like size_t 
typedef long long			vlong;
typedef uvlong				u64int;
typedef vlong				s64int;
typedef unsigned __int8		u8int;
typedef __int8				s8int;
typedef unsigned __int16	u16int;
typedef __int16				s16int;
typedef unsigned int*		uintptr;
typedef int*				intptr;
typedef unsigned int		u32int;
typedef int					s32int;

/* Network connectivity prototypes */

// Protocol selection
typedef int Proto;
enum Proto {
	tcp,
	unix,
	maxproto,
};

// Holds a 9p network connection 
typedef struct Conn Conn;
struct Conn {
	Proto	proto;			// Protocol to use
	Rune	addr[BUFSIZE];	// Hostname to connect to
	Rune	port[BUFSIZE];	// Port to connect to
	SOCKET	sock;
	//Fid		cwd;	// Current directory on a connection
};

Conn*	parsedialstr(Rune*);
Conn*	dial(Rune*);
Conn*	closeconn(Conn*);
int		creadstr(Conn*, Rune*, int);
int		cwritestr(Conn*, Rune*, int);

/* Arg prototypes */
void	arginit(int, Rune *argv[], void (*ousage)(void));
Rune	argopt(void);
Rune*	arg(void);
Rune*	earg(void);


/* Util prototypes */

// Flow control
void	exits(Rune*);
void	sysfatal(Rune*);
void	efatal(int, Rune*);
void*	emalloc(uvlong);
void*	ecalloc(uvlong, uvlong);

// Input
Rune*	getrunes(Rune*, int);
Rune	getrune(void);

// Output
void	print(Rune*, ...);
void	fprint(FILE*, Rune*, ...);
void	snprint(Rune*, uvlong, Rune*, ...);

/* Runestr prototypes */

// Utf-8 string (Rune*) operation wrappers
ulong	tokenize(Rune *str, Rune **args, int maxargs, Rune *delims);
uvlong	runestrlen(Rune*);
ulong	runestrcmp(Rune*, Rune*);
int		runestreq(Rune*, Rune*);
int		runestrncpy(Rune*, uvlong, Rune*);
int		runecontains(Rune*, Rune);
uvlong	runecount(Rune*, Rune);
int		rune2int(Rune*);
Rune*	runesliceright(Rune*, uvlong);
void	runes2bytes(Rune*, byte*, uvlong);
void	bytes2runes(byte*, Rune*);

/* Global variables */
extern int	chatty;		// Debug logging chattiness
