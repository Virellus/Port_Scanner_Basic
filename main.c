#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>       // For fcntl
#include <sys/select.h>  // For select
#include <errno.h>       // For errno

int main() {
    int sock;
    struct sockaddr_in target;
    char target_ip[16];
    int target_port;
    
    printf("=== Basic Port Scanner ===\n");
    printf("Enter target IP address: ");
    
    // Safer input reading
    if (scanf("%15s", target_ip) != 1) {
        printf("Error reading IP address\n");
        return 1;
    }
    
    printf("Enter port number: ");
    if (scanf("%d", &target_port) != 1) {
        printf("Error reading port number\n");
        return 1;
    }
    
    // Validate port
    if (target_port < 1 || target_port > 65535) {
        printf("Port must be between 1 and 65535\n");
        return 1;
    }
    
    printf("Scanning %s:%d...\n", target_ip, target_port);
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Failed to create socket\n");
        return 1;
    }
    
    // Setup target address
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(target_port);
    
    // Convert IP address
    if (inet_addr(target_ip) == INADDR_NONE) {
        printf("Invalid IP address format\n");
        close(sock);
        return 1;
    }
    target.sin_addr.s_addr = inet_addr(target_ip);
    
    // Try to connect with timeout
    printf("Connecting (timeout: 3 seconds)...\n");
    
    // Set socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    // Attempt connection
    int result = connect(sock, (struct sockaddr*)&target, sizeof(target));
    
    if (result == 0) {
        // Connected immediately
        printf("SUCCESS: Port %d is OPEN!\n", target_port);
    } else if (errno == EINPROGRESS) {
        // Connection in progress, wait with timeout
        fd_set write_fds;
        struct timeval timeout;
        
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);
        timeout.tv_sec = 3;  // 3 second timeout
        timeout.tv_usec = 0;
        
        int select_result = select(sock + 1, NULL, &write_fds, NULL, &timeout);
        
        if (select_result > 0) {
            // Check if connection was successful
            int error;
            socklen_t len = sizeof(error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
            
            if (error == 0) {
                printf("SUCCESS: Port %d is OPEN!\n", target_port);
            } else {
                printf("Port %d is CLOSED or filtered\n", target_port);
            }
        } else if (select_result == 0) {
            printf("Port %d is CLOSED or filtered (timeout)\n", target_port);
        } else {
            printf("Port %d is CLOSED or filtered (error)\n", target_port);
        }
    } else {
        // Connection failed immediately
        printf("Port %d is CLOSED or filtered\n", target_port);
    }
    
    close(sock);
    return 0;
}