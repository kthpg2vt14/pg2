/*
 * simple tcp server
 */

#include "includes.h"

#define PATH
#define TRUE 1
#define FALSE 0
#define EVER ;;
#define NUMCLIENT 6

pthread_mutex_t mutex[NUMCLIENT] = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t thread_key;

struct socket {
    int client_sockets[NUMCLIENT];
    int num_connections;
};

void *packageDirector(void *);
void player_positions(void);
void *get_in_addr(struct sockaddr *);

int main(void)
{
    int tsfd, usfd, done = 0, status, data, i, connections = 0;
    struct sockaddr_in my_addr, their_addr; /*sockaddr_storage*/
    struct addrinfo hints, *res;
    socklen_t addr_size;
    char str[512], tmp[512];
    struct socket sockets;
    pthread_t thread[NUMCLIENT];

    /// memset() first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    /// getaddrinfo() set ip and port to listen for incomming connections on
    if ((status = getaddrinfo(NULL, "2002", &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    /// socket() make a socket
    if((tsfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("socket");
        exit(1);
    }

    /// bind() to the port we passed in to getaddrinfo():
    if((bind(tsfd, res->ai_addr, res->ai_addrlen)) == -1) {
        perror("bind");
        exit(1);
    }

    /// wait for incomming connections
    if (listen(tsfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    connections = 0;
for(;;) {
        int done, n;
        printf("Waiting for a connection...\n");

        /// accept() an incoming connection:
        addr_size = sizeof their_addr;

        if ((sockets.client_sockets[connections] = accept(tsfd, (struct sockaddr *)&their_addr, (socklen_t *)&my_addr)) == -1) {
            perror("accept");
            exit(1);
        }
        else {
            connections++;
            sockets.num_connections = connections; // informing threads of number of connected clients
            pthread_create(&thread[connections-1], NULL, &packageDirector, &sockets);
            printf("Client %d connected. num_connections: %d\n", connections,sockets.num_connections);  // DEBUG
        }

        /// TODO: join()
//        for (i = 0; i < connections; i++)
//            done = pthread_join(thread[i], NULL);


//        close(tcfd);
    }
    return 0;
}

void *packageDirector(void *incoming)
{
    char tmp[512];
    int i = 0, flag = 0;
    struct socket* p = (struct socket*) incoming;    /// POINTING TO THE STRUCT
    int id = ((p->num_connections) - 1);

    for(EVER) {

            if (recv(p->client_sockets[id], tmp, 512, 0) < 0)
            {
                perror("recv");
                exit(1);
            }
            else
                printf("Recv:Client %d socket: %d\n", i, p->client_sockets[i]);

            printf("id %d got: %s \n", id, tmp);    // DEBUG


        /// sending back data
        for (i = 0; i < p->num_connections; i++) {
            if (send(p->client_sockets[i], tmp, sizeof(tmp), 0) < 0) {
                perror("send");
                exit(1);
            }
            else    // DEBUG
                printf("Send:Client %d socket: %d num_connections: %d\n", i, p->client_sockets[i], p->num_connections);
        }
    }
}

// resolve client adress
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
