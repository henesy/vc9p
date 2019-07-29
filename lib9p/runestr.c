/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Corporation, Sean Hinchee
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/*
	runestr.c contains Rune* string routines
*/
#include "9p.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Unicode string slicing */

// Equivalent of r := s[1:] ;; this makes a copy
// You need to free the return string
Rune*
runesliceright(Rune *s, uvlong n)
{
	if(s == nil)
		return nil;

	if(n == 0)
		return s;

	uvlong len = runestrlen(s);
	if(len == 0)
		return nil;		// String length of 0

	// (abc, 3) -> 3 - 3 = 0 -> nil string
	if(n >= len)
		return nil;

	uvlong diff = len - n;

	// (abcd, 3) -> [d] result -> (4 - 3) + 1 -> len [d, \0] = 2
	Rune *r = ecalloc(diff + 1, sizeof(Rune));
	runestrncpy(r, diff, s+n);
	r[diff] = L'\0';

	return r;
}

Rune*
runeslicel()
{
	sysfatal(L"runeslicel not impl");
	return nil;
}

// Convert a Rune[] into char[]
// This involves downcasts and is bad, please never do this
// TODO - Do better
// TODO - I hate this function name
void
runes2bytes(Rune *r, byte *c, uvlong clen)
{
	uvlong ret;

	wcstombs_s(&ret, c, clen, r, clen);
}

// Convert a char[] into a Rune[]
// Assume that r has enough space to hold c
// This is better than the other direction
// TODO - Do better
// TODO - I hate this function name, too
void
bytes2runes(byte *c, Rune *r)
{
	for(int i = 0; i < strlen(c); i++)
		r[i] = c[i];
}

/* Unicode string wrappers */

// Test if a string s contains r
int
runecontains(Rune *s, Rune r)
{
	uvlong len = runestrlen(s);

	for(int i = 0; i < len; i++)
		if(s[i] == r)
			return 1;

	return 0;
}

// Return the number of instances of Rune r in string s
uvlong
runecount(Rune *s, Rune r)
{
	uvlong n = 0;
	uvlong len = runestrlen(s);

	for(ulong i = 0; i < len; i++)
		if(s[i] == r)
			n++;

	return n;
}

// Rune strlen()
uvlong
runestrlen(Rune *s)
{
	return wcslen(s);
}

// Rune strequals
int
runestreq(Rune *a, Rune *b)
{
	return (runestrcmp(a, b) == 0);
}

// Rune strcmp
ulong
runestrcmp(Rune *a, Rune *b)
{
	return wcscmp(a, b);
}

// Rune strncpy
int
runestrncpy(Rune *dest, uvlong n, Rune *src)
{
	return wcsncpy_s(dest, n, src, runestrlen(src));
}

// Rune atoi
int
rune2int(Rune *s)
{
	return _wtoi(s);
}

// Get tokens from a string split by a set of delimiters
// Assume we have maxargs number of open pointer slots
// You need to free the fields
// Ex. tokenize("a!b!c", Rune buf[3][], 3, "!") -> []{"a", "b", "c"}
// http://man.postnix.pw/purgatorio/2/sys-tokenize
ulong
tokenize(Rune *str, Rune **toks, int maxargs, Rune *delims)
{
	if(str == nil || toks == nil || maxargs <= 0)
		return 0;

	ulong ntoks = 0;
	ulong nr = 0;		// Number of runes in current token

	Rune r;
	int intok = 0;	// Consider ourselves NOT in a token by default
	Rune *a = str;	// Track a pointer to drop into args[x] = a

	uvlong len = runestrlen(str);
	if(len == (uvlong)(-1))
		sysfatal(L"rstrlen failed!");

	// Full string is at least rstrlen()+1 to include the mandatory null-terminator rune
	for(ulong i = 0; i < len + 1; i++) {
		r = str[i];

		// \0 is a valid delimiter :)
		if(runecontains(delims, r) || r == L'\0') {
			if(intok) {
				// Write out if leaving a token
				toks[ntoks] = ecalloc(nr + 1, sizeof(Rune));
				ulong k = 0;
				for(ulong j = i - nr; j < runestrlen(str) && k < nr; j++) {
					toks[ntoks][k] = str[j];
					k++;
				}

				toks[ntoks][nr] = L'\0';

				ntoks++;
				intok = 0;
				nr = 0;
			}
		}
		else {
			// This is not a separator
			if(!intok) {
				// We are starting a token
				nr = 0;
			}

			intok = 1;

			nr++;
		}
	}

	return ntoks;
}
