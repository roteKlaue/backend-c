#include "webserver.h"

#ifdef _WIN32
void initialize_winsock()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

void cleanup_winsock()
{
    WSACleanup();
}
#endif

const char *JSON_TYPE = "application/json";
const char *TEXT_TYPE = "text/plain";
const char *HTML_TYPE = "text/html";
const char *XML_TYPE = "application/xml";
const char *CSS_TYPE = "text/css";
const char *JS_TYPE = "application/javascript";

const char* ContentType_to_string(enum ContentType content_type)
{
    switch (content_type) {
        case TEXT:
            return TEXT_TYPE;
        case JSON:
            return JSON_TYPE;
        case JS:
            return JS_TYPE;
        case HTML:
            return HTML_TYPE;
        case XML:
            return XML_TYPE;
        case CSS:
            return CSS_TYPE;
        default:
            return "unknown";
    }
}

size_t digit_count(size_t number)
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

void handle_client(int client_socket, int buffer_size, route *(*path_func)(char *method, char *actual_path, char *path, HashTable *params), void (*not_found_404)(int client_socket), void (*internal_error_500)(int client_socket))
{
    char *buffer = (char *)malloc(sizeof(char) * buffer_size);
    int bytes_read;

    bytes_read = recv(client_socket, buffer, buffer_size - 1, 0);
    if (bytes_read < 0)
    {
        perror("recv");
        close(client_socket);
        free(buffer);
        return;
    }
    buffer[bytes_read] = '\0';

    char method[16], path[256], version[16];
    sscanf(buffer, "%s %s %s", method, path, version);
    char actual_path[256];
    strcpy_until_char(actual_path, path, '?');

    HashTable *params = create_table(10);
    parse_url_params(params, path);

    route *page_response = path_func(method, actual_path, path, params);

    if (page_response == NULL)
    {
        free(buffer);
        not_found_404(client_socket);
        return;
    }

    char *content_type = ContentType_to_string(page_response->content_type); 
    size_t page_response_length = strlen(page_response->content);
    size_t content_type_length = strlen(content_type);
    size_t num_digits = digit_count(page_response_length);

    char *str_value = (char *)malloc(sizeof(char) * (num_digits + 1));

    if (str_value == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(buffer);
        internal_error_500(client_socket);
        return;
    }

    sprintf(str_value, "%zu", page_response_length);
    size_t response_length = strlen("HTTP/1.1 200 OK\r\nContent-Type: \r\nContent-Length: \r\n\r\n") + content_type_length + num_digits + page_response_length + 1;
    char *response = (char *)malloc(response_length);

    if (response == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(str_value);
        free(buffer);
        internal_error_500(client_socket);
        return;
    }

    snprintf(response, response_length, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %s\r\n\r\n%s", content_type, str_value, page_response->content);
    send(client_socket, response, (int)strlen(response), 0);
    close(client_socket);

    if(page_response->clean_up)
    {
        free(page_response->content);
    }

    free_route(page_response);
    free_table(params);
    free(response);
    free(buffer);
    free(str_value);
}

int setup_webserver(int port, int buffer_size, route *(*path_func)(char *method, char *actual_path, char *path, HashTable *params), void (*not_found_404)(int client_socket), void (*internal_error_500)(int client_socket), bool (*stop_server)())
{
#ifdef _WIN32
    initialize_winsock();
#endif
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = (int) socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("listen");
        close(server_socket);
        return -1;
    }

    printf("Server is listening on port %d\n", port);

    while (stop_server())
    {
        client_socket = (int) accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("accept");
            close(server_socket);
            return -1;
        }

        handle_client(client_socket, buffer_size, path_func, not_found_404, internal_error_500);
    }

    return server_socket;
}

void clean_up_webserver(int server_socket) {
    close(server_socket);
#ifdef _WIN32
    cleanup_winsock();
#endif
}

route *create_route(char *content, bool clean_up, enum ContentType content_type)
{
    route *router = (route *)malloc(sizeof(route));
    router->clean_up = clean_up;
    router->content_type = content_type;
    router->content = content;
    return router;
}

void free_route(route *route)
{
    free(route);
}
