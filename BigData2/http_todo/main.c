#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/cJSON/cJSON.h"

#define MAX_TODOS 4096
#define MAX_NAME_LEN 256
#define MAX_DESC_LEN 1024

int id_cursor = 0;

typedef struct {
    char method[8];
    char path[256];
    char version[16];
    char* headers;
    char* body;
    size_t body_length;
} Request;

void parse_http_request(char* raw, Request* req) {
    memset(req, 0, sizeof(Request));

	// Parse request line
    sscanf(raw, "%7s %255s %15s", req->method, req->path, req->version);

	// Find headers start
    char* headers_start = strstr(raw, "\r\n");
    if (!headers_start) return;  // malformed request

    headers_start += 2; // skip "\r\n"

    // Find body start
    char* body_start = strstr(headers_start, "\r\n\r\n");
    if (body_start) {
        // Headers block = between headers_start and body_start
        size_t headers_len = body_start - headers_start;
        req->headers = malloc(headers_len + 1);
        memcpy(req->headers, headers_start, headers_len);
        req->headers[headers_len] = 0;

        // Body block
        body_start += 4;  // skip the "\r\n\r\n"
        req->body_length = strlen(body_start);
        req->body = malloc(req->body_length + 1);
        memcpy(req->body, body_start, req->body_length + 1);
    } else {
        // No body
        req->headers = strdup(headers_start);
        req->body = NULL;
        req->body_length = 0;
    }
}

typedef  enum {
	ENDPOINT_UNKNOWN,
    ENDPOINT_POST,
    ENDPOINT_GET,
    ENDPOINT_DELETE,
    ENDPOINT_GETALL,
    ENDPOINT_PUT,
} Endpoint;

int is_number_path(const char *path, int* id) {
    // Must start with '/'
    if (path[0] != '/') return 1;

    // Empty or just '/' is not an ID path
    if (path[1] == '\0') return 1;

    // Parse number after '/'
    const char *p = path + 1;
    while (*p) {
        if (!isdigit(*p)) return 1;   // not numeric → not valid ID
        *id = *id * 10 + (*p - '0');
        p++;
    }

    return 0;
}

Endpoint get_endpoint(const Request* request, int* id) {
    // POST /post
    if (strcmp(request->method, "POST") == 0 &&
        strcmp(request->path, "/post") == 0)
        return ENDPOINT_POST;

    // GET /get/{id}
    if (strcmp(request->method, "GET") == 0 &&
        strncmp(request->path, "/get/", 5) == 0 &&
        is_number_path(request->path + 4, id) == 0)
        return ENDPOINT_GET;

    // DELETE /remove/{id}
    if (strcmp(request->method, "DELETE") == 0 &&
        strncmp(request->path, "/remove/", 8) == 0 &&
        is_number_path(request->path + 7, id) == 0)
        return ENDPOINT_DELETE;

    // GET /getall
    if (strcmp(request->method, "GET") == 0 &&
        strcmp(request->path, "/getall") == 0)
        return ENDPOINT_GETALL;

    // PUT /update/{id}
    if (strcmp(request->method, "PUT") == 0 &&
        strncmp(request->path, "/update/", 8) == 0 &&
        is_number_path(request->path + 7, id) == 0)
        return ENDPOINT_PUT;

    return ENDPOINT_UNKNOWN;
}


typedef struct {
	int id;
	char name[MAX_NAME_LEN];
	bool is_done;
	uint8_t priority;
	char description[MAX_DESC_LEN];
} Todo;

void todo_to_json(const Todo* todo, cJSON* json) {
    cJSON_AddNumberToObject(json, "id", todo->id);
    cJSON_AddStringToObject(json, "name", todo->name);
    cJSON_AddBoolToObject(json, "is_done", todo->is_done);
    cJSON_AddNumberToObject(json, "priority", todo->priority);
    cJSON_AddStringToObject(json, "description", todo->description);
}

int json_to_todo(const char* json_str, Todo* todo) {
    cJSON* root = cJSON_Parse(json_str);
    if (!root) return 1;

	cJSON* id = cJSON_AddNumberToObject(root, "id", id_cursor++);
    cJSON* name = cJSON_GetObjectItem(root, "name");
    cJSON* is_done = cJSON_GetObjectItem(root, "isDone");
    cJSON* priority = cJSON_GetObjectItem(root, "priority");
    cJSON* description = cJSON_GetObjectItem(root, "description");

    if (!cJSON_IsNumber(id) || !cJSON_IsString(name) || !cJSON_IsBool(is_done) ||
        !cJSON_IsNumber(priority) || !cJSON_IsString(description)) {
        cJSON_Delete(root);
        return 1;
    }

    todo->id = id->valueint;
    strncpy(todo->name, name->valuestring, MAX_NAME_LEN - 1);
    todo->name[MAX_NAME_LEN - 1] = '\0'; // ensure null-terminated

    todo->is_done = is_done->type == cJSON_True;
    todo->priority = (uint8_t)priority->valueint;

    strncpy(todo->description, description->valuestring, MAX_DESC_LEN - 1);
    todo->description[MAX_DESC_LEN - 1] = '\0';

    cJSON_Delete(root);
    return 0;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Allow quick restart of the server
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
	int port = 31337;
    server_addr.sin_port = htons(port);

    // Bind to port 31337
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Start listening
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server running on http://0.0.0.0:%i\n", port);

	Todo todos[MAX_TODOS] = {0};
    while (1) {
        // Accept a client
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        // Read HTTP request
        char buffer[2048];
        int bytes = read(client_fd, buffer, sizeof(buffer) - 1);
		Request request = {0};
        if (bytes > 0) {
            buffer[bytes] = 0;

			parse_http_request(buffer, &request);
        }

		int id = 0;
		Endpoint endpoint = get_endpoint(&request, &id);
		cJSON* json = cJSON_CreateObject();
		switch (endpoint) {
			case ENDPOINT_UNKNOWN:
				break;

			case ENDPOINT_POST:
			case ENDPOINT_PUT:
				if (request.body_length < 1) break;

				assert(id_cursor < MAX_TODOS);

				Todo todo = {0};
				if (json_to_todo(request.body, &todo) != 0) {
					printf("Failed to handle POST request!\n");
					exit(1);
				}

				assert(todo.id < MAX_TODOS);
				todos[todo.id] = todo;

				cJSON_AddNumberToObject(json, "id", todo.id);

				break;

			case ENDPOINT_GET:
				assert(id < MAX_TODOS);

				todo_to_json(&todos[id], json);

				break;

			case ENDPOINT_DELETE:
				assert(id < MAX_TODOS);
				memset(&todos[id], 0, sizeof(Todo));

				break;

			case ENDPOINT_GETALL:
				assert(id < MAX_TODOS);

				cJSON_Delete(json);

				json= cJSON_CreateArray();
				if (!json) return -1;

				for (int i = 0; i < MAX_TODOS; i++) {
					cJSON* json_object = cJSON_CreateObject();
					if (!json_object) {
						cJSON_Delete(json);
						return -1;
					}

					todo_to_json(&todos[i], json_object);

					cJSON_AddItemToArray(json, json_object);
				}

				break;

			default:
				printf("Unhandled endpoint, exiting server.\n");
				exit(1);
		}

		const char* json_str = cJSON_PrintUnformatted(json);
		size_t body_len = strlen(json_str);

		// Build HTTP response
		char response[1024*MAX_TODOS];
		snprintf(response, sizeof(response),
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: %zu\r\n"
			"Connection: close\r\n"
			"\r\n"
			"%s",
			body_len, json_str
		);

        // Send response
        write(client_fd, response, strlen(response));

        // Close connection
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
