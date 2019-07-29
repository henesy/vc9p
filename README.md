# Visual C 9p

Visual C 9p, or vc9p, is a project proividing an accessible 9p2000.x library on Windows implemented natively in Visual C. 

Primarily, vc9p is being developed to facilitate the creation of the 9pf utility which is intended for scriptable fuzzing of 9p connections. 

## Depdendencies

- Visual C stdlib

## Build

### Visual Studio

This source and its build is tested under Visual Studio 2019. 

## Usage

Currently for testing purposes, if you want to test the 9p connection on Windows, I recommend running a local instance of Inferno.

Under Inferno, you can create a listener from the shell using a command similar to:

	; listen -A 'tcp!*!1337' { export /n/MYFS & }

For an example of a file system you can use for debugging purposes, try [Tic-Tac-Toe FS](https://github.com/henesy/tictactoefs-limbo). 

### 9p1

This program is the equivalent of the [plan9port project's 9p(1) 9p2000 client tool](https://raw.githubusercontent.com/9fans/plan9port/master/src/cmd/9p.c).

`9p1 [-Dn] [-a address] [-A aname] cmd args...`

`-D`			-- Turn on chatty debug tracing

`-n`			-- Connect without using auth

`-a address`	-- Dialstring to connect to

`-A aname`		-- Server aname to connect to

`cmd args...`	-- Command and arguments to pass for execution against 9p server

### 9pf

9pf is a program similar to 9p1 which attaches to a given connection (like a shell) and allows the textual crafting of 9p messages for arbitrary T/R messages.

9pf is intended to be useful for scripted fuzzing of 9p servers/clients

`9pf [-DnsSt] [-a address] [-A aname] [-p proto]`

`-D`			-- Show the traces for our messages

`-n`			-- Don't use auth

`-s`			-- Show message sizes (automatically calculated)

`-S`			-- Use server mode (Default: client mode)

`-t`			-- Automatically assign/handle tag field

`-a address`	-- Dialstring to connect/listen

`-A aname`		-- Server aname to connect/listen

`-p proto`		-- Select a protocol signature to use (if not entering manually) ;; this negotiates Tversion automatically (Default: 9p2000)

The input command format is:

	# size[4] Tversion tag[2] msize[4] version[s]

	; 9pf -D -n -a 'tcp!127.0.0.1!1337' 
	> Tversion -1 8216 9p2000
	→ Tversion 65535 8216 "9P2000"
	← Rversion 65535 8216 "9p2000"
	> exit
	; 

## Contribute

Please do.

Vague style guide: https://9p.io/magic/man2html/6/style

Please comment all functions, structs, and major - shared - variables. 

Please use `git config core.autocrlf false` for this repository. This is intended to only run natively under Windows. 

Send patches via e-mail or PR on GitHub :)

## References: 

- https://github.com/torvalds/linux/tree/master/fs/9p
- https://www.mjmwired.net/kernel/Documentation/filesystems/9p.txt 
- https://9p.io/magic/man2html/5/0intro
