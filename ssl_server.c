#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_LENGTH 100

// --- MUTEX REMOVED ---

int checkNameAndID(const char *filename, const char *search_name, const char *search_id) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return 0;
    }

    char line[MAX_LENGTH];
    while (fgets(line, MAX_LENGTH, file) != NULL) {
        char *temp_line = strdup(line); 
        if (temp_line == NULL) continue;
        
        char *name = strtok(temp_line, ",");
        char *id_str = strtok(NULL, ",");
        
        if (name != NULL && id_str != NULL) {
            name[strcspn(name, "\n\r")] = '\0';
            id_str[strcspn(id_str, "\n\r")] = '\0';
            
            if (strcmp(name, search_name) == 0 && strcmp(id_str, search_id) == 0) {
                free(temp_line);
                fclose(file);
                return 1;
            }
        }
        free(temp_line);
    }
    fclose(file);
    return 0;
}

void appendToFile(const char *filename, const char *name, const char *id, const char *symbol) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return ;
    }
    fprintf(file, "%s,%s,%s\n", name, id, symbol);
    fclose(file);
}

char* extract_last_string(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    int bat_count = 0;
    int rose_count=0;
    int car_count=0;
    int sco_count=0;
    char line[100];
    char last_string[20];

    while (fgets(line, sizeof(line), file) != NULL) {
        char *last_comma = strrchr(line, ',');
        
        if (last_comma != NULL) {
            strcpy(last_string, last_comma + 1);
            last_string[strcspn(last_string, "\n\r ")] = '\0'; 
            
            if(strcmp("BAT",last_string)==0)
                bat_count++;
            else if(strcmp("ROSE",last_string)==0)
                rose_count++;
            else if(strcmp("CAR",last_string)==0)
                car_count++;
            else if(strcmp("SCORPIO",last_string)==0)
                sco_count++;
        }
    }
    fclose(file);
    char *count_string = malloc(100 * sizeof(char));
    if (count_string == NULL) {
        perror("Memory allocation error");
        return NULL;
    }

    sprintf(count_string, "\nBAT count: %d \nROSE count: %d \nCAR count: %d \nSCORPIO count: %d", bat_count, rose_count, car_count, sco_count);

    return count_string;
}

struct ThreadArgs {
    int client_sock;
    struct sockaddr_in client_addr;
    SSL *ssl;
};

void *processClient(void *args) {
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    int client_sock = threadArgs->client_sock;
    SSL *ssl = threadArgs->ssl;

    char server_message[2000], client_message[2000], NAME[2000], VID[2000];
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Receive Name
    if (SSL_read(ssl, client_message, sizeof(client_message)) <= 0) {
        printf("Couldn't receive Name or connection closed\n");
        goto cleanup;
    }
    strcpy(NAME, client_message);
    NAME[strcspn(NAME, "\n\r")] = '\0'; 
    printf("Msg from client: %s\n", NAME);

    // Receive Voter ID
    memset(client_message, 0, sizeof(client_message));
    if (SSL_read(ssl, client_message, sizeof(client_message)) <= 0) {
        printf("Couldn't receive VID or connection closed\n");
        goto cleanup;
    }
    strcpy(VID, client_message);
    VID[strcspn(VID, "\n\r")] = '\0';
    printf("Msg from client: %s\n", VID);
    memset(client_message, 0, sizeof(client_message));

    // --- INITIAL CHECK ---
    int check1 = checkNameAndID("voters_list.txt", NAME, VID);
    int check2 = checkNameAndID("voted_list.txt", NAME, VID);

    if (check1 == 1 && check2 != 1) {
        printf("Found, proceeding to vote.\n");
        
        // 1. Send Welcome
        strcpy(server_message, "Welcome");
        if (SSL_write(ssl, server_message, strlen(server_message)) <= 0) {
            printf("Can't send Welcome\n");
            goto cleanup;
        }

        // 2. Receive Vote
        if (SSL_read(ssl, client_message, sizeof(client_message)) <= 0) {
            printf("Couldn't receive vote or connection closed\n");
            goto cleanup;
        }

        // --- SECOND CHECK BEFORE RECORDING VOTE (NEW LOGIC) ---
        // This is your requested check to prevent double-voting in this part of the flow.
        int check_again = checkNameAndID("voted_list.txt", NAME, VID);

        if (check_again == 0) {
            
            // 3. Record Vote (Only if check_again is 0)
            client_message[strcspn(client_message, "\n\r ")] = '\0';
            if (strlen(client_message) > 0) {
                appendToFile("voted_list.txt", NAME, VID, client_message);
                printf("Output Done - Vote Recorded: %s\n\n\n\n\n", client_message);
            }

            // 4. Get Results
            char *buff = extract_last_string("voted_list.txt");
            strcpy(server_message, buff);
            free(buff);

            // 5. Send Results
            if (SSL_write(ssl, server_message, strlen(server_message)) <= 0) {
                printf("Can't send results\n");
            }
            
        } else {
            // This case handles the scenario where the initial check passed, but 
            // another thread voted in the interim (the race condition).
            printf("Double-voting detected during second check for %s.\n", NAME);
            memset(server_message, '\0', sizeof(server_message));
            strcpy(server_message, "\nVOTE FAILED: Double voting attempt detected.\n");
            SSL_write(ssl, server_message, strlen(server_message));
        }

    } else {
        // Ineligible Voter (Not registered or already voted initially)
        printf("Not Found, Ineligible Voter. Sending refusal.\n");
        memset(server_message, '\0', sizeof(server_message));
        strcpy(server_message, "\nVOTER NOT ELIGIBLE TO VOTE\n");
        if (SSL_write(ssl, server_message, strlen(server_message)) <= 0) {
            printf("Can't send refusal message\n");
        }
    }

cleanup:
    close(client_sock);
    free(threadArgs);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    pthread_exit(NULL);
}

int main() {
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_server_method());
    if (!ssl_ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_certificate_file(ssl_ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ssl_ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    // Add SO_REUSEADDR
    int opt = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001); 
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        exit(EXIT_FAILURE);
    }
    printf("Done with binding\n");

    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        exit(EXIT_FAILURE);
    }
    printf("\nListening for incoming connections.....\n");

    while (1) {
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);
        if (client_sock < 0) {
            printf("Can't accept\n");
            continue;
        }
        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        SSL *ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, client_sock);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            close(client_sock);
            SSL_free(ssl);
            continue;
        }

        struct ThreadArgs *threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
        if (threadArgs == NULL) {
            perror("Memory allocation error");
            close(client_sock);
            SSL_free(ssl);
            exit(EXIT_FAILURE);
        }
        threadArgs->client_sock = client_sock;
        threadArgs->client_addr = client_addr;
        threadArgs->ssl = ssl;

        pthread_t thread;
        if (pthread_create(&thread, NULL, processClient, (void *)threadArgs) != 0) {
            perror("Error creating thread");
            close(client_sock);
            SSL_free(ssl);
            free(threadArgs);
            continue;
        }

        pthread_detach(thread);
    }

    close(socket_desc);
    SSL_CTX_free(ssl_ctx);
    EVP_cleanup();
    ERR_free_strings();
    return 0;
}
