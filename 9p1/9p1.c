/* ----------------------------------------------------------------------------
Copyright (c) 2019, Microsoft Corporation, Sean Hinchee
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/*
	This program is the equivalent of the plan9port project's 9p(1) tool

	usage: 9p1 [-Dn] [-a address] [-A aname] cmd args...
*/
#include <stdio.h>
#include <9p.h>

// Print our usage
void
usage(void)
{
	fprint(stderr, L"usage: 9p1 [-Dn] [-a address] [-A aname] cmd args...\n");
	exits(L"usage");
}

// 9p1 entry point
// TODO - Maybe use wmain() ?
int
wmain(int argc, Rune *argv[], Rune *envp[])
{
	SetConsoleOutputCP(CP_UTF8);

	Rune addr[BUFSIZE];
	runestrncpy(addr, BUFSIZE, L"tcp!localhost!1339");

	arginit(argc, argv, usage);
	Rune r;
	Rune *a;
	while((r = argopt()) != 0)
		switch(r) {
		case L'D':
			chatty++;
			break;
		case L'a':
			a = earg();
			runestrncpy(addr, BUFSIZE, a);
			break;
		// TODO -- more opts
		default:
			usage();
		}

	print(L"Dialing %ls...\n", addr);
	Conn *c = dial(addr);
	if(c == nil)
		sysfatal(L"main() network connection failed, socket was nil");
	print(L"Dial success.\n");

	/* TODO - DO THINGS HERE */
	Rune buf[BUFSIZE];
	ZeroMemory(buf, BUFSIZE);

	// Test reading from server
	creadstr(c, buf, BUFSIZE);
	print(L"From server: %ls\n", buf);

	// Input loop
	int n = 1;
	while(n > 0) {
		ZeroMemory(buf, BUFSIZE);
		print(L"\n> ");
		getrunes(buf, BUFSIZE);

		// Check for interpreted commands
		if(runestreq(buf, L"exit") || runestreq(buf, L"quit")) {
			print(L"Goodbye!\n");
			break;
		}

		// Test writing to server
		n = cwritestr(c, buf, BUFSIZE);
		if(n <= 0)
			break;

		print(L"To server: %ls\n", buf);

		ZeroMemory(buf, BUFSIZE);

		n = creadstr(c, buf, BUFSIZE);

		print(L"From server: %ls\n", buf);
	}

	// If you double close BAD things happen
	if(n > 0)
		closeconn(c);

	exits(nil);
}
