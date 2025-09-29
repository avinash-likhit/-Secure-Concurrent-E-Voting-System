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
sudo apt install build-essential libssl-dev```





🚀 Getting Started
1. Setup Data Files

Create the following files in your project directory:

voters_list.txt → List of eligible voters (Name,ID)

voted_list.txt → Start as an empty file; the server appends votes here

Example: voters_list.txt

Alice,1001
Bob,1002
Veer,125
