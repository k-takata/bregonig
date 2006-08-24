/*
 *	dbgtrace.h
 */
/*
 * Copyright (C) 2006  K.Takata
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

#ifdef _DEBUG
#pragma comment(lib, "user32.lib")

#define TRACE0(msg)	\
	{	\
		TCHAR buf_[1024]; wsprintf(buf_, TEXT("%s"), msg); \
		OutputDebugString(buf_);	\
	}
#define TRACE1(msg, p1)	\
	{	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1); \
		OutputDebugString(buf_);	\
	}
#define TRACE2(msg, p1, p2)	\
	{	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1, p2); \
		OutputDebugString(buf_);	\
	}
#define TRACE3(msg, p1, p2, p3)	\
	{	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1, p2, p3); \
		OutputDebugString(buf_);	\
	}
#define TRACE4(msg, p1, p2, p3, p4)	\
	{	\
		TCHAR buf_[1024]; wsprintf(buf_, msg, p1, p2, p3, p4); \
		OutputDebugString(buf_);	\
	}
#else
#define TRACE0(msg)
#define TRACE1(msg, p1)
#define TRACE2(msg, p1, p2)
#define TRACE3(msg, p1, p2, p3)
#define TRACE4(msg, p1, p2, p3, p4)
#endif


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
