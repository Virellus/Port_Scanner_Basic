//headers
#include <stdio.h>      // For printf
#include <stdlib.h>     // For exit
#include <string.h>     // For memset
#include <unistd.h>     // For close
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_addr

#pragma comment(lib, "ws2_32.lib")

int main() {
    /*
    Testing
    printf("Hello, World!\n");
    return 0;

    Key Concepts
    - Variables
    int port = 80;
    char *hostname = "exmaple.com";
    - Pointers
    Variables that store memory adresses instead of values
    int *ptr
    - Structures
    Group related data 
    struct sockaddr_in
    - Functions
    Reusable code blocks
    - Memory Management
    malloc()
    free()

    Projects outline and goals - 
    What it does, is attempts to connect to different ports on a target machine to see whcih services are running
    Core Components:
    Socket Creation, creating a network soccet for communication
    Address Setup, configure a target IP and port
    Connection Attempt, try and connecting to each port
    Result Reporting, displaying whcih ports are opened and closed
    Error Handling, managing conenction failures 
    Learning Goals: 
    Network socket programming 
    System calls
    Error handeling with return codes
    Commnad-line argument parsing
    Basic networking concepts

    **code outline**
    main.c
    -include headers
    -fuction delarations
    -main functio, argument parsing
    -portscanning function
    -helper functions, ip validation, etc
   */   
  // Variables we need
    int sock;                   // Socket file descriptor
    struct sockaddr_in target;   // Structure to hold target address info
    char *target_ip = "8.8.8.8"; // Localhost for testing
    int target_port = 53;        // HTTP port
    
    printf("Checking if port %d is open on %s...\n", target_port, target_ip);
    
    // Step 1: Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Step 2: Set up the target address structure
    memset(&target, 0, sizeof(target));        // Clear the structure
    target.sin_family = AF_INET;               // IPv4
    target.sin_port = htons(target_port);      // Convert port to network byte order
    target.sin_addr.s_addr = inet_addr(target_ip); // Convert IP string to binary
    
    // Step 3: Try to connect
    if (connect(sock, (struct sockaddr*)&target, sizeof(target)) == 0) {
        printf("SUCCESS: Port %d is OPEN!\n", target_port);
    } else {
        printf("Port %d is CLOSED or filtered\n", target_port);
    }
    
    // Step 4: Clean up
    close(sock);
    
    return 0;
}