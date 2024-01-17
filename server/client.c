#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    fd_set readfds;
    int max_sd;

    // Crearea socket-ului
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convertirea adresei IPv4 din text în formă binară
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Conectarea la server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(0, &readfds); // 0 este file descriptor pentru stdin
        max_sd = sock;

        // Așteptarea activității pe socket sau stdin
        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // Verifică dacă activitatea este pe socket - mesaj de la server
        if (FD_ISSET(sock, &readfds)) {
            int valread = read(sock, buffer, BUFFER_SIZE);
            if (valread > 0) {
                buffer[valread] = '\0';
                printf("Server: %s\n", buffer);
            }
        }

        // Verifică dacă activitatea este pe stdin - input de la utilizator
        if (FD_ISSET(0, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0; // Elimină newline-ul

            // Închide conexiunea dacă mesajul este "exit"
            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            // Trimite mesajul la server
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
