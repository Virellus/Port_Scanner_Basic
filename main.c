#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

// Function declarations
void show_menu();
void scan_single_port();
void scan_port_range();
void scan_common_ports();
int test_port(char* ip, int port);
const char* get_service_name(int port);

int main() {
    int choice;
    
    printf("=== Advanced Port Scanner ===\n");
    
    while (1) {  // Main menu loop
        show_menu();
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue;
        }
        
        switch (choice) {
            case 1:
                scan_single_port();
                break;
            case 2:
                scan_port_range();
                break;
            case 3:
                scan_common_ports();
                break;
            case 4:
                printf("Coming soon: Network discovery!\n");
                break;
            case 5:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice! Please select 1-5.\n");
        }
        
        printf("\nPress Enter to continue...");
        while (getchar() != '\n');  // Clear buffer
        getchar();  // Wait for Enter
    }
    
    return 0;
}

void show_menu() {
    printf("\n" "================================\n");
    printf("        PORT SCANNER MENU\n");
    printf("================================\n");
    printf("1. Scan single port\n");
    printf("2. Scan port range\n");
    printf("3. Scan common ports\n");
    printf("4. Network discovery\n");
    printf("5. Exit\n");
    printf("================================\n");
}

void scan_single_port() {
    char target_ip[16];
    int target_port;
    
    printf("\n--- Single Port Scan ---\n");
    printf("Enter target IP address: ");
    
    if (scanf("%15s", target_ip) != 1) {
        printf("Error reading IP address\n");
        return;
    }
    
    printf("Enter port number: ");
    if (scanf("%d", &target_port) != 1) {
        printf("Error reading port number\n");
        return;
    }
    
    // Validate input
    if (target_port < 1 || target_port > 65535) {
        printf("Port must be between 1 and 65535\n");
        return;
    }
    
    if (inet_addr(target_ip) == INADDR_NONE) {
        printf("Invalid IP address format\n");
        return;
    }
    
    printf("\nScanning %s:%d...\n", target_ip, target_port);
    
    int result = test_port(target_ip, target_port);
    if (result == 1) {
        printf("SUCCESS: Port %d is OPEN!\n", target_port);
    } else {
        printf("Port %d is CLOSED or filtered\n", target_port);
    }
}

void scan_port_range() {
    char target_ip[16];
    int start_port, end_port;
    int open_ports = 0;
    
    printf("\n--- Port Range Scan ---\n");
    printf("Enter target IP address: ");
    
    if (scanf("%15s", target_ip) != 1) {
        printf("Error reading IP address\n");
        return;
    }
    
    printf("Enter starting port: ");
    if (scanf("%d", &start_port) != 1) {
        printf("Error reading start port\n");
        return;
    }
    
    printf("Enter ending port: ");
    if (scanf("%d", &end_port) != 1) {
        printf("Error reading end port\n");
        return;
    }
    
    // Validate input
    if (start_port < 1 || end_port > 65535 || start_port > end_port) {
        printf("Invalid port range! Ports must be 1-65535 and start <= end\n");
        return;
    }
    
    if (inet_addr(target_ip) == INADDR_NONE) {
        printf("Invalid IP address format\n");
        return;
    }
    
    printf("\nScanning %s ports %d-%d...\n", target_ip, start_port, end_port);
    printf("Port\tStatus\n");
    printf("----\t------\n");
    
    // Loop through each port in the range
    for (int current_port = start_port; current_port <= end_port; current_port++) {
        int result = test_port(target_ip, current_port);
        
        if (result == 1) {
            printf("%d\tOPEN\n", current_port);
            open_ports++;
        } else {
            printf("%d\tCLOSED\n", current_port);
        }
    }
    
    // Summary
    printf("\nScan complete!\n");
    printf("Scanned %d ports, found %d open\n", (end_port - start_port + 1), open_ports);
}

void scan_common_ports() {
    char target_ip[16];
    int open_ports = 0;
    
    // Most important/common ports to scan
    int common_ports[] = {
        21,    // FTP
        22,    // SSH
        23,    // Telnet
        25,    // SMTP
        53,    // DNS
        80,    // HTTP
        110,   // POP3
        143,   // IMAP
        443,   // HTTPS
        993,   // IMAPS
        995,   // POP3S
        1433,  // MS SQL
        3306,  // MySQL
        3389,  // RDP
        5432,  // PostgreSQL
        5900,  // VNC
        6379,  // Redis
        8080,  // HTTP Alt
        8443,  // HTTPS Alt
        9090   // Various services
    };
    
    int num_ports = sizeof(common_ports) / sizeof(common_ports[0]);
    
    printf("\n--- Common Ports Scan ---\n");
    printf("Enter target IP address: ");
    
    if (scanf("%15s", target_ip) != 1) {
        printf("Error reading IP address\n");
        return;
    }
    
    if (inet_addr(target_ip) == INADDR_NONE) {
        printf("Invalid IP address format\n");
        return;
    }
    
    printf("\nScanning %s for common ports...\n", target_ip);
    printf("Scanning %d important ports...\n\n", num_ports);
    printf("Port\tStatus\tService\n");
    printf("----\t------\t-------\n");
    
    // Scan each common port
    for (int i = 0; i < num_ports; i++) {
        int port = common_ports[i];
        int result = test_port(target_ip, port);
        
        if (result == 1) {
            printf("%d\tOPEN\t%s\n", port, get_service_name(port));
            open_ports++;
        } else {
            printf("%d\tCLOSED\t%s\n", port, get_service_name(port));
        }
    }
    
    // Summary
    printf("\nCommon ports scan complete!\n");
    printf("Scanned %d ports, found %d open\n", num_ports, open_ports);
}

int test_port(char* ip, int port) {
    int sock;
    struct sockaddr_in target;
    
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 0;  // Failed to create socket
    }
    
    // Setup target address
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr(ip);
    
    // Set socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    // Attempt connection
    int result = connect(sock, (struct sockaddr*)&target, sizeof(target));
    int is_open = 0;
    
    if (result == 0) {
        // Connected immediately
        is_open = 1;
    } else if (errno == EINPROGRESS) {
        // Connection in progress, wait with timeout
        fd_set write_fds;
        struct timeval timeout;
        
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;
        
        int select_result = select(sock + 1, NULL, &write_fds, NULL, &timeout);
        
        if (select_result > 0) {
            // Check if connection was successful
            int error;
            socklen_t len = sizeof(error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len);
            
            if (error == 0) {
                is_open = 1;
            }
        }
    }
    
    close(sock);
    return is_open;  // 1 = open, 0 = closed
}

const char* get_service_name(int port) {
    switch (port) {
        case 21: return "FTP";
        case 22: return "SSH";
        case 23: return "Telnet";
        case 25: return "SMTP";
        case 53: return "DNS";
        case 80: return "HTTP";
        case 110: return "POP3";
        case 143: return "IMAP";
        case 443: return "HTTPS";
        case 993: return "IMAPS";
        case 995: return "POP3S";
        case 1433: return "MS SQL";
        case 3306: return "MySQL";
        case 3389: return "RDP";
        case 5432: return "PostgreSQL";
        case 5900: return "VNC";
        case 6379: return "Redis";
        case 8080: return "HTTP-Alt";
        case 8443: return "HTTPS-Alt";
        case 9090: return "Various";
        default: return "Unknown";
    }
}