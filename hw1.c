#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROMPT() {printf("\n> ");fflush(stdout);}
#define DOWN_COMMAND "down"
#define QUIT_COMMAND "quit"

int main() {
    int server_sock = -1;
    char buffer[BUFSIZ];
    struct hostent *server_host;
    struct sockaddr_in server_addr;
    int client_sock;

    printf("Student ID : 20221170\n");
    printf("Name : Sangbin Lee\n");

    for (;;) {
        PROMPT();
        if (!fgets(buffer, BUFSIZ - 1, stdin)) {
            if (ferror(stdin)) {
                perror("stdin");
                exit(1);
            }
            exit(0);
        }

        char *command = strtok(buffer, " \t\n\r");

        if ((command == NULL) || (strcmp(command, "") == 0)) {
            PROMPT();
            continue;
        } else if (strcasecmp(command, QUIT_COMMAND) == 0) {
            exit(0);
        }

        if (!strcasecmp(command, DOWN_COMMAND) == 0) {
            printf("Wrong command %s\n", command);
            PROMPT();
            continue;
        }

        if ((client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
            perror("socket");
            exit(1);
        }

        char *url = strtok(NULL, " \t\n\r");

        char *protocol = strtok(url, ":");
        if(strcmp(protocol, "http") != 0) {
            printf("Only support http, not %s\n", protocol);
            close(client_sock);
            PROMPT();
            continue;
        }
        char *hostname = strtok(NULL, "/");
        char *filename = strtok(NULL, "\n");

        int port = 80; 
        char *port_separator = strstr(hostname, ":");
        if (port_separator != NULL) {
            *port_separator = '\0'; 
            port = atoi(port_separator + 1); 
        }

        if ((server_host = gethostbyname(hostname)) == 0) {
            fprintf(stderr, "%s: unknown host\n", hostname);
            exit(1);
        }

        memset((void *) &server_addr, 0, sizeof(server_addr)); 
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons((u_short)port);
        memcpy((void *) &server_addr.sin_addr, server_host->h_addr, server_host->h_length);

        if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            (void) close(client_sock);
            fprintf(stderr, "connect");
            exit(1);
        }

        char request_message[BUFSIZ];
        snprintf(request_message, BUFSIZ, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-agent: HW1/1.0\r\nConnection: close\r\n\r\n", filename, hostname);
        printf(request_message, BUFSIZ, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-agent: HW1/1.0\r\nConnection: close\r\n\r\n", filename, hostname);
        printf("\n");
        send(client_sock, request_message, strlen(request_message), 0);  
        char *sub_filename = strrchr(filename, '/'); 
        sub_filename = (sub_filename != NULL) ? sub_filename + 1 : filename;

        FILE *file_pointer = fopen(sub_filename, "w");

        char file_name[100];
        strcpy(file_name, sub_filename); 

        char *temp;
        char *header_position;  
        char file_buffer[BUFSIZ]; 
        char *file_size;  
        int response_length = recv(client_sock, buffer, BUFSIZ, 0); 
        int total_size = 0;
        int percentage; 
        int header_size;  
        int byte_size; 

        if (!(buffer[9] == '2' && buffer[10] == '0' && buffer[11] == '0')) {
            char status_code[4];
            strncpy(status_code, &buffer[9], 3);
            status_code[3] = '\0';

            char *status_message = strtok(&buffer[12], "\r\n");

            printf("%s%s\n", status_code, status_message);
            printf("\n");

            fclose(file_pointer);
            close(client_sock);
            continue;
        }


        temp = strstr(buffer, "Content-Length");
        header_position = strstr(temp, "\r\n\r\n");
        header_position += 4; 
        header_size = header_position - buffer; 
        memcpy(file_buffer, &buffer[header_size], response_length - header_size); 

        fwrite(file_buffer, response_length - header_size, 1, file_pointer);
        total_size += response_length - header_size; 
        memset(file_buffer, 0, BUFSIZ);

        file_size = strtok(temp, "\n");
        file_size += 16;  
        byte_size = atoi(file_size);  
        printf("Total Size %d bytes\n", byte_size);
        memset(buffer, 0, BUFSIZ);
        int file_size_var = byte_size;
        percentage = 10;
        while ((response_length = read(client_sock, buffer, BUFSIZ)) > 0) {
            total_size += response_length;
            while (total_size >= file_size_var * percentage / 100) {
                printf("Current Downloading %d / %d (bytes) %d%%\n", (file_size_var * percentage / 100), file_size_var, percentage);
                percentage += 10;

                if (percentage > 100) {
                    printf("Download Complete: %s, %d/%d\n", file_name, file_size_var, file_size_var);
                }
            }
            if (total_size >= byte_size) {
                response_length -= (total_size - byte_size);
                fwrite(buffer, response_length, 1, file_pointer);
                memset(buffer, 0, BUFSIZ);
                break;
            } else {
                fwrite(buffer, response_length, 1, file_pointer);
                memset(buffer, 0, BUFSIZ);
            }
        }

        fclose(file_pointer);
        close(client_sock);
    }
}
