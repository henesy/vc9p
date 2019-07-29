/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Corporation, Sean Hinchee
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* 
	This file is an aggregate for miscellaneous functions and structures which do not fit into any other location intuitively 
*/
#include "9p.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Read a string from stdin -- returns nil on an error ;; strips trailing \n
Rune*
getrunes(Rune *buf, int n)
{
	Rune *err = fgetws(buf, n, stdin);

	// Strip trailing newline
	uvlong len = runestrlen(buf);
	if(len > 0)
		if(buf[len - 1] == L'\n')
			buf[len - 1] = L'\0';
		
	return err;
}

// Read 1 rune from stdin -- returns EOF on an error
Rune
getrune(void)
{
	return fgetwc(stdin);
}

// Wrapper for wprintf()
void
print(Rune* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vwprintf_s(fmt, args);

	va_end(args);
}

// Wrapper for fwprintf()
void
fprint(FILE* f, Rune* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfwprintf_s(f, fmt, args);

	va_end(args);
}

// Wrapper for snwprintf()
void
snprint(Rune *buf, uvlong count, Rune* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	// Might be unsafe, one of the counts should be a calculated max; use a wrapper fn?
	_vsnwprintf_s(buf, count, count, fmt, args);

	va_end(args);
}

/* Flow control */

// Exit normally with a status message -- no \n expected 
void
exits(Rune *s)
{
	fwprintf(stderr, L"%ls\n", s);
	exit(0);
}

// Exit fatally with a status message -- no \n expected ;; Exit code 1
void
sysfatal(Rune *s)
{
	fwprintf(stderr, L"fatal -- %ls\n", s);
	exit(1);
}

// Exit fatally if err > 0 along with a message
void
efatal(int err, Rune *s)
{
	if(err) {
		Rune buf[BUFSIZE];
		snprint(buf, BUFSIZE, L"err: %ls", s);
		sysfatal(buf);
	}
}

// If allocation fails, bring us down
void*
emalloc(uvlong size)
{
	void* mem = malloc(size);
	if(mem == nil)
		sysfatal(L"err: malloc failed");

	return mem;
}

// If allocation fails, bring us down
void*
ecalloc(uvlong n, uvlong size)
{
	void* mem = calloc(n, size);
	if (mem == nil)
		sysfatal(L"err: calloc failed");

	return mem;
}
