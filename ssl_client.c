#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h> // Include for close()

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (!ssl_ctx) {
        printf("Error creating SSL context\n");
        exit(EXIT_FAILURE);
    }

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    // Confirmed port and IP match server (127.0.0.1:2001)
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2001);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Unable to connect\n");
        exit(EXIT_FAILURE);
    }
    printf("Connected with server successfully\n");

    SSL *ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        printf("Error creating SSL structure\n");
        close(socket_desc);
        SSL_CTX_free(ssl_ctx);
        exit(EXIT_FAILURE);
    }

    if (SSL_set_fd(ssl, socket_desc) <= 0) {
        printf("Error attaching SSL to socket\n");
        SSL_free(ssl);
        close(socket_desc);
        SSL_CTX_free(ssl_ctx);
        exit(EXIT_FAILURE);
    }

    if (SSL_connect(ssl) <= 0) {
        printf("SSL handshake error\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(socket_desc);
        SSL_CTX_free(ssl_ctx);
        exit(EXIT_FAILURE);
    }

    // send name
    printf("Enter Name (with exact punctuation): ");
    fgets(client_message, sizeof(client_message), stdin);

    // Rectification: Remove trailing newline
    client_message[strcspn(client_message, "\n")] = 0; 

    // Send the name (sending null terminator for robust server reading)
    if (SSL_write(ssl, client_message, strlen(client_message) + 1) < 0) {
        printf("Unable to send Name\n");
        exit(EXIT_FAILURE);
    }

    // send VOTER ID
    printf("Enter VOTER ID: ");
    fgets(client_message, sizeof(client_message), stdin);

    // Rectification: Remove trailing newline
    client_message[strcspn(client_message, "\n")] = 0;

    // Send the Voter ID (sending null terminator)
    if (SSL_write(ssl, client_message, strlen(client_message) + 1) < 0) {
        printf("Unable to send Voter ID\n");
        exit(EXIT_FAILURE);
    }

    // Receive the server's authentication response:
    memset(server_message, 0, sizeof(server_message));
    if (SSL_read(ssl, server_message, sizeof(server_message)) < 0) {
        printf("Error while receiving server's authentication msg\n");
        exit(EXIT_FAILURE);
    }
    printf("\n\n\nServer's response: %s\n", server_message);

    // Check for "Welcome"
    if (strstr(server_message, "Welcome") != NULL) {

        printf("\nThe Candidates of each party are:\n\n");
        printf("Pappu Singh :: (Symbol: BAT)\n");
        printf("Nagendra Maddy :: (Symbol: ROSE)\n");
        printf("Karthik :: (Symbol: CAR)\n");
        printf("Avinash :: (Symbol: SCORPIO)\n\n\n");
        int m;
        char *message;
    l1:
        printf("Enter 1 to vote for BAT, \nEnter 2 to vote for ROSE ,\nEnter 3 to vote for CAR ,\nEnter 4 to vote for SCORPIO\n\n");
        
        if (scanf("%d", &m) != 1) {
            // Handle non-integer input gracefully
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input type. Please enter a number.\n");
            goto l1;
        }
        
        // Clear input buffer after successful scanf (before next fgets)
        while (getchar() != '\n'); 

        switch (m) {
            case 1:
                message = "BAT";
                break;
            case 2:
                message = "ROSE";
                break;
            case 3:
                message = "CAR";
                break;
            case 4:
                message = "SCORPIO";
                break;
            default:
                printf("invalid number\n");
                goto l1;
        }

        printf("\n\nCasting vote for: %s\n", message);
        memset(client_message, 0, sizeof(client_message));
        strcpy(client_message, message);

        // Send the vote
        if (SSL_write(ssl, client_message, strlen(client_message) + 1) < 0) {
            printf("Unable to send vote\n");
            exit(EXIT_FAILURE);
        }

        printf("\n\n");
        printf("\nCurrent results:\n\n");

        // Receive the vote results
        memset(server_message, 0, sizeof(server_message));
        if (SSL_read(ssl, server_message, sizeof(server_message)) < 0) {
            printf("Error while receiving results\n");
            exit(EXIT_FAILURE);
        }
        printf("\n\nServer's response: %s\n", server_message);

        printf("\n\n \tThank you for your visit. Happy Voting\n");

    } else {
        printf("\n\nWe couldn't find your information or vote under this id is already casted. \nPlease re-check your credentials at Indian Voter Service Portal(IVSP), by visiting www.ivsp.gov.in \n\n");
        printf("\n\n \tThank you for your visit. Happy Voting\n");
    }

    // Close SSL connection and free SSL context
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);

    // Close socket
    close(socket_desc);

    return 0;
}
