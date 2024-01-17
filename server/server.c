#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 30


int initialize_server(int *server_fd, struct sockaddr_in *address) {
    int opt = 1;
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(PORT);

    if (bind(*server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(*server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int accept_new_connection(int server_fd, int *client_socket) {
    int new_socket, addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in address;

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("New connection: socket fd is %d, ip is: %s, port: %d\n", 
           new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
  
    // Adaugă noul socket în array-ul de socket-uri
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[i] == 0) {
            client_socket[i] = new_socket;
            printf("Adding to list of sockets at index %d\n", i);
            break;
        }
    }

    return new_socket;
}

void handle_activity(int *client_socket, fd_set *readfds) {
    char buffer[1024];
    int valread, sd;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        sd = client_socket[i];

        if (FD_ISSET(sd, readfds)) {
            valread = read(sd, buffer, 1024);
            if (valread == 0) {
                getpeername(sd, (struct sockaddr*)NULL, NULL);
                printf("Host disconnected, socket fd is %d\n", sd);
                close(sd);
                client_socket[i] = 0;
            } else {
                buffer[valread] = '\0';
                printf("Message from client %d: %s\n", sd, buffer);

                // Broadcast mesajul la toți clienții, cu excepția celui care a trimis
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (client_socket[j] != 0 && client_socket[j] != sd) {
                        send(client_socket[j], buffer, strlen(buffer), 0);
                    }
                }
            }
        }
    }
}


int main() {
	int server_fd, client_socket[MAX_CLIENTS] = {0}, max_sd, activity;
	struct sockaddr_in address;
	fd_set readfds;

	// Inițializarea serverului
	initialize_server(&server_fd, &address);

	// Bucla principală
	while (1) {
		FD_ZERO(&readfds);
		FD_SET(server_fd, &readfds);
		max_sd = server_fd;

		// Adăugarea socket-urilor client în set
		for (int i = 0; i < MAX_CLIENTS; i++) {
			int sd = client_socket[i];

			if (sd > 0)
				FD_SET(sd, &readfds);

			if (sd > max_sd)
				max_sd = sd;
		}

		// Așteptarea activității pe oricare dintre socket-uri
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR)) {
			printf("select error");
		}

		// Dacă ceva s-a întâmplat pe socket-ul serverului, este o conexiune în așteptare
		if (FD_ISSET(server_fd, &readfds)) {
			accept_new_connection(server_fd, client_socket);
		}

		// Altfel, este o activitate IO pe unul din socket-urile client
		handle_activity(client_socket, &readfds);
	}

	return 0;
}
