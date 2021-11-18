#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "csapp.h"

#include "debug.h"
#include "server.h"
#include "globals.h"


static void terminate(int);
void handler();
/*
 * "Charla" chat server.
 *
 * Usage: charla <port>
 */


void sighuphandler() {
    terminate(EXIT_SUCCESS);
     }

int main(int argc, char* argv[]){





    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initializations of the client_registry and
    // player_registry.
    char* port = argv[2];
    if(argc != 3 || strcmp(argv[1],"-p")){
        printf("INVALID port");
        exit(EXIT_FAILURE);

    }

    user_registry = ureg_init();
    client_registry = creg_init();

    Signal(SIGHUP,&sighuphandler);
     int serverfd;
    int *clientfd;
    socklen_t clientfd_length = sizeof(struct sockaddr_storage);
   struct sockaddr_storage clientfdaddress;
     pthread_t tid;


    serverfd = Open_listenfd(port);
    if(serverfd <0){
        terminate(EXIT_FAILURE);
    }
    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function charla_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.


    while(1){
        clientfd = Calloc(sizeof(int),sizeof(char));
        *clientfd = Accept(serverfd,(SA*) &clientfdaddress,&clientfd_length);

        if(*clientfd <0){
            free(clientfd);
            clientfd = NULL;
            terminate(EXIT_FAILURE);
        }

        Pthread_create(&tid,NULL,chla_client_service,clientfd);
    }


    fprintf(stderr, "You have to finish implementing main() "
	    "before the server will function.\n");

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}

void handler(){
    terminate(EXIT_SUCCESS);
}

