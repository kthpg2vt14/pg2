#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

int sending(void *ptr);
int receiving(void *ptr);

struct data {
    TCPsocket fd;   /* Socket descriptor */
};

int main(int argc, char *argv[])
{
	IPaddress ip;		/* Server address */
	int quit, len;
    SDL_Thread *thread_send, *thread_recv;  /* Prepare threads */
    int         threadReturnValue;
    struct data client_data;

	/* Simple parameter checking */
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s host port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

	/* Resolve the host we are connecting to */
	if (SDLNet_ResolveHost(&ip, argv[1], atoi(argv[2])) < 0)
	{
		fprintf(stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

    /* Create socket */
	if (!(client_data.fd = SDLNet_TCP_Open(&ip)))
	{
		fprintf(stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit(EXIT_FAILURE);
	}

    /* Start threads */
    thread_send = SDL_CreateThread(sending, &client_data);
    thread_recv = SDL_CreateThread(receiving, &client_data);

    /* Wait for threads */
    if (NULL == thread_send) {
        printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
    } else {
        SDL_WaitThread(thread_send, &threadReturnValue);
        printf("\nThread returned value: %d", threadReturnValue);
    }

    if (NULL == thread_recv) {
        printf("\nSDL_CreateThread failed: %s\n", SDL_GetError());
    } else {
        SDL_WaitThread(thread_recv, &threadReturnValue);
        printf("\nThread returned value: %d", threadReturnValue);
    }

    /* Uninitialize */
	SDLNet_TCP_Close(client_data.fd);
	SDLNet_Quit();
	return EXIT_SUCCESS;
}

int sending(void *ptr)
{
    int len;
    char buffer[512], tmp[100];
    struct data* p = (struct data*) ptr;

    for(;;) {
        printf("Write something> ");
        scanf("%s", buffer);

        len = strlen(buffer) + 1;
		if (SDLNet_TCP_Send(p->fd, (void *)buffer, len) < len) {
			fprintf(stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError());
			exit(EXIT_FAILURE);
		}
    }
}

int receiving(void *ptr)
{
    int len;
    char other_buffer[512];
    struct data* p = (struct data*) ptr;

    for(;;) {
        SDLNet_SocketReady(p->fd);
        if (SDLNet_TCP_Recv(p->fd, other_buffer, 512) > 0) {
            printf("Other client: %s\n fd: %u\n", other_buffer, p->fd);
        }
    }
}
