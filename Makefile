KMOD=	udp_hook
SRCS=	WjCryptLib_Sha256.c udp_io.c chat_server.c udp_hook.c

.include <bsd.kmod.mk> opt_inet.h opt_inet6.h opt_ipsec.h opt_rss.h
