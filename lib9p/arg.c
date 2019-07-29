/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Corporation, Sean Hinchee
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/*
	arg.c provides functionality similar to arg.m does in Inferno
	http://man.postnix.pw/purgatorio/2/arg

	This requires you to have a function of the signature

		void usage();

	defined.

	NOTE: Maybe should make an Arg{}; struct which holds all this info so we don't pollute the ns
*/
#include "9p.h"
#include <stdio.h>
#include <stdlib.h>

// Variables stored by arg library
void (*ausage)(void);

int		argc;
Rune	**argv;
Rune	*curropt;	// Maybe we should allocate curropt every time?
int		argcn;		// Counter for up to argc

// Initialize arguments state -- must be called first
// usage() function should have the signature `void usage(void);`
// usage() is expected to end program operation
void
arginit(int oargc, Rune *oargv[], void (*ousage)(void))
{
	argc = oargc;
	// argv[][] should probably be a deep copy rather than shallow
	argv = oargv;
	ausage = ousage;
	argcn = 1; // skip argv[0]
}

// Get next option letter
// Ref: http://code.9front.org/hg/purgatorio/raw-file/tip/appl/lib/arg.b
Rune
argopt(void)
{
	if(curropt != nil) {
		Rune r = curropt[0];

		// Might not be correct
		// Want curropt = curropt[1:]
		Rune *co = runesliceright(curropt + 1, runestrlen(curropt));
		free(curropt);
		curropt = co;

		return r;
	}

	if(argv == nil || argcn >= argc)
		return 0;

	Rune *nextarg = argv[argcn];
	if(runestrlen(nextarg) < 2 || nextarg[0] != L'-')
		return 0;

	if(runestrcmp(nextarg, L"--") == 0) {
		Rune *co = runesliceright(curropt, 1);
		free(curropt);
		curropt = co;
		return 0;
	}

	Rune opt = nextarg[1];
	if(runestrlen(nextarg) > 2) {
		Rune *co = runesliceright(curropt, 2);
		free(curropt);
		curropt = co;
	}
	argcn++;

	return opt;
}

// Get the argument value after a flag
Rune*
arg(void)
{
	if(curropt != nil) {
		Rune *r = curropt;
		free(curropt);
		curropt = nil;
		return r;
	}

	if(argv == nil)
		return nil;

	Rune *r = argv[argcn];
	argcn++;
	return r;
}

// arg() with an error check for the value being nil
Rune*
earg(void)
{
	Rune *r = arg();
	if(r == nil)
		(*ausage)();

	return r;
}
