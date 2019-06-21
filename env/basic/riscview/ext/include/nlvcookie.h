/*  nlvcookie.h 1.27 2017/11/22
 *  Copyright 1998-2017 by Concept Engineering GmbH.
 *  All rights reserved.
 *  ===========================================================================
 *  This source code belongs to Concept Engineering.  It is considered 
 *  trade secret, and is not to be divulged or used or copied by parties
 *  who have not received written authorization from the owner, Concept
 *  Engineering.
 *  ===========================================================================
 *  Title:
 *	Nlview cookie - Interface to get a cookie to license Nlview widget.
 *
 *  Description:
 *	This header file is an interface for Concept Engineering's
 *	FLEXnet client support library used by customers who want to
 *	check-out FLEXnet licenses (e.g. during evaluation).
 *  ===========================================================================
*/

#ifndef nlvcookie_h
#define nlvcookie_h

#ifdef __cplusplus
extern "C" {
#endif

/* Flags to use for NlviewCookie(). Use OR '|' to combine them.
 */
#define NlviewCookieFVerbose 0x01	/* verbose messages */
#define NlviewCookieFHasp    0x02	/* check for HASP dongle */
#define NlviewCookieFFlex    0x04	/* check for FLEXlm license */
#define NlviewCookieFNoCache 0x08	/* disable FLEXlm cache */
#define NlviewCookieFDbg     0x10	/* enable debug prints */
#define NlviewCookieFOnlyJS  0x40	/* only for GUI=JS */

/* ----------------------------------------------------------------------------
 * NlviewCookieArgv: call NlviewCookie with options from argc/argv
 * NlviewCookie:     checkout FLEXnet or HASP license and return cookie
 *
 * NlviewGetNotice: get FLEXnet NOTICE field
 * NlviewUncheck:   checkin (return) FLEXnet license
 *
 *	all functions return 1 if ok, or 0 for error; in case of
 *	an error, the "result" stores the error message.
 */
extern int NlviewCookieArgv(char result[1024], int argc, const char* argv[]);

extern int NlviewCookie(
	char result[1024],	/* IN/OUT: returned cookie or error message */
	const char* wname,	/* IN: name of Nlview widget to license */
	int flags,		/* IN: control flags (see above) */
	unsigned long salt);	/* IN: number needed for salt-based cookies */

extern int NlviewGetNotice(char result[1024] /* OUT: NOTICE or error */);
extern int NlviewUncheck(  char result[1024] /* OUT: error message */);


#ifdef __cplusplus
}
#endif

#endif
