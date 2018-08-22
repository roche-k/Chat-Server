#include "chat_server.h"
#include "udp_io.h"

static unsigned int cli_count = 0;
static int uid = 10;

client_t *clients[MAX_CLIENTS];

/* Find client to queue */
static int queue_find(struct sockaddr_in* addr) 
{
	int i, fake_count = cli_count;
	for(i = 0; i < fake_count; i++) {
		if (i == MAX_CLIENTS) break;
		if (!clients[i]) {
			fake_count += 1;
			continue;
		}
		if (memcmp(&clients[i]->addr, addr, sizeof(struct sockaddr_in)) == 0) {
			return i;
		}
	}
	return -1;
}

/* Add client to queue */
static void queue_add(client_t *cl)
{
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(!clients[i]){
			clients[i] = cl;
			return;
		}
	}
}

/* Delete client from queue */
static void queue_delete(int uid)
{
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				return;
			}
		}
	}
}

/* Send message to all clients but the sender */
static void send_message(char *s, int uid)
{
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			if(clients[i]->uid != uid){
				//write(clients[i]->connfd, s, strlen(s));
			}
		}
	}
}

/* Send message to all clients */
static void send_message_all(char *s)
{
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			//write(clients[i]->connfd, s, strlen(s));
		}
	}
}

/* Send message to sender */
static void send_message_self(const char *s, int connfd)
{
	//write(connfd, s, strlen(s));
}

/* Send message to client */
static void send_message_client(char *s, int uid)
{
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			if(clients[i]->uid == uid){
				//write(clients[i]->connfd, s, strlen(s));
			}
		}
	}
}

/* Send list of active clients */
static void send_active_clients(int connfd)
{
	int i;
	char s[64];
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			sprintf(s, "<<CLIENT %d | %s\r\n", clients[i]->uid, clients[i]->name);
			send_message_self(s, connfd);
		}
	}
}

/* Strip CRLF */
static void strip_newline(char *s)
{
	while(*s != '\0'){
		if(*s == '\r' || *s == '\n'){
			*s = '\0';
		}
		s++;
	}
}

/* Print ip address */
static void print_client_addr(struct sockaddr_in addr)
{
	print_chat("%d.%d.%d.%d",
		addr.sin_addr.s_addr & 0xFF,
		(addr.sin_addr.s_addr & 0xFF00)>>8,
		(addr.sin_addr.s_addr & 0xFF0000)>>16,
		(addr.sin_addr.s_addr & 0xFF000000)>>24);
}

/* Handle all communication with the client */
static void *handle_client(client_t *cli, char* data, int len)
{
	char buff_out[1024];
	char buff_in[1024];
	int rlen = strlen(data);

	cli_count++;

	print_chat("<<ACCEPT ");
	print_client_addr(cli->addr);
	print_chat(" REFERENCED BY %d\n", cli->uid);

	sprintf(buff_out, "<<JOIN, HELLO %s\r\n", cli->name);
	send_message_all(buff_out);

	/* Receive input from client */
	//while((rlen = read(cli->connfd, buff_in, sizeof(buff_in)-1)) > 0){
	do 
	{
	    buff_in[rlen] = '\0';
	    buff_out[0] = '\0';
		strip_newline(buff_in);

		/* Ignore empty buffer */
		if(!strlen(buff_in)){
			return NULL;
		}
	
		/* Special options */
		if(buff_in[0] == '\\'){
			char *command, *param;
			command = strtok(buff_in," ");
			if(!strcmp(command, "\\QUIT")){
				break;
			}else if(!strcmp(command, "\\PING")){
				send_message_self("<<PONG\r\n", cli->connfd);
			}else if(!strcmp(command, "\\NAME")){
				param = strtok(NULL, " ");
				if(param){
					char *old_name = strdup(cli->name, M_STATFS);
					strcpy(cli->name, param);
					sprintf(buff_out, "<<RENAME, %s TO %s\r\n", old_name, cli->name);
					free(old_name, M_STATFS);
					send_message_all(buff_out);
				}else{
					send_message_self("<<NAME CANNOT BE NULL\r\n", cli->connfd);
				}
			}else if(!strcmp(command, "\\PRIVATE")){
				param = strtok(NULL, " ");
				if(param){
					int uid = atoi(param);
					param = strtok(NULL, " ");
					if(param){
						sprintf(buff_out, "[PM][%s]", cli->name);
						while(param != NULL){
							strcat(buff_out, " ");
							strcat(buff_out, param);
							param = strtok(NULL, " ");
						}
						strcat(buff_out, "\r\n");
						send_message_client(buff_out, uid);
					}else{
						send_message_self("<<MESSAGE CANNOT BE NULL\r\n", cli->connfd);
					}
				}else{
					send_message_self("<<REFERENCE CANNOT BE NULL\r\n", cli->connfd);
				}
			}else if(!strcmp(command, "\\ACTIVE")){
				sprintf(buff_out, "<<CLIENTS %d\r\n", cli_count);
				send_message_self(buff_out, cli->connfd);
				send_active_clients(cli->connfd);
			}else if(!strcmp(command, "\\HELP")){
				strcat(buff_out, "\\QUIT     Quit chatroom\r\n");
				strcat(buff_out, "\\PING     Server test\r\n");
				strcat(buff_out, "\\NAME     <name> Change nickname\r\n");
				strcat(buff_out, "\\PRIVATE  <reference> <message> Send private message\r\n");
				strcat(buff_out, "\\ACTIVE   Show active clients\r\n");
				strcat(buff_out, "\\HELP     Show help\r\n");
				send_message_self(buff_out, cli->connfd);
			}else{
				send_message_self("<<UNKOWN COMMAND\r\n", cli->connfd);
			}
		}else{
			/* Send message */
			sprintf(buff_out, "[%s] %s\r\n", cli->name, buff_in);
			send_message(buff_out, cli->uid);
		}
	} while (0);

	/* Close connection */
	sprintf(buff_out, "<<LEAVE, BYE %s\r\n", cli->name);
	send_message_all(buff_out);

	/* Delete client from queue and yeild thread */
	queue_delete(cli->uid);
	print_chat("<<LEAVE ");
	print_client_addr(cli->addr);
	print_chat(" REFERENCED BY %d\n", cli->uid);
	free(cli, M_STATFS);
	cli_count--;
	
	return NULL;
}

int server_recive (struct sockaddr_in* addr, char* data, int len)
{
	client_t *cli;
	int i;

	print_chat("<[SERVER STARTED]>\n");

	/* Check if max clients is reached */
	if((cli_count + 1) == MAX_CLIENTS) {
		print_chat("<<MAX CLIENTS REACHED\n");
		print_chat("<<REJECT ");
		print_client_addr(*addr);
		print_chat("\n");
		return 0;
	}

	if ((i = queue_find(addr)) < 0) {
		/* Client settings */
		cli = (client_t *)malloc(sizeof(client_t), M_STATFS, M_WAITOK);
		cli->addr = *addr;
		cli->uid = uid++;
		sprintf(cli->name, "%d", cli->uid);

		/* Add client to the queue and fork thread */
		queue_add(cli);
	} else {
		cli = clients[i];
	}
	handle_client(cli, data, len);

	return 1;
}
