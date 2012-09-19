
#ifndef GW_STRING_H
#define GW_STRING_H

#ifndef HAVE_BZERO
#	define HAVE_BZERO
#	define bzero(buf,sz) (memset(buf,0,sz))
#endif

#ifndef gwerr
#	define gwerr(str) fprintf(stderr,"gwrl error: %s\n",str)
#endif

#ifndef gwwrn
#	define gwwrn(str) fprintf(stderr,"gwrl warning: %s\n",str)
#endif

#ifndef gwprintsyserr
#	if !defined(PLATFORM_WINDOWS)
#		define gwprintsyserr(prefix,errnm) fprintf(stderr,"%s: (%i), %s\n",prefix,errnm,strerror(errnm));
#	else
#		define gwprintsyserr(prefix,errnm) LPSTR msg;\
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,0,errnm,0,(LPSTR)&msg,0,0);\
		fprintf(stderr,"%s: (%i), %s\n",prefix,errnm,msg);\
		LocalFree(msg);\

#	endif
#endif

#endif
