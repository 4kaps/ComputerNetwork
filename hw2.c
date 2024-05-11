// 20221170 sangbin lee

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

#define BUFSIZE 1024

void send_responseMsg(char *msg, int client_socket, struct sockaddr_in remote) {
    char filename[256];
    int sent = 0, filesize = 0;
    sscanf(msg, "GET /%s", filename);

    struct stat st;
    if (stat(filename, &st) == 0) {
        filesize = st.st_size;
    }else {
    char errorResponseMsg[] = "HTTP/1.0 404 NOT FOUND\r\nConnection: close\r\nContent-Length: 31\r\nContent-Type: text/plain\r\n\r\n";
    send(client_socket, errorResponseMsg, strlen(errorResponseMsg), 0);
    printf("Server Error: No such file ./%s!\n", filename);
    return; 
}
    printf("Connection : Host IP %s, Port %d, socket %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), client_socket);
    printf("GET /%s HTTP/1.1\n", filename);

int headers_count = 0;
char *header = strtok(msg, "\r\n");
while (header != NULL) {
    headers_count++;
    if (strstr(header, "User-Agent: ") != NULL) {
        char *user_agent = strstr(header, "(");
        if (user_agent != NULL) {
            *user_agent = '\0'; 
        }
        printf("%s\n", header); 
    }
    header = strtok(NULL, "\r\n");
}

    printf("%d headers\n", headers_count);

    if (filesize > 0) {
        char responseMsg[BUFSIZE];
        if(filesize > 100000){
            snprintf(responseMsg, BUFSIZE, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Length: %d\r\nContent-Type: image/jpeg\r\n\r\n", filesize);
        }else{
            snprintf(responseMsg, BUFSIZE, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n", filesize);
        }
        send(client_socket, responseMsg, strlen(responseMsg), 0);

        FILE *file = fopen(filename, "rb");
        if (file) {
            char buffer[BUFSIZE];
            while ((sent = fread(buffer, 1, BUFSIZE, file)) > 0) {
                send(client_socket, buffer, sent, 0);
            }
            fclose(file);
        }
    } else {
        char responseMsg[] = "HTTP/1.0 404 NOT FOUND\r\nConnection: close\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n";
        send(client_socket, responseMsg, strlen(responseMsg), 0);
    }

    printf("finish %d %d\n", filesize, filesize);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server, remote;
    int request_sock, new_sock;
    int bytesread;
    socklen_t addrlen;
    char buf[BUFSIZ];

    if (argc != 2) {
        fprintf(stderr,"usage: %s portnum \n", argv[0]);
        exit(1);
    }

    int portnum = atoi(argv[1]);

    if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }

    printf("Student ID : 20221170\n");
    printf("Name : Sangbin Lee\n");

    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portnum);

    if (bind(request_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(request_sock, 5);

    while(1) {
        addrlen = sizeof(remote);
        new_sock = accept(request_sock, (struct sockaddr *)&remote, &addrlen);
        if (new_sock < 0) {
            perror("accept");
            exit(1);
        }

        memset(buf, 0, BUFSIZE);
        bytesread = recv(new_sock, buf, BUFSIZE, 0);
        if (bytesread > 0) {
            //printf("%s\n", buf);
            send_responseMsg(buf, new_sock, remote);
        }

        close(new_sock);
    }

    close(request_sock);

    return 0;
}