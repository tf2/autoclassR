/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* <fcntl.h> does not exist in SunOS 4.1.3 GNU installation */
/* <fcntl.h> & <sys/fcntlcom.h> do not exist in Solaris GNU installation */
/* hard code flags from fcntlcom.h & fcntl.h */
/* 27apr95 wmt */

#ifndef NO_FCNTL_H

#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>

#else

/*
 * Rewack the FXXXXX values as _FXXXX so that _POSIX_SOURCE works.
 */
	/* non blocking I/O (4.2 style) */
#define	_FNDELAY	0x0004
	/* append (writes guaranteed at the end) */
#define	_FAPPEND	0x0008
	/* open with file create */
#define	_FCREAT		0x0200
	/* open with truncation */
#define	_FTRUNC		0x0400

/* fcntl(2) requests - get & set file flags */
#define	F_GETFL		3
#define	F_SETFL		4

/* Non-blocking I/O (4.2 style) */
#define O_NDELAY        _FNDELAY

extern int fcntl();
extern int _filbuf ();

#endif	/* NO_FCNTL_H */
