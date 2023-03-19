#include "server.h"

void * run_server(void * args)
{
	table_t * table = ((server_args_t *) args)->table;
	ui_t * ui = ((server_args_t *) args)->ui;
	if (table == NULL || ui == NULL) {
		return NULL;
	}

	/* Detach from main thread and setup socket to listen for connections */
	int sock;
	if (pthread_detach(pthread_self()) || (sock = tcp_listen()) < 0) {
		log_tui(ui, "Error : failed to initialize server");
		return NULL;
	}

	/* Display socket info (IP & port) */
	fetch_server_info(ui, sock);

	/* Block to accept incoming connections */
	for (;;) {
		handler_args_t * hndlr_args = malloc(sizeof(handler_args_t));
		if (hndlr_args == NULL) {
			return NULL;
		}
		hndlr_args->table = table;
		hndlr_args->ui = ui;

		if (tcp_accept(sock, &hndlr_args->csock, &hndlr_args->ip, &hndlr_args->port) != OK) {
			log_tui(ui, "Error : failed to accept new connection");
			return NULL;
		}

		/* Spawn new thread to handle client */
		pthread_t h_thr;
		if (pthread_create(&h_thr, NULL, handle, hndlr_args) || pthread_detach(h_thr)) {
			log_tui(ui, "Error : failed to create handler thread");
			close(hndlr_args->csock);
			free(hndlr_args);
			return NULL;
		}
	}

	return NULL;
}

void fetch_server_info(ui_t * ui, int sock)
{
    struct ifaddrs * ifaddr;
	if (getifaddrs(&ifaddr) == ERR) {
		ui->ip[0] = '\0';
		return;
	}

	/* Iterate through network interfaces to get IP address */
	for (struct ifaddrs * ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

		/* Get the first interface that is IPv4 and not local */
		if (ifa->ifa_addr->sa_family == AF_INET && ifa->ifa_name[0] != 'l') {
			inet_ntop(AF_INET, &(((struct sockaddr_in *) ifa->ifa_addr)->sin_addr), ui->ip, INET_ADDRSTRLEN);
			break;
		}
	}
	freeifaddrs(ifaddr);

	/* Get port allocated number */
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(sock, (struct sockaddr *)&sin, &len) == ERR) {
		ui->port = 0;
		return;
	}
	ui->port = ntohs(sin.sin_port);
	if (sem_post(ui->update_sem) == ERR) {
		return;
	}
}

void * handle(void * args)
{
	handler_args_t * hndlr_args = (handler_args_t *) args;
	int csock = hndlr_args->csock;
    uint32_t ip = hndlr_args->ip;
    uint16_t port = hndlr_args->port;
	table_t * table = hndlr_args->table;
	ui_t * ui = hndlr_args->ui;

	free(hndlr_args);
	hndlr_args = NULL;

	/* Parse command and topic */
	enum CMD cmd = parse_cmd(csock);
	char * topic = parse_topic(csock);
	if (cmd == CMD_UNDEFINED || topic == NULL) {
		tcp_write(csock, SERVER_MSG_FAIL, strlen(SERVER_MSG_FAIL));
		close(csock);
		return NULL;
	}
	log_connection(ui, ip, port, cmd, topic);

	/* Handle command */
	switch (cmd) {
		case CMD_SUBSCRIBE:
			subscribe(table, topic, csock, ip, port);
			/* Signal to update the UI */
			sem_post(ui->update_sem);
			break;
		case CMD_UNSUBSCRIBE:
			unsubscribe(table, topic, csock, ip, port);
			/* Signal to update the UI */
			sem_post(ui->update_sem);
			break;
		case CMD_PUBLISH:
			publish(table, topic, csock);
			break;
		default:
			tcp_write(csock, SERVER_MSG_FAIL, strlen(SERVER_MSG_FAIL));
			close(csock);
			break;
	}
	
	/* TODO: Don't close connection as it may be needed in publishing */
	free(topic);

	return NULL;
}

void log_connection(ui_t * ui, uint32_t ip, uint16_t port, enum CMD cmd, char * topic)
{
	/* Parse the four fields of the IP address */
	unsigned int f4 = 0xff & ip;
	ip = ip >> 8;
	unsigned int f3 = 0xff & ip;
	ip = ip >> 8;
	unsigned int f2 = 0xff & ip;
	ip = ip >> 8;
	unsigned int f1 = 0xff & ip;

	char temp[200];
	if (cmd == CMD_SUBSCRIBE) {
		sprintf(temp, "Subscribing %u.%u.%u.%u:%u to %s", f1, f2, f3, f4, port, topic);
	} else if (cmd == CMD_UNSUBSCRIBE) {
		sprintf(temp, "Unsubscribing %u.%u.%u.%u:%u from %s", f1, f2, f3, f4, port, topic);
	} else if (cmd == CMD_PUBLISH) {
		sprintf(temp, "%u.%u.%u.%u:%u publishing to %s", f1, f2, f3, f4, port, topic);
	} else {
		return;
	}
	log_tui(ui, temp);
}

enum CMD parse_cmd(int csock)
{
	/* Command is only 1 byte */
	char buf;

	/* If error or EOF, return undefined */
	if (read(csock, &buf, P_CMD_LEN) <= 0) {
		return CMD_UNDEFINED;
	}

	if (buf == P_CMD_SUBSCRIBE) {
		return CMD_SUBSCRIBE;
	}
	if (buf == P_CMD_UNSUBSCRIBE) {
		return CMD_UNSUBSCRIBE;
	}
	if (buf == P_CMD_PUBLISH) {
		return CMD_PUBLISH;
	}
	return CMD_UNDEFINED;
}

char * parse_topic(int csock)
{
	/* Topic is at length P_TOPIC_LEN (+ null terminator) */
	char * topic = malloc(P_TOPIC_LEN+1);
	if (topic == NULL) {
		return NULL;
	}

	/* Since TCP is byte-stream, there can be cases where it reads less than */
	/* expected although it should read more. But, I think the size is small */
	/* enough to ignore that for now. (famouse last words)                   */
	char buf[P_TOPIC_LEN+1] = {0};
	size_t ret;
	if ((ret = read(csock, buf, P_TOPIC_LEN)) < 0) {
		free(topic);
		return NULL;
	}

	/* Skipping non-alphanumerical in buffer */
	int index = 0;
	for (int i = 0; i < ret; i++) {
		if ((buf[i] >= '0' && buf[i] <= '9') ||
			(buf[i] >= 'a' && buf[i] <= 'z') ||
			(buf[i] >= 'A' && buf[i] <= 'Z') ||
			(buf[i] == ' '))
		{
			topic[index++] = buf[i];
		}
	}

	/* Pad it with spaces */
	while (index < P_TOPIC_LEN) {
		topic[index] = ' ';
		index++;
	}
	topic[index] = '\0';
	topic[P_TOPIC_LEN] = '\0';

	return topic;
}

void subscribe(table_t * table, char * topic, int csock, uint32_t ip, uint16_t port)
{
	subscriber_t * subscriber = malloc(sizeof(subscriber_t));
	if (subscriber == NULL) {
		tcp_write(csock, SERVER_MSG_FAIL, strlen(SERVER_MSG_FAIL));
		close(csock);
		return;
	}
	subscriber->csock = csock;
	subscriber->ip = ip;
	subscriber->port = port;

	/* Insert to the table */
	int ret = insert_sub(table, topic, subscriber);

	/* Adding to table returns a positive value if it already exists */
	if (ret > 0) {

		/* Free the duplicate */
		free(subscriber);
		subscriber = NULL;

	} else if (ret == ERR) {
		/* On ERR, something went wrong with the table */
		tcp_write(csock, SERVER_MSG_FAIL, strlen(SERVER_MSG_FAIL));
		close(csock);
		return;
	}

	/* On successfully adding new subscriber, send confirmation */
	if (tcp_write(csock, SERVER_MSG_OK, strlen(SERVER_MSG_OK)) != OK) {
		/* If unable to send confirmation, revert since client doesn't know */
		close(csock);
		remove_sub(table, topic, *subscriber);
		if (subscriber) {
			free(subscriber);
		}
	}
}

void unsubscribe(table_t * table, char * topic, int csock, uint32_t ip, uint16_t port)
{
	subscriber_t temp = {
		.next = NULL,
		.prev = NULL,
		.csock = csock,
		.ip = ip,
		.port = port,
	};

	remove_sub(table, topic, temp);

	/* Regardless whether it exists in the table or not, send OK */
	tcp_write(csock, SERVER_MSG_OK, strlen(SERVER_MSG_OK));
	close(csock);
}

void publish(table_t * table, char * topic, int csock)
{	
	/* Get the list of subscribers to send the message to */
	topic_t * temp = get_topic(table, topic);
	if (temp == NULL) {
		close(csock);
		return;
	}
	if (temp->subscriber == NULL) {
		close(csock);
		return;
	}

	/* Read from the publisher */
	char buf[SERVER_BUF_SIZE] = {0};
	size_t ret;
	while ((ret = read(csock, buf, SERVER_BUF_SIZE)) > 0) {

		/* Pass on the message to the subscribers  */
		subscriber_t * subscribers = temp->subscriber;
		while (subscribers != NULL) {
			/* If error during write, remove the unsubscribe */
			if (tcp_write(subscribers->csock, buf, ret) != OK) {
				subscribers = subscribers->next;
				remove_sub(table, topic, *(subscribers->prev));
			} else {
				subscribers = subscribers->next;
			}
		}
	}

	/* Cleanup */
	close(csock);
}
