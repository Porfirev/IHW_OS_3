#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <pthread.h>

#define N 2
#define INVITATION_LEN 255

void DieWithError(char *errorMessage);  // Error handling function

sem_t *sem;
int already_going = 0;
int was = 0;
typedef struct {
    int valentine_type_num[N];
    int lucky;
} Memory;


void *ClientHandle(void *arg) {
    int socket = *(int *) arg;
    sem_wait(sem);
    char request[INVITATION_LEN];

    /* Receive message from client */
    if ((recv(socket, request, INVITATION_LEN, 0)) < 0)
        DieWithError("recv() failed");

	char res[2];
	res[0] = '0';
    if (already_going == 0) {
    	int test = (rand() % (N - was));
    	if (test == 0) {
    		already_going = 1;
    		res[0] = '1';
    	}
    }
    /* Message response to client */
    send(socket, res, 2, 0);
    sem_post(sem);
    was++;
}


int main(int argc, char *argv[]) {
	srand(NULL);
    int servSock;                    /* Socket descriptor for server */
    int clntSock[N];                    /* Socket descriptor for client */
    struct sockaddr_in ServAddr; /* Local address */
    struct sockaddr_in ClntAddr[N]; /* Client address */
    unsigned short ServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    
    
	pid_t children[N];
    sem_t lsem;
    sem = &lsem;
    sem_init(sem, 0, 1);
    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }
    
    ServPort = atoi(argv[1]);
    
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");
    
    memset(&ServAddr, 0, sizeof(ServAddr));   /* Zero out structure */
    ServAddr.sin_family = AF_INET;                /* Internet address family */
    ServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    ServAddr.sin_port = htons(ServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0)
        DieWithError("bind() failed");

    printf("Server IP address = %s. Wait...\n", inet_ntoa(ServAddr.sin_addr));
    
    
    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, 100) < 0)
        DieWithError("listen() failed");
    clntLen = sizeof(ClntAddr);


	for (int i = 0; i < N; ++i) {
		/* Wait for a client to connect */
		if ((clntSock[i] = accept(servSock, (struct sockaddr *) &(ClntAddr[i]),
		                             &clntLen)) < 0)
		    DieWithError("accept() failed");
		printf("Handling client %s\n", inet_ntoa(ClntAddr[i].sin_addr));
	}
	
	
	pthread_t threads[N];
	int flag = 0;
	for (int i = 0; i < N; ++i) {
		flag += pthread_create(&(threads[i]), NULL,
                                 ClientHandle, (int *) &(clntSock[i]));
	}

	sleep(100);
    if (flag > 0) {
        printf("The threads could not be joined.\n");
        exit(2);
    }
	return 0;
}
