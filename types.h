//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//_/_/
//_/_/ HUMANOBS - CoreLibrary
//_/_/
//_/_/ Eric Nivel, Thor List
//_/_/ Center for Analysis and Design of Intelligent Agents
//_/_/   Reykjavik University, Menntavegur 1, 101 Reykjavik, Iceland
//_/_/   http://cadia.ru.is
//_/_/ Copyright(c)2012
//_/_/
//_/_/ This software was developed by the above copyright holder as part of 
//_/_/ the HUMANOBS EU research project, in collaboration with the 
//_/_/ following parties:
//_/_/ 
//_/_/ Autonomous Systems Laboratory
//_/_/   Technical University of Madrid, Spain
//_/_/   http://www.aslab.org/
//_/_/
//_/_/ Communicative Machines
//_/_/   Edinburgh, United Kingdom
//_/_/   http://www.cmlabs.com/
//_/_/
//_/_/ Istituto Dalle Molle di Studi sull'Intelligenza Artificiale
//_/_/   University of Lugano and SUPSI, Switzerland
//_/_/   http://www.idsia.ch/
//_/_/
//_/_/ Institute of Cognitive Sciences and Technologies
//_/_/   Consiglio Nazionale delle Ricerche, Italy
//_/_/   http://www.istc.cnr.it/
//_/_/
//_/_/ Dipartimento di Ingegneria Informatica
//_/_/   University of Palermo, Italy
//_/_/   http://roboticslab.dinfo.unipa.it/index.php/Main/HomePage
//_/_/
//_/_/
//_/_/ --- HUMANOBS Open-Source BSD License, with CADIA Clause v 1.0 ---
//_/_/
//_/_/ Redistribution and use in source and binary forms, with or without 
//_/_/ modification, is permitted provided that the following conditions 
//_/_/ are met:
//_/_/
//_/_/ - Redistributions of source code must retain the above copyright 
//_/_/ and collaboration notice, this list of conditions and the 
//_/_/ following disclaimer.
//_/_/
//_/_/ - Redistributions in binary form must reproduce the above copyright 
//_/_/ notice, this list of conditions and the following
//_/_/ disclaimer in the documentation and/or other materials provided 
//_/_/ with the distribution.
//_/_/
//_/_/ - Neither the name of its copyright holders nor the names of its 
//_/_/ contributors may be used to endorse or promote products 
//_/_/ derived from this software without specific prior written permission.
//_/_/
//_/_/ - CADIA Clause: The license granted in and to the software under this 
//_/_/ agreement is a limited-use license. The software may not be used in 
//_/_/ furtherance of: 
//_/_/ (i) intentionally causing bodily injury or severe emotional distress 
//_/_/ to any person; 
//_/_/ (ii) invading the personal privacy or violating the human rights of 
//_/_/ any person; or 
//_/_/ (iii) committing or preparing for any act of war.
//_/_/
//_/_/ THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//_/_/ "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//_/_/ LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
//_/_/ A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//_/_/ OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//_/_/ SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//_/_/ LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
//_/_/ DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
//_/_/ THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//_/_/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//_/_/ OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//_/_/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef core_types_h
#define core_types_h

#include	<cstddef>

#if defined(WIN32)
	#define	WINDOWS
	#define	ARCH_32
#elif defined(WIN64)
	#define	WINDOWS
	#define	ARCH_64
#elif defined(__GNUC__)

#if __GNUC__ == 4
#if __GNUC_MINOR__ < 3
#error "GNU C++ 4.3 or later is required to compile this program"
#endif /* __GNUC_MINOR__ */
#endif /* __GNUC__ */

	#if defined(__x86_64)
		#define	ARCH_64
	#elif defined(__i386)
		#define	ARCH_32
	#endif
	#if defined(__linux)
		#define	LINUX
	#endif
#endif

#if defined WINDOWS

	#define	WIN32_LEAN_AND_MEAN
	#define	_WIN32_WINNT	0x0501	//	i.e. win xp sp2
	#include	<windows.h>
#ifndef AF_MAX
	#include	<winsock2.h>
#endif

	#include	<vector>
	#include	<unordered_set>
	#include	<unordered_map>
	#define		UNORDERED_MAP		std::tr1::unordered_map
	#define		UNORDERED_SET		std::tr1::unordered_set
	#define		UNORDERED_MULTIMAP	std::tr1::unordered_multimap
	#define		UNORDERED_MULTISET	std::tr1::unordered_multiset

	#if defined	CORELIBRARY_EXPORTS
		#define core_dll	__declspec(dllexport)
	#else
		#define core_dll	__declspec(dllimport)
	#endif
	#define	dll_export	__declspec(dllexport)
	#define	dll_import	__declspec(dllimport)
	
	#pragma	warning(disable:	4530)	//	warning: exception disabled
	#pragma	warning(disable:	4996)	//	warning: this function may be unsafe
	#pragma	warning(disable:	4800)	//	warning: forcing value to bool
	#pragma	warning(disable:	4251)	//	warning: class xxx needs to have dll-interface to be used by clients of class yyy

#if 0 // Shut off these warnings.
	#if( _SECURE_SCL != 0 )
        #pragma message( "Warning: _SECURE_SCL != 0. You _will_ get either slowness or runtime errors." )
    #endif

    #if( _HAS_ITERATOR_DEBUGGING != 0 )
        #pragma message( "Warning: _HAS_ITERATOR_DEBUGGING != 0. You _will_ get either slowness or runtime errors." )
    #endif
#endif

#elif defined LINUX
	#define core_dll
	#include <iostream>
	#include <string>
	#include <pthread.h>
	#include <semaphore.h>
	#include <signal.h>
	#include <limits.h>

	#include	<vector>
	#include	<unordered_map>
	#include	<unordered_set>
	#define		UNORDERED_MAP	std::unordered_map
	#define		UNORDERED_SET	std::unordered_set
	#define		UNORDERED_MULTIMAP	std::unordered_multimap
	#define		UNORDERED_MULTISET	std::unordered_multiset

	#define dll_export __attribute((visibility("default")))
	#define dll_import __attribute((visibility("default")))
	#define cdecl

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/ip.h>
	#include <arpa/inet.h>
	#define SOCKET_ERROR	-1
	#define INVALID_SOCKET	-1
	#define closesocket(X)	close(X)
#else
	#error "This is a new platform"
#endif

#define	NEWLINE	'\n'

namespace	core{

#define	WORD32_MASK					0xFFFFFFFF

typedef	long						word32;
typedef	short						word16;

typedef	char						int8;
typedef	unsigned	char			uint8;
typedef	short						int16;
typedef	unsigned	short			uint16;
typedef	float						float32;
typedef	double						float64;

typedef	word32						word;
typedef	word16						half_word;

#if defined	WINDOWS
	typedef	long					int32;
	typedef	unsigned	long		uint32;
	typedef	__int64					int64;
	typedef	unsigned __int64		uint64;
#else
	typedef	int						int32;
	typedef	unsigned	int			uint32;
	typedef	int64_t					int64;
	typedef	uint64_t				uint64;
#endif

#if defined	ARCH_32

	#define	HALF_WORD_SHIFT				16
	#define	HALF_WORD_HIGH_MASK			0xFFFF0000
	#define	HALF_WORD_LOW_MASK			0x0000FFFF
	#define	WORD_MASK					0xFFFFFFFF

#elif defined	ARCH_64
	// UPS!
	// typedef	unsigned	long			uint64;

	#define	HALF_WORD_SHIFT				32
	#define	HALF_WORD_HIGH_MASK			0xFFFFFFFF00000000
	#define	HALF_WORD_LOW_MASK			0x00000000FFFFFFFF
	#define	WORD_MASK					0xFFFFFFFFFFFFFFFF

#endif

#if defined	WINDOWS
	typedef	HINSTANCE						shared_object;
	typedef	HANDLE							thread;
	#define thread_ret						core::uint32
	#define thread_ret_val(ret)					return ret;
	typedef	LPTHREAD_START_ROUTINE			thread_function;
	#define	thread_function_call			WINAPI
	typedef	SOCKET							socket;
	typedef	HANDLE							semaphore;
	typedef	HANDLE							mutex;
	typedef	CRITICAL_SECTION				critical_section;
	typedef	HANDLE							timer;
	typedef	HANDLE							event;
	#define	signal_handler_function_call	WINAPI
	typedef	PHANDLER_ROUTINE				signal_handler;
#elif defined	LINUX
	typedef void *							shared_object;
	typedef pthread_t						thread;
	#define thread_ret						void *
	#define thread_ret_val(ret)				pthread_exit((thread_ret)ret);
	typedef thread_ret (*thread_function)(void *);
	#define thread_function_call
	typedef int								socket;
	typedef struct sockaddr					SOCKADDR;
	typedef	sem_t							semaphore;
	typedef pthread_mutex_t					mutex;
	typedef pthread_mutex_t					critical_section;
	typedef timer_t							timer;
	#define signal_handler_function_call
	typedef sighandler_t					signal_handler;
	#define stricmp							strcasecmp
#endif

}


#endif
