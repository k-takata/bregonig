/*
 *	dbgtrace.h
 */
/*
 * Copyright (C) 2006-2011  K.Takata
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef DBGTRACE_H_
#define DBGTRACE_H_


/*** Debugging Routines ***/

#if defined(__cplusplus) || defined(_MSC_VER)
#ifdef __cplusplus
inline
#else /* __cplusplus */
__inline
#endif /* __cplusplus */
void TRACEx_(LPCTSTR msg, ...)
{
	TCHAR buf_[1024];
	va_list ap;
	va_start(ap, msg);
	wvsprintf(buf_, msg, ap);
	va_end(ap);
	OutputDebugString(buf_);
}
#endif /* __cplusplus || _MSC_VER */

#pragma comment(lib, "user32.lib")

#if defined(_DEBUG) || defined(DEBUG)
#include <stdarg.h>

#define TRACE0(msg)	OutputDebugString(msg)
#define TRACE1(msg, p1)	\
	do {	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1); \
		OutputDebugString(buf_);	\
	} while(0)
#define TRACE2(msg, p1, p2)	\
	do {	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1, p2); \
		OutputDebugString(buf_);	\
	} while(0)
#define TRACE3(msg, p1, p2, p3)	\
	do {	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1, p2, p3); \
		OutputDebugString(buf_);	\
	} while(0)
#define TRACE4(msg, p1, p2, p3, p4)	\
	do {	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1, p2, p3, p4); \
		OutputDebugString(buf_);	\
	} while(0)
#if defined(__cplusplus) || defined(_MSC_VER)
#define TRACE	TRACEx_
#endif /* __cplusplus || _MSC_VER */

/*
#define ASSERT(x)	\
	do {	\
		if (!(x)) { \
			TRACE2(TEXT("Assertion failed! in %s (%d)\n"), __FILE__, __LINE__); \
			DebugBreak(); \
		}	\
	} while(0)
#define VERIFY(x)	ASSERT(x)
*/

#else /*_DEBUG */

#define TRACE0(msg)
#define TRACE1(msg, p1)
#define TRACE2(msg, p1, p2)
#define TRACE3(msg, p1, p2, p3)
#define TRACE4(msg, p1, p2, p3, p4)
#if defined(__cplusplus) || defined(_MSC_VER)
#define TRACE	1 ? (void) 0 : TRACEx_
#endif /* __cplusplus || _MSC_VER */

/*
#define ASSERT(x)	((void) 0)
#define VERIFY(x)	((void) x)
*/

#endif /* _DEBUG */


/*
#ifdef _DEBUG
  #define DEBUG_CLIENTBLOCK	new(_CLIENT_BLOCK, __FILE__, __LINE__)
#else
  #define DEBUG_CLIENTBLOCK
#endif // _DEBUG
*/
#ifdef  _DEBUG
  #define DEBUG_NEW	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
  #define DEBUG_NEW	new
#endif


#endif /* DBGTRACE_H_ */
