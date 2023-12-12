//
// Created by aleksandar on 12.12.23..
//
#include "server.h"


char* url_decode(char *src) {
    size_t src_len = strlen(src);
    char* decoded = malloc(src_len+1);
    size_t decoded_len = 0;

    //decode, get %2x and move it to Hex
    for (size_t i = 0; i < src_len; ++i) {
        if (src[i] == '%' && i + 2 < src_len) {
            int hex_val;
            sscanf(src + i +1, "%2x", &hex_val);
            decoded[decoded_len++] = hex_val;
            i += 2;
        } else {
            decoded[decoded_len++] = src[i];
        }
    }

    //add end of string
    decoded[decoded_len] = '\0';
    return  decoded;
}

char* get_file_ext(char* filename) {
    //pointer to the last . in string
    char *dot = strrchr(filename, '.');
    if(dot == NULL || dot == filename) {
        return "";
    }
    return dot+1;

}

char* get_header_tipe(char* file_ext) {
    if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
        return "text/html";
    }
    if (strcasecmp(file_ext, "txt") == 0) {
        return "text/plain";
    }
    if (strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) {
        return "image/jpeg";
    }
    if (strcasecmp(file_ext, "png") == 0) {
        return "image/png";
    }
    if (strcasecmp(file_ext, "css") == 0) {
    return "text/css;";
    }
    if (strcasecmp(file_ext, "js") == 0) {
        return "application/javascript;";
    }

    return "application/octet-stream";

}

void build_response(char* filename, char* file_ext, char* response, size_t* response_len) {

    if(strlen(filename) == 0) {
        printf("ulazim ovde");
        strcpy(filename, "index.html");
        strcpy(file_ext, "html");
    }
    printf("\n%s \n", filename);
    printf("%s \n", file_ext);
    printf("%s \n", response);

    char* Content_Type = get_header_tipe(file_ext);
    char* header = malloc(BUFFER_SIZE * sizeof(char));

    int file_fd = open(filename, O_RDONLY);
    if(file_fd > 0) {
        snprintf(header, BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "\r\n",
                 Content_Type);
    } else {
        snprintf(response, BUFFER_SIZE,
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "404 Not Found");
        *response_len = strlen(response);
        return;
    }

    //file size stat is sys statistics for file size ...
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    //copy headear to response buff
    *response_len = 0;
    memcpy(response,header, strlen(header));
    *response_len += strlen(header);

    //copy file to response
    ssize_t bytes_read;
    //move pointer of __buff by response_len and subtract BUFFER_SIZE
    while ((bytes_read = read(file_fd,
                            response + *response_len,
                            BUFFER_SIZE - *response_len)) > 0)
        {
        *response_len += bytes_read;
        }
    free(header);
    close(file_fd);

}

void* handleConnection(void* args) {
    int client_fd = *((int *) args);
    char* buffer = malloc(BUFFER_SIZE* sizeof(char));
    char* params_buff = malloc(BUFFER_SIZE * sizeof(char));
    ssize_t bites_recived = recv(client_fd, buffer, BUFFER_SIZE, 0);

    printf("od klijenta primljeno %lu \n", (unsigned long)bites_recived);
    // printf("%s \n", buffer);
    if (bites_recived > 0) {
        regex_t regex;
        regcomp(&regex, "^GET /([^ ?]*)([^ ]*) HTTP/1", REG_EXTENDED);
        regmatch_t matches[3];

        if (regexec(&regex, buffer, 2, matches, 0) == 0) {
            //extract get with path ex. GET /marko
            //set last bit after substring to end of dtring

            buffer[matches[1].rm_eo] = '\0';
            printf("\n%s \n", buffer);
            char *url_filename_encoded = buffer + matches[1].rm_so;

            printf("%s \n", url_filename_encoded);

            //remove %2x from url
            char *filename = url_decode(url_filename_encoded);

            char file_ext[32];
            strcpy(file_ext, get_file_ext(filename));

            printf("%s \n", file_ext);

            char* response = malloc(BUFFER_SIZE *2 *sizeof(char));
            size_t response_len;

            build_response(filename, file_ext, response, &response_len);

            send(client_fd, response, response_len, 0);

            free(response);
            free(filename);
        }
        regfree(&regex);
    }

    close(client_fd);
    free(args);
    free(buffer);
    return NULL;
}

int main(int argc, char *argv[]){
    int server_id;

    //sockaddr_in is arp/inet.h lib
    struct  sockaddr_in server_addr;

    //AF_INET ipv6 address, SOCK_STREAM- two way connection based byte sistem
    server_id = socket(AF_INET, SOCK_STREAM, 0);
    if (server_id < 0) {
        perror("Failed to initialize socket");
        exit(EXIT_FAILURE);
    }

    //config socket with server adr
    //INADDR_ANY - IPv4 local host address.
    //htons() — Translate an unsigned short integer into network byte order
    server_addr.sin_port = htons(PORT);;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //bind socket to a port
    int binded = bind(server_id, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(binded < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if(listen(server_id, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("server podignut osluskuje na %d \n", PORT);

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *clinet_fd = malloc(sizeof(int));
        printf("klijent uspesno povezan \n");
        *clinet_fd = accept(server_id, (struct sockaddr *) &client_addr, &client_addr_len);

        if(*clinet_fd < 0) {
            perror("Accept failure");
            exit(EXIT_FAILURE);
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleConnection, (void *) clinet_fd);
        pthread_detach(thread_id);

    }

}
