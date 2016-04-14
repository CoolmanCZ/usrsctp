/*
   The following code is an implementation of a simple client that sends
   a number of messages marked for unordered delivery to an echo server
   making use of all outgoing streams.  The example shows how to use
   some features of one-to-one style IPv4 SCTP sockets, including

   o  Creating and connecting an SCTP socket.
   o  Making a request to negotiate a number of outgoing streams.
   o  Determining the negotiated number of outgoing streams.
   o  Setting an adaptation layer indication.
   o  Sending messages with a given payload protocol identifier on a
      particular stream using sctp_sendv().

   <CODE BEGINS>

   Copyright (c) 2011 IETF Trust and the persons identified
   as authors of the code.  All rights reserved.

   Redistribution and use in source and binary forms, with
   or without modification, is permitted pursuant to, and subject
   to the license terms contained in, the Simplified BSD License
   set forth in Section 4.c of the IETF Trust's Legal Provisions
   Relating to IETF Documents (http://trustee.ietf.org/license-info).
*/

#if !defined(_WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <io.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <stdlib.h>

#include <stdarg.h>
#include <usrsctp.h>

#if !defined(HAVE_INET_NTOP) || !defined(HAVE_INET_PTON)
#include "inet_functions.h"
#endif

#define PORT 9
#define ADDR "127.0.0.1"
#define SIZE_OF_MESSAGE 1000
#define NUMBER_OF_MESSAGES 10
#define PPID 1234

#define USRSCTP_UDP_ENCAPS_PORT 9899
#define LOCAL_PORT 5001
#define LOCAL_ENCAPS_PORT 5002

void debug_printf(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

void bind_local_port(struct socket *sock, uint16_t port)
{
    struct sockaddr_in6 addr6;

    memset((void *)&addr6, 0, sizeof(struct sockaddr_in6));
#ifdef HAVE_SIN6_LEN
    addr6.sin6_len = sizeof(struct sockaddr_in6);
#endif
    addr6.sin6_family = AF_INET6;
    addr6.sin6_port = htons(port);
    addr6.sin6_addr = in6addr_any;
    printf("Local port = %d\n", port);
    if (usrsctp_bind(sock, (struct sockaddr *)&addr6, sizeof(struct sockaddr_in6)) < 0) {
        perror("bind");
    }
}

void set_remote_encaps_port(struct socket *sock, uint16_t port)
{
    struct sctp_udpencaps encaps;

    memset(&encaps, 0, sizeof(struct sctp_udpencaps));
    encaps.sue_address.ss_family = AF_INET6;
    encaps.sue_port = htons(port);
    printf("Remote UDP encapsulation port = %d\n", port);
    if (usrsctp_setsockopt
        (sock, IPPROTO_SCTP, SCTP_REMOTE_UDP_ENCAPS_PORT, (const void *)&encaps,
         (socklen_t) sizeof(struct sctp_udpencaps)) < 0) {
        perror("setsockopt SCTP_REMOTE_UDP_ENCAPS_PORT");
    }
}

int main(void)
{
    unsigned int i;
    struct socket *sd;
    struct sockaddr_in addr;
    char buffer[SIZE_OF_MESSAGE];
    struct sctp_status status;
    struct sctp_initmsg init;
    struct sctp_sndinfo info;
    struct sctp_setadaptation ind;
    socklen_t opt_len;

    usrsctp_init(LOCAL_ENCAPS_PORT, NULL, debug_printf);
    printf("Local UDP encapsulation port  = %d\n", LOCAL_ENCAPS_PORT);

    /* Create a one-to-one style SCTP socket. */
    if ((sd = usrsctp_socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP, NULL, NULL, 0, NULL)) == NULL) {
        perror("usrsctp_socket");
        exit(1);
    }

    bind_local_port(sd, LOCAL_PORT);
    set_remote_encaps_port(sd, USRSCTP_UDP_ENCAPS_PORT);

    /* Prepare for requesting 2048 outgoing streams. */
    memset(&init, 0, sizeof(init));
    init.sinit_num_ostreams = 2048;
    if (usrsctp_setsockopt(sd, IPPROTO_SCTP, SCTP_INITMSG, &init, (socklen_t) sizeof(init)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    ind.ssb_adaptation_ind = 0x01020304;
    if (usrsctp_setsockopt(sd, IPPROTO_SCTP, SCTP_ADAPTATION_LAYER, &ind, (socklen_t) sizeof(ind)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    /* Connect to the discard server. */
    printf("Trying to connect IP=%s and port=%d\n", ADDR, PORT);
    memset(&addr, 0, sizeof(addr));
#ifdef HAVE_SIN_LEN
    addr.sin_len = sizeof(struct sockaddr_in);
#endif
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
	inet_pton(AF_INET, ADDR, &addr.sin_addr);
	//addr.sin_addr.s_addr = inet_addr(ADDR);

    if (usrsctp_connect(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
        perror("connect");
        exit(1);
    }

    /* Get the actual number of outgoing streams. */
    memset(&status, 0, sizeof(status));
    opt_len = (socklen_t) sizeof(status);
    if (usrsctp_getsockopt(sd, IPPROTO_SCTP, SCTP_STATUS, &status, &opt_len) < 0) {
        perror("getsockopt");
        exit(1);
    }

    memset(&info, 0, sizeof(info));
    info.snd_ppid = htonl(PPID);
    info.snd_flags = SCTP_UNORDERED;
    memset(buffer, 'A', SIZE_OF_MESSAGE);
    for (i = 0; i < NUMBER_OF_MESSAGES; i++) {
        info.snd_sid = i % status.sstat_outstrms;
        if (usrsctp_sendv(sd, buffer, SIZE_OF_MESSAGE, NULL, 0, &info, sizeof(info), SCTP_SENDV_SNDINFO, 0) < 0) {
            perror("sctp_sendv");
            exit(1);
        }
    }

    usrsctp_close(sd);
    while (usrsctp_finish() != 0) {
#if defined(_WIN32)
        Sleep(1000);
#else
        sleep(1);
#endif
    }
    return (0);
}
