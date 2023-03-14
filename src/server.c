#include "server.h"

void * run_server(void * args)
{
	table_t * table = ((server_args_t *) args)->table;
	ui_t * ui = ((server_args_t *) args)->ui;
	if (table == NULL || ui == NULL) {
		return NULL;
	}

	/* Detach from main thread */
	if (pthread_detach(pthread_self())) {
		return NULL;
	}

	int sock = tcp_listen();
	if (sock < 0) {
		// TODO: add logging
		fprintf(stderr, "Error : failed to initialize server\n");
		return NULL;
	}

	/* Display socket info (IP & port) */
	fetch_server_info(ui, sock);

	/* Block to accept incoming connections */
	for (;;) {
		handler_args_t hndlr_args = {
			.table = table,
			.ui = ui,
		};

		if (tcp_accept(sock, &hndlr_args.csock, &hndlr_args.ip, &hndlr_args.port) != OK) {
			// TODO: main program needs to know that server has quit
			// TODO: make a field in the UI to display logs from the server
			fprintf(stderr, "Failed to accept new connection");
			return NULL;
		}

		/* Spawn new thread to handle client */
		pthread_t h_thr;
		if (pthread_create(&h_thr, NULL, handle, &hndlr_args) || pthread_detach(h_thr)) {
			// TODO: main program needs to know that server has quit
			// TODO: make a field in the UI to display logs from the server
			fprintf(stderr, "Failed to create handler thread");
			close(hndlr_args.csock);
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
	int csock = ((handler_args_t *) args)->csock;
    uint32_t ip = ((handler_args_t *) args)->ip;
    uint16_t port = ((handler_args_t *) args)->port;
	table_t * table = ((handler_args_t *) args)->table;
	ssize_t ret;

	/* Parse command */
	char cmd[P_CMD_LEN+1];
	if ((ret = read(csock, cmd, P_CMD_LEN)) < 0) {
		perror("read(cmd)");
		close(csock);
		return NULL;
	} else if (ret == 0) {
		return NULL;
	}
	cmd[P_CMD_LEN] = '\0';

	/* Parse topic */
	char topic[P_TOPIC_LEN+1];
	if ((ret = read(csock, topic, P_TOPIC_LEN)) < 0) {
		perror("read(topic)");
		close(csock);
		return NULL;
	} else if (ret == 0) {
		return NULL;
	}
	topic[P_TOPIC_LEN] = '\0';

	/* Handle command */
	if (strncmp(cmd, P_TOPIC_SUBSCRIBE, P_CMD_LEN) == 0) {
		subscribe(table, topic, csock, ip, port);
	} else if (strncmp(cmd, P_TOPIC_UNSUBSCRIBE, P_CMD_LEN) == 0) {
		unsubscribe(table, topic, csock, ip, port);
	} else if (strncmp(cmd, P_TOPIC_PUBLISH, P_CMD_LEN) == 0) {
		publish(table, topic, csock);
	} else {
		/* Unrecognized command */
		tcp_write(csock, SERVER_MSG_FAIL, strlen(SERVER_MSG_FAIL));
		close(csock);
	}
	
	return NULL;
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

		// TODO: use logging tui
		// debug("%d is already subscribed to %s", port, topic);

		/* Free the duplicate */
		free(subscriber);
		subscriber = NULL;

	} else if (ret != OK) {
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
	if (tcp_write(csock, SERVER_MSG_OK, strlen(SERVER_MSG_OK)) != OK) {
		/* If unable to send confirmation, close socket */
		close(csock);
	}	
}

void publish(table_t * table, char * topic, int csock)
{
	// If error sending, remove from subscriber list
}
