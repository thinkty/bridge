#include "helper.hh"

void * handle(void * csock);

int main(int argc, char *argv[])
{
	// Check arguments
	if (argc != 2) {
		std::cout << "usage: bridge <port>" << std::endl;
		return ERR;
	}

	// Listen to port
	int sock = h_listen((unsigned short) atoi(argv[1]));
	if (sock < 0) {
		return ERR;
	}

	while (1) {

		// Allocate memory for the connecting socket
		int * csock = new int;
		if (csock == NULL) {
			std::cerr << "Unable to allocate for connecting socket" << std::endl;
			close(sock);
			return ERR;
		}

		// Accept incoming connections and handle requests
		if (h_accept(sock, csock) < 0) {
			close(sock);
			delete csock;
			return ERR;
		}

		// Handle request in new thread
		pthread_t thread;
		if (pthread_create(&thread, NULL, handle, csock) != 0) {
			perror("Unable to create thread to handle request");
			close(*csock);
			delete csock;
			close(sock);
			return ERR;
		}
		if (pthread_detach(thread) != 0) {
			perror("Unable to detach request handler thread");
			close(*csock);
			delete csock;
			close(sock);
			return ERR;
		}
	}

	// Clean up
	close(sock);
	return 0;	
}

/**
 * Function to run in a thread to handle commands from client
 */
void * handle(void * csock)
{
	// Parse argument
	int sock = *((int *) csock);
	delete (int *)csock;

	// Read for buffer size
	// if bigger than buffersize, too bad :/
	char buff[BUFFERSIZE] = {0};
	int ret = 0;
	while(1) {
		if ((ret = read(sock, buff, BUFFERSIZE)) < 0) {
			std::cerr << "read()" << std::endl;
			close(sock);
			return NULL;
		}

		// EOF
		if (ret == 0) {
			close(sock);
			return NULL;
		}

		buff[ret] = '\0';
		printf("< %s", buff);

		if (write(sock, buff, ret) <= 0) {
			std::cerr << "write()" << std::endl;
			close(sock);
			return NULL;
		}
	}

	close(sock);
	return NULL;
}

