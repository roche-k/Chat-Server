///////////////////////////////////////////////////////////////////////////////////////////////////////
//  INCLUDES
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////
//  DEFINES
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

/* DEBUG */
#define DEBUG_SERVER
#ifdef DEBUG_SERVER
#define print_chat(info, ...) uprintf("SERVER DEBUG %s(%d) %s: "info, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define print_chat(...)
#endif

#define MAX_CLIENTS	100

/* Client structure */
typedef struct {
	struct sockaddr_in addr;	/* Client remote address */
	int connfd;			/* Connection file descriptor */
	int uid;			/* Client unique identifier */
	char name[32];			/* Client name */
} client_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////
//  CUNCTIONS
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

static int queue_find(struct sockaddr_in* addr);
static void queue_add(client_t *cl);
static void queue_delete(int uid);
static void send_message(char *s, int uid);
static void send_message_all(char *s);
static void send_message_self(const char *s, int connfd);
static void send_message_client(char *s, int uid);
static void send_active_clients(int connfd);
static void strip_newline(char *s);
static void print_client_addr(struct sockaddr_in addr);
static void *handle_client(client_t *cli, char* data, int len);
int server_recive (struct sockaddr_in* addr, char* data, int len);

