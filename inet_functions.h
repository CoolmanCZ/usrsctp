#ifndef _INET_FUNCTIONS_H_
#define _INET_FUNCTIONS_H_

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <ws2tcpip.h>
#endif

#if !defined(HAVE_INET_NTOP)
char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
#endif

#if !defined(HAVE_INET_PTON)
int inet_pton(int af, const char *src, void *dst);
#endif

#endif
