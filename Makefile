KMOD=	udp_hook
SRCS=	udp_hook.c

.include <bsd.kmod.mk> opt_inet.h opt_inet6.h opt_ipsec.h opt_rss.h
