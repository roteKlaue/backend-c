#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mongoc/mongoc.h>
#include "./map.h"
#include "./routes.h"
#include "./mongoc-setup.h"
#include "./util.h"

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "Ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

bool exitMe = false;

#ifdef _WIN32
void initialize_winsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

void cleanup_winsock() {
    WSACleanup();
}
#endif

size_t digitCount(size_t number)
{
    if (number == 0)
        return 1;

    size_t count = 0;
    while (number != 0)
    {
        number /= 10;
        count++;
    }
    return count;
}

void notfound404(int client_socket)
{
    const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 3\r\n"
        "\r\n"
        "404";
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

void internalError500(int client_socket)
{
    const char *response =
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 3\r\n"
        "\r\n"
        "500";
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

void handle_client(int client_socket, HashTable *request_type_map, mongoc_client_t *mongo_client)
{
    char buffer[BUFFER_SIZE];
    int bytes_read;

    bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read < 0) {
        perror("recv");
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\0';

    printf("Received request:\n%s\n", buffer);

    char method[16], path[256], version[16];
    sscanf(buffer, "%s %s %s", method, path, version);
    printf("Requested path: %s\n", path);
    char actual_path[256];
    strcpy_until_char(actual_path, path, '?');

    HashTable *request_path_map = (HashTable *)search(request_type_map, method);
    if (request_path_map == NULL)
    {
        notfound404(client_socket);
        return;
    }

    char *(*path_func)(char *path_arg) = (char *(*)(char *))search(request_path_map, actual_path);
    if (path_func == NULL)
    {
        notfound404(client_socket);
        return;
    }

    const char *page_response = path_func(path);

    size_t page_response_length = strlen(page_response);
    size_t num_digits = digitCount(page_response_length);

    char *str_value = (char *)malloc(sizeof(char) * (num_digits + 1));

    if (str_value == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        internalError500(client_socket);
        return;
    }

    sprintf(str_value, "%zu", page_response_length);
    size_t response_length = strlen("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n") + num_digits + page_response_length + 1;
    char *response = (char *)malloc(response_length);

    if (response == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(str_value);
        internalError500(client_socket);
        return;
    }

    snprintf(response, response_length, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %s\r\n\r\n%s", str_value, page_response);
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

char *exit_the_server(char *path) {
    exitMe = true;
    return "exiting the game";
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    initialize_winsock();
#endif
    mongoc_client_t *client = setup_mongoc();

    HashTable *requestsType = create_table(5);
    HashTable *getRequests = create_table(10);
    HashTable *putRequests = create_table(10);
    HashTable *patchRequests =  create_table(10);
    HashTable *postRequests = create_table(10);
    HashTable *deleteRequests = create_table(10);

    insert(requestsType, "GET", getRequests);
    insert(requestsType, "PUT", putRequests);
    insert(requestsType, "POST", postRequests);
    insert(requestsType, "PATCH", patchRequests);
    insert(requestsType, "DELETE", deleteRequests);

    insert(getRequests, "/", &my_index);
    insert(getRequests, "/exit", &exit_the_server);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (exitMe == false)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("accept");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        handle_client(client_socket, requestsType, client);
    }

    free_table(getRequests);
    free_table(deleteRequests);
    free_table(patchRequests);
    free_table(postRequests);
    free_table(putRequests);
    free_table(requestsType);
    close(server_socket);
#ifdef _WIN32
    cleanup_winsock();
#endif
    return 0;
}
