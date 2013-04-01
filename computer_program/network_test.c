void networkListen(char *buffer)
{
	int list_s;		//Listening socket
	int conn_s;		//connection socket
	short int port = 3357;	//port number
	struct sockaddr_in servaddr;	//socket address struct
	printf("Starting socket");
	if((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		printf("Error making listening socket\n");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
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

	while ( 1 )
	{
		printf("Accepting on socket\n");
		if((conn_s = accept(list_s, NULL, NULL)) < 0)
		{
			printf("Error calling accept\n");
			exit(EXIT_FAILURE);
		}
		printf("Connection made\n");
		if(read(conn_s, buffer, 255) < 0)
		{
			printf("Error reading\n");
			exit(EXIT_FAILURE);
		}
		printf("Received: %s\n", buffer);

		if(write(conn_s, buffer, 12) < 0)
		{
			printf("Error writing\n");
			exit(EXIT_FAILURE);
		}

		printf("Closing the connection\n");
		if(close(conn_s) < 0)
		{
			printf("Error closing\n");
			exit(EXIT_FAILURE);
		}
	}
}
