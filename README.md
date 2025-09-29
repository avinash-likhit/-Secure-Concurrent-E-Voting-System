# -Secure-Concurrent-E-Voting-System
This repository hosts a secure, multi-threaded client-server application designed to simulate an electronic voting process. The project serves as a robust demonstration of Systems Programming, Network Security (SSL/TLS), and Operating System (OS) Concurrency concepts using the C language.



# 🗳️ Secure Concurrent E-Voting System

This project implements a secure client-server application for an electronic voting system.  
The **TCP server** is built using **C** and **Pthreads** for high concurrency and uses **OpenSSL** to secure all communication with **SSL/TLS encryption**.  
It employs a **double-check validation mechanism** to minimize the risk of a single voter casting multiple ballots.

---

## ✨ Features

- 🔒 **SSL/TLS Security**: Encrypts all voter credentials and vote submissions using the OpenSSL library.  
- ⚡ **High Concurrency**: Utilizes Pthreads to handle multiple clients simultaneously.  
- ✅ **Double-Check Validation**: Implements a two-stage check against `voted_list.txt` to mitigate race conditions and prevent double-voting in a highly concurrent environment.  
- 📂 **Simple File-Based Storage**:  
  - Voter eligibility stored in `voters_list.txt`.  
  - Voting results stored in `voted_list.txt`.  

---

## 🛠️ Prerequisites

You need a **Unix-like environment** (Linux/Ubuntu assumed) and the following libraries/tools:

- GNU Compiler Collection (**GCC**)  
- OpenSSL Development Libraries  
- Pthreads Library  
- Bash  

### Install required tools on Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libssl-dev
```




## 🚀 Getting Started

### 1. Setup Data Files

Create the following files in your project directory:

- `voters_list.txt` → List of eligible voters (`Name,ID`)  
- `voted_list.txt` → Start as an empty file; the server appends votes here  

**Example: voters_list.txt**

```txt
Alice,1001
Bob,1002
Veer,125
```


### 2. Generate SSL Certificate and Key

The server requires a **self-signed certificate** (`server.crt`) and **private key** (`server.key`) for the SSL handshake.

```bash
# Generate private key
openssl genrsa -out server.key 2048

# Generate self-signed certificate (valid for 365 days)
openssl req -new -x509 -key server.key -out server.crt -days 365 -nodes -subj "/CN=localhost/O=VotingApp"
```

### 3. Compilation

Compile both the **server** and **client**, linking the required libraries (`-lssl -lcrypto -lpthread`).

```bash
# Compile the server (in separate device or terminal)
gcc ssl_server.c -o ssl_server -lssl -lcrypto -lpthread

# Compile the client (in separate device or terminal)
gcc ssl_client.c -o ssl_client -lssl -lcrypto
```

### 4. Execution

Run the server and client in **separate terminal windows**.

#### Terminal 1 → Start the Server (listens on `127.0.0.1:2001`)

```bash
./ssl_server
```

#### Terminal 2 → Run the Client

```bash
./ssl_client
```

👉 The client will prompt for credentials. Use a Name,ID pair from voters_list.txt.


## 💻 Technical Details

| Component       | Technology         | Description                                                                 |
|------------------|--------------------|-----------------------------------------------------------------------------|
| **Server**      | C, Pthreads,TCP   | Multi-threaded design to manage concurrent voters.                         |
| **Security**    | OpenSSL (SSL/TLS) | Encrypts network traffic over port 2001.                                   |
| **Server Addr** | 127.0.0.1:2001    | Default binding address (can be changed in code for external access).      |
| **Concurrency** | Double-Check Logic| Eligibility check after credentials and final check before vote submission.|



## 📊 File Structure

```
project/
├── ssl_server.c # Server source code
├── ssl_client.c # Client source code
├── voters_list.txt # List of eligible voters (Name,ID)
├── voted_list.txt # Stores votes (generated at runtime)
├── server.key # SSL private key (generated)
└── server.crt # SSL certificate (generated)
```

## 🌐 Running Across a Local Network

To run the client and server on different machines (device-to-device), you must change the hardcoded IP addresses in the source code.

1.  **Find Server IP:** Determine the local IP address of the server machine (e.g., `192.168.1.100`) by running `ip a` in its terminal.
2.  **Server Change (`ssl_server.c`):** Change the bind address from `127.0.0.1` to `0.0.0.0` (to listen globally).
3.  **Client Change (`ssl_client.c`):** Change the connect address from `127.0.0.1` to the **actual Server IP** found in Step 1.

All devices must be on the same local network for the connection to succeed.



---

## 📝 Notes

- Make sure `voters_list.txt` exists before running the server.  
- `voted_list.txt` will be created automatically if not present.  
- For real-world deployment, replace the self-signed certificate with a CA-signed certificate.


