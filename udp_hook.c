#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/route.h>
#include <net/rss_config.h>

#include <netinet/in.h>
#include <netinet/in_kdtrace.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/ip_var.h>
#include <netinet/ip_options.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/udplite.h>
#include <netinet/in_rss.h>

extern struct protosw inetsw[];
pr_input_t udp_input_hook;

/* udp_input hook. */
int
udp_input_hook(struct mbuf **mp, int *offp, int proto)
{
	struct ip *ip;
	int iphlen = *offp;
	struct udphdr *uh;
	struct mbuf *m;
	struct sockaddr_in udp_in;

	m = *mp;
	/*
	 * Strip IP options, if any; should skip this, make available to
	 * user, and use on returned packets, but we don't yet have a way to
	 * check the checksum with options still present.
	 */
	if (iphlen > sizeof (struct ip)) {
		ip_stripoptions(m);
		iphlen = sizeof(struct ip);
	}

	/*
	 * Get IP and UDP header together in first mbuf.
	 */
	ip = mtod(m, struct ip *);
	if (m->m_len < iphlen + sizeof(struct udphdr)) {
		if ((m = m_pullup(m, iphlen + sizeof(struct udphdr))) == NULL) {

			UDPSTAT_INC(udps_hdrops);
			return (IPPROTO_DONE);
		}
		ip = mtod(m, struct ip *);
	}
	uh = (struct udphdr *)((caddr_t)ip + iphlen);

	/* Locate */
	m->m_len -= iphlen + sizeof(struct udphdr);
	m->m_data += iphlen + sizeof(struct udphdr);

	/*
	 * Construct sockaddr format source address.  Stuff source address
	 * and datagram in user buffer.
	 */
	bzero(&udp_in, sizeof(udp_in));
	udp_in.sin_len = sizeof(udp_in);
	udp_in.sin_family = AF_INET;
	udp_in.sin_port = uh->uh_sport;
	udp_in.sin_addr = ip->ip_src;

	printf("load=%s \n", m->m_data);

	/* Locate */
	m->m_len += iphlen + sizeof(struct udphdr);
	m->m_data -= iphlen + sizeof(struct udphdr);

	return udp_input(mp, offp, proto);
}

/* The function called at load/unload. */
static int
load(struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch (cmd) {
	case MOD_LOAD:
		/* Replace icmp_input with icmp_input_hook. */
		inetsw[ip_protox[IPPROTO_UDP]].pr_input = udp_input_hook;
		break;

	case MOD_UNLOAD:
		/* Change everything back to normal. */
		inetsw[ip_protox[IPPROTO_UDP]].pr_input = udp_input;
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	return(error);
}

static moduledata_t udp_input_hook_mod = {
	"udp_input_hook",	/* module name */
	load,			/* event handler */
	NULL			/* extra data */
};

DECLARE_MODULE(udp_input_hook, udp_input_hook_mod, SI_SUB_DRIVERS,
    SI_ORDER_MIDDLE);
