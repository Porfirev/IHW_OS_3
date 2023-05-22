#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


#define N 2
#define K 6
#define INVITATION_LEN 255

int main(int argc, char *argv[]) {
	srand(NULL);
	char* varirants[K] = {
		"Ресторан",
		"Кофе",
		"Кино",
		"Настольная игра",
		"Прогулка",
		"Поездка",
	};
	
	int sock;                        // Socket descriptor 
    struct sockaddr_in ServAddr; 
    unsigned short port;     
    char *servIP;                    // Server IP address (dotted quad)
    char* name;
    int type;
    
    char response[INVITATION_LEN];     // Buffer for echo string 
    unsigned int requestSize;      // Length of string to echo 
    int bytesRcvd, totalBytesRcvd;   // Bytes read in single recv() and total bytes read 
    
    if (argc != 5)  // Test for correct number of arguments 
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n",
                argv[0]);
        exit(1);
    } 
    servIP = argv[1];             // First arg: server IP address (dotted quad)
    port = atoi(argv[2]); // Use given port, if any
    name = argv[3];
    type = atoi(argv[4]);
    
    // Create a reliable, stream socket using TCP
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        DieWithError("socket() failed");
    }
    
    memset(&ServAddr, 0, sizeof(ServAddr));     // Zero out structure
    ServAddr.sin_family = AF_INET;             // Internet address family
    ServAddr.sin_addr.s_addr = inet_addr(servIP);   // Server IP address
    ServAddr.sin_port = htons(port); // Server port
    
    // Establish the connection to the echo server
    if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
        DieWithError("connect() failed\n");
    }
    
    char* request = varirants[type];

	printf("%s: %s\n", name, request);
    // Send the string to the server
    send(sock, request, INVITATION_LEN, 0);

    if ((bytesRcvd = recv(sock, response, 41, 0)) <= 0) {
        DieWithError("recv() failed or connection closed prematurely\n");
    }
    if (response[0] == '1') {
    	printf("%s: accepted\n", name);
    } else {
    	printf("%s: failed\n", name);
    }
    sleep(50);
    close(sock);
    exit(0);
	return 0;
}
