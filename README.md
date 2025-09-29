# -Secure-Concurrent-E-Voting-System
This repository hosts a secure, multi-threaded client-server application designed to simulate an electronic voting process. The project serves as a robust demonstration of Systems Programming, Network Security (SSL/TLS), and Operating System (OS) Concurrency concepts using the C language.



🗳️ Secure Concurrent E-Voting System
This project implements a secure client-server application for an electronic voting system. The server is built using C and Pthreads for high concurrency and uses OpenSSL to secure all communication with SSL/TLS encryption. It employs a double-check validation mechanism to minimize the risk of a single voter casting multiple ballots.

✨ Features
SSL/TLS Security: Encrypts all voter credentials and vote submissions using the OpenSSL library.

High Concurrency: Utilizes Pthreads to handle multiple clients simultaneously.

Double-Check Validation: Implements a two-stage check against voted_list.txt to mitigate race conditions and prevent double-voting in a highly concurrent environment.

Robust Networking: Uses the SO_REUSEADDR socket option for quick server restarts.

Simple File-Based Storage: Stores voter eligibility in voters_list.txt and vote results in voted_list.txt.

🛠️ Prerequisites
To build and run this project, you need a Unix-like environment (Linux/Ubuntu is assumed) and the following libraries:

GNU Compiler Collection (GCC)

OpenSSL Development Libraries

Pthreads Library

Bash

# Install necessary tools and libraries on Ubuntu
sudo apt update
sudo apt install build-essential libssl-dev
🚀 Getting Started
1. Setup Data Files
Create the following files in your project directory:

voters_list.txt: List of eligible voters (Name,ID).

voted_list.txt: Create this file as empty; the server appends votes here.

Example voters_list.txt Content:

Alice,1001
Bob,1002
Veer,125
2. Generate SSL Certificate and Key
The server requires a self-signed certificate (server.crt) and private key (server.key) for the SSL handshake.

Bash

# Generate private key
openssl genrsa -out server.key 2048

# Generate self-signed certificate (valid for 365 days)
openssl req -new -x509 -key server.key -out server.crt -days 365 -nodes -subj "/CN=localhost/O=VotingApp"
3. Compilation
Compile both the server and client, ensuring you link the required libraries (-lssl, -lcrypto, -lpthread).

Bash

# Compile the server
gcc ssl_server.c -o ssl_server -lssl -lcrypto -lpthread

# Compile the client
gcc ssl_client.c -o ssl_client -lssl -lcrypto
4. Execution
You must run the server and client in separate terminal windows.

Terminal 1: Start the Server

The server listens on 127.0.0.1:2001.

Bash

./ssl_server
Terminal 2: Run the Client

The client will prompt for credentials (use a name/ID from voters_list.txt).

Bash

./ssl_client
💻 Technical Details
Component	Technology	Description
Server	C, Pthreads	Multi-threaded design to manage concurrent voters.
Security	OpenSSL (SSL/TLS)	Encrypts network traffic over port 2001.
Server Address	127.0.0.1:2001	Default binding address. Change in code for external network access.
Concurrency Logic	Double-Check	The system performs an eligibility check after credentials and a final check immediately before writing the vote to prevent double-voting.

Export to Sheets
