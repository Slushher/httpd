#include "../headers/socket.h"

#define bufferSize 1024

int serverSocket;
int clientSocket;
socklen_t clientlen;

FILE *stream;
short listenPort;
int optval;
bool is_static;
struct hostent *hostp; /* client host info */
struct sockaddr_in server_address;
struct sockaddr_in client_address;
std::string host_address;

char buffer[bufferSize];
char method[bufferSize];
char uri[bufferSize];
char version[bufferSize];
char filename[bufferSize];
char filetype[bufferSize];
char cgiargs[bufferSize];

char *p;
int fd;
struct stat sbuf;

bool setupSocket(unsigned short port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    
    //bind port to socket
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);
    if (bind(serverSocket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0){
        std::cout<<"ERROR on binding\n";
        return false;
    }

    if (listen(serverSocket, 5) < 0)
    {
        std::cout<<"ERROR on listen\n";
        return false;
    }
    clientlen = sizeof(client_address);
    do{
        clientSocket = accept(serverSocket, (struct sockaddr *) &client_address, &clientlen);
        if (clientSocket < 0)
        {
            std::cout<<"ERROR on accept";
            return false;
        }
        hostp = gethostbyaddr((const char *)&client_address.sin_addr.s_addr, sizeof(client_address.sin_addr.s_addr), AF_INET);
        host_address = inet_ntoa(client_address.sin_addr);
        if (host_address == "")
        {
            std::cout<<"Could not resolve host address\n";
            return false;
        }
        std::cout<<"Incoming connection on "<<host_address<<"\n";
        if ((stream = fdopen(clientSocket, "r+")) == NULL)
        {
            std::cout<<"ERROR on fdopen\n";
            return false;
        }
        fgets(buffer, sizeof(buffer), stream);
        printf("%s", buffer);
        sscanf(buffer, "%s %s %s\n", method, uri, version);

        if (strcasecmp(method, "GET")) {
            std::cout << stream << method << "501" << "Not Implemented\n";
            fclose(stream);
            close(clientSocket);
            continue;
        }

        fgets(buffer, bufferSize, stream);
        printf("%s", buffer);
        while(strcmp(buffer, "\r\n")) {
            fgets(buffer, bufferSize, stream);
            printf("%s", buffer);
        }

        setenv("QUERY_STRING", cgiargs, 1); 
        sprintf(buffer, "HTTP/1.1 200 OK\n");
        write(clientSocket, buffer, strlen(buffer));
        sprintf(buffer, "Server: Tiny Web Server\n");
        write(clientSocket, buffer, strlen(buffer));

        if (!strstr(uri, "cgi-bin")) { /* static content */
            is_static = true;
            strcpy(cgiargs, "");
            strcpy(filename, ".");
            strcat(filename, uri);
            if (uri[strlen(uri)-1] == '/')
                strcat(filename, "index.html");
        }
        else { /* dynamic content */
            is_static = 0;
            p = index(uri, '?');
            if (p) {
                strcpy(cgiargs, p+1);
                *p = '\0';
            }
            else {
                strcpy(cgiargs, "");
            }
            strcpy(filename, ".");
            strcat(filename, uri);
        }

        if (stat(filename, &sbuf) < 0) {
            std::cout << stream << filename << "404" << "Not found";
            fclose(stream);
            close(clientSocket);
            continue;
        }

        if (is_static) {
            if (strstr(filename, ".html"))
                strcpy(filetype, "text/html");
            else if (strstr(filename, ".gif"))
                strcpy(filetype, "image/gif");
            else if (strstr(filename, ".jpg"))
                strcpy(filetype, "image/jpg");
            else
                strcpy(filetype, "text/plain");
            
            /* print response header */
            fprintf(stream, "HTTP/1.1 200 OK\n");
            fprintf(stream, "Server: Tiny Web Server\n");
            fprintf(stream, "Content-length: %d\n", (int)sbuf.st_size);
            fprintf(stream, "Content-type: %s\n", filetype);
            fprintf(stream, "\r\n");
            fflush(stream);
            
            /* Use mmap to return arbitrary-sized response body */
            fd = open(filename, O_RDONLY);
            p = mmap(0, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
            fwrite(p, 1, sbuf.st_size, stream);
            munmap(p, sbuf.st_size);
            close(fd);
        }
    }
    while(true);
    //close(stream);
    close(clientSocket);
    return true;
}