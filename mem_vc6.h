/*
 *	mem_vc6.h
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


#ifndef MEM_VC6_H_
#define MEM_VC6_H_

#include <new>

#if _MSC_VER < 1300

#include <new.h>


inline int throw_bad_alloc(size_t)
{
	throw std::bad_alloc();
	return 0;
}

inline void set_new_throw_bad_alloc()
{
	_set_new_handler(throw_bad_alloc);
}

inline void *operator new(size_t cb, const std::nothrow_t&) throw()
{
	char *p;
	try {
		p = new char[cb];
	} catch (std::bad_alloc) {
		p = 0;
	}
	return p;
}

inline void operator delete(void *p, const std::nothrow_t&) throw()
{
	delete p;
}


#else /* _MSC_VER */


#define set_new_throw_bad_alloc()	/**/


#endif /* _MSC_VER */

#endif /* MEM_VC6_H_ */
