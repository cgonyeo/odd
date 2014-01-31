#include "odd.h"

#define ANIMATION(animationName, num) {\
    .name=#animationName,\
    .function=animationName,\
    .numParams=num \
},
Animation animation_list[] = {
#include "animations.def"
    NULL
};
#undef ANIMATION

void *networkListen(char *buffer)
{
    int list_s;        //Listening socket
    int conn_s;        //connection socket
    short int port = INCPORT;    //port number
    struct sockaddr_in servaddr;    //socket address struct
    char* tok;        //To split off tokens from input
    printf("Starting socket\n");
    if((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("Error making listening socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_port = htons(port);

    int yes = 1;

    if(setsockopt(list_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        printf("Error setting socket options\n");
        exit(EXIT_FAILURE);
    }

    printf("Binding to socket\n");
    if(bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        printf("Error calling bind\n");
        exit(EXIT_FAILURE);
    }
    printf("Setting socket to listen\n");
    if(listen(list_s, LISTENQ) < 0)
    {
        printf("Error calling listen\n");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("Accepting on socket\n");
        if((conn_s = accept(list_s, NULL, NULL)) < 0)
        {
            printf("Error calling accept\n");
            exit(EXIT_FAILURE);
        }

        printf("Connection made\n");
        pthread_t handler;
        int *num = malloc(sizeof(int));
        memcpy(num, &conn_s, sizeof(int));
        pthread_create(&handler,NULL,handleConnection,num);
    }
}

void *handleConnection(void *num)
{
    char buffer[256];
    int conn_s = *((int *)num);
    free(num);
    int conn_status = read(conn_s, &buffer, 255);
    if(conn_status <= 0)
        perror("Connection");
    while(conn_status > 0)
    {
        printf("Received %d: %s\n", conn_status, buffer);
        conn_status = read(conn_s, &buffer, 255);
        if(conn_status <= 0)
            perror("Connection");
    }
    return NULL;
}
