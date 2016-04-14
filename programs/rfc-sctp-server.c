/*
   The following code is a simple implementation of a discard server
   over SCTP.  The example shows how to use some features of one-to-many
   style IPv6 SCTP sockets, including

   o  Opening and binding of a socket.
   o  Enabling notifications.
   o  Handling notifications.
   o  Configuring the auto-close timer.
   o  Using sctp_recvv() to receive messages.

   Please note that this server can be used in combination with the
   client described in Appendix A.

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
#include <stdlib.h>
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include <stdarg.h>
#include <usrsctp.h>

#if !defined(HAVE_INET_NTOP) || !defined(HAVE_INET_PTON)
#include "inet_functions.h"
#endif

#define BUFFER_SIZE (1<<16)
#define PORT 9
#define ADDR "0.0.0.0"
#define TIMEOUT 5

#define USRSCTP_UDP_ENCAPS_PORT 9899

void debug_printf(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

static void print_notification(void *buf)
{
    struct sctp_assoc_change *sac;
    struct sctp_paddr_change *spc;
    struct sctp_adaptation_event *sad;
    union sctp_notification *snp;
    char addrbuf[INET6_ADDRSTRLEN];
    const char *ap;
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;

    snp = buf;

    switch (snp->sn_header.sn_type) {
    case SCTP_ASSOC_CHANGE:
        sac = &snp->sn_assoc_change;
        printf("^^^ Association change: ");
        switch (sac->sac_state) {
        case SCTP_COMM_UP:
            printf("Communication up (streams (in/out)=(%u/%u)).\n",
                   sac->sac_inbound_streams, sac->sac_outbound_streams);
            break;
        case SCTP_COMM_LOST:
            printf("Communication lost (error=%d).\n", sac->sac_error);
            break;
        case SCTP_RESTART:
            printf("Communication restarted (streams (in/out)=(%u/%u).\n",
                   sac->sac_inbound_streams, sac->sac_outbound_streams);
            break;
        case SCTP_SHUTDOWN_COMP:
            printf("Communication completed.\n");
            break;
        case SCTP_CANT_STR_ASSOC:
            printf("Communication couldn't be started.\n");
            break;
        default:
            printf("Unknown state: %d.\n", sac->sac_state);
            break;
        }
        break;
    case SCTP_PEER_ADDR_CHANGE:
        spc = &snp->sn_paddr_change;
        if (spc->spc_aaddr.ss_family == AF_INET) {
            sin = (struct sockaddr_in *)&spc->spc_aaddr;
            ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf, INET6_ADDRSTRLEN);
        } else {
            sin6 = (struct sockaddr_in6 *)&spc->spc_aaddr;
            ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf, INET6_ADDRSTRLEN);
        }
        printf("^^^ Peer Address change: %s ", ap);
        switch (spc->spc_state) {
        case SCTP_ADDR_AVAILABLE:
            printf("is available.\n");
            break;
        case SCTP_ADDR_UNREACHABLE:
            printf("is not available (error=%d).\n", spc->spc_error);
            break;
        case SCTP_ADDR_REMOVED:
            printf("was removed.\n");
            break;
        case SCTP_ADDR_ADDED:
            printf("was added.\n");
            break;
        case SCTP_ADDR_MADE_PRIM:
            printf("is primary.\n");
            break;
        default:
            printf("unknown state (%d).\n", spc->spc_state);
            break;
        }
        break;
    case SCTP_SHUTDOWN_EVENT:
        printf("^^^ Shutdown received.\n");
        break;
    case SCTP_ADAPTATION_INDICATION:
        sad = &snp->sn_adaptation_event;
        printf("^^^ Adaptation indication 0x%08x received.\n", sad->sai_adaptation_ind);
        break;
    default:
        printf("^^^ Unknown event of type: %u.\n", snp->sn_header.sn_type);
        break;
    };
}

int main(void)
{
    int flags, timeout, on;
    struct socket *sd;
    ssize_t n;
    unsigned int i;
    union {
        struct sockaddr sa;
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
    } addr;
    socklen_t fromlen, infolen;
    struct sctp_rcvinfo info;
    unsigned int infotype;
    char buffer[BUFFER_SIZE];
    struct sctp_event event;
    uint16_t event_types[] = { SCTP_ASSOC_CHANGE,
        SCTP_PEER_ADDR_CHANGE,
        SCTP_SHUTDOWN_EVENT,
        SCTP_ADAPTATION_INDICATION
    };

    usrsctp_init(USRSCTP_UDP_ENCAPS_PORT, NULL, debug_printf);
    printf("Local UDP encapsulation port  = %d\n", USRSCTP_UDP_ENCAPS_PORT);

    /* Create a one-to-many style SCTP socket. */
    if ((sd = usrsctp_socket(AF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP, NULL, NULL, 0, NULL)) == NULL) {
        perror("socket");
        exit(1);
    }

    /* Enable the events of interest. */
    memset(&event, 0, sizeof(event));
    event.se_assoc_id = SCTP_FUTURE_ASSOC;
    event.se_on = 1;
    for (i = 0; i < sizeof(event_types) / sizeof(uint16_t); i++) {
        event.se_type = event_types[i];
        if (usrsctp_setsockopt(sd, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(event)) < 0) {
            perror("setsockopt");
            exit(1);
        }
    }

    /* Configure auto-close timer. */
    timeout = TIMEOUT;
    if (usrsctp_setsockopt(sd, IPPROTO_SCTP, SCTP_AUTOCLOSE, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt SCTP_AUTOCLOSE");
        exit(1);
    }

    /* Enable delivery of SCTP_RCVINFO. */
    on = 1;
    if (usrsctp_setsockopt(sd, IPPROTO_SCTP, SCTP_RECVRCVINFO, &on, sizeof(on)) < 0) {
        perror("setsockopt SCTP_RECVRCVINFO");
        exit(1);
    }

    /* Bind the socket to all local addresses. */
    memset(&addr, 0, sizeof(addr));
#ifdef HAVE_SIN6_LEN
    addr.sin6.sin6_len = sizeof(addr.sin6);
#endif
    addr.sin6.sin6_family = AF_INET6;
    addr.sin6.sin6_port = htons(PORT);
    addr.sin6.sin6_addr = in6addr_any;
    if (usrsctp_bind(sd, &addr.sa, sizeof(addr.sin6)) < 0) {
        perror("bind");
        exit(1);
    }
    /* Enable accepting associations. */
    if (usrsctp_listen(sd, 1) < 0) {
        perror("listen");
        exit(1);
    }

    for (;;) {
        flags = 0;
        memset(&addr, 0, sizeof(addr));
        fromlen = (socklen_t) sizeof(addr);
        memset(&info, 0, sizeof(info));
        infolen = (socklen_t) sizeof(info);
        infotype = 0;

        n = usrsctp_recvv(sd, buffer, BUFFER_SIZE, &addr.sa, &fromlen, &info, &infolen, &infotype, &flags);

        if (flags & MSG_NOTIFICATION) {
            print_notification(buffer);
        } else {
            char addrbuf[INET6_ADDRSTRLEN];
            const char *ap;
            unsigned int port;

            if (addr.sa.sa_family == AF_INET) {
                ap = inet_ntop(AF_INET, &addr.sin.sin_addr, addrbuf, INET6_ADDRSTRLEN);
                port = ntohs(addr.sin.sin_port);
            } else {
                ap = inet_ntop(AF_INET6, &addr.sin6.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
                port = ntohs(addr.sin6.sin6_port);
            }
            printf("Message received from %s:%u: len=%d", ap, port, (int)n);
            switch (infotype) {
            case SCTP_RECVV_RCVINFO:
                printf(", sid=%u", info.rcv_sid);
                if (info.rcv_flags & SCTP_UNORDERED) {
                    printf(", unordered");
                } else {
                    printf(", ssn=%u", info.rcv_ssn);
                }
                printf(", tsn=%u", info.rcv_tsn);
                printf(", ppid=%u.\n", ntohl(info.rcv_ppid));
                break;
            case SCTP_RECVV_NOINFO:
            case SCTP_RECVV_NXTINFO:
            case SCTP_RECVV_RN:
                printf(".\n");
                break;
            default:
                printf(" unknown infotype.\n");
            }
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
