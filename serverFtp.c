/* 
 * server FTP program
 *
 * Group 6
 * @authors Spencer McAvey, Andy Nguyen, Suban Krishnamoorthy
 * 
 * NOTE: Starting homework #2, add more comments here describing the overall function
 * performed by server ftp program
 * This includes, the list of ftp commands processed by server ftp.
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* Listen port for control connection. */
#define SERVER_FTP_PORT 5432 // Spencer set the server ftp port number.

/* Listen port for data connection. */
#define CLIENT_FTP_PORT 5433 // Andy set the client ftp port number.

/* Error and OK codes */
#define OK 0
#define ER_INVALID_HOST_NAME -1
#define ER_CREATE_SOCKET_FAILED -2
#define ER_BIND_FAILED -3
#define ER_CONNECT_FAILED -4
#define ER_SEND_FAILED -5
#define ER_RECEIVE_FAILED -6
#define ER_ACCEPT_FAILED -7


/* Function prototypes */

int clntConnect(char *serverName, int *s); // Andy copied function from clientftp.c
int svcInitServer(int *s);
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);


/* List of all global variables */

char userCmd[1024];	/* user typed ftp command line received from client */
char cmd[1024];		/* ftp command (without argument) extracted from userCmd */
char argument[1024];	/* argument (without ftp command) extracted from userCmd */
char replyMsg[1024];       /* buffer to send reply message to client */

/* Spencer added these user accounts. */
char *userNames[5] = {"Joe", "Jim", "Mary", "Karen", "Chad"};
char *userPasswords[5] = {"Joe123", "Jim123", "Mary123", "Karen123", "Chad123"};
int matchIndex = 0; // index for matching userNames and userPasswords

/* Andy added these variables, except for filePtr and bytesRead - Send and receive file transfer data */
FILE *filePtr;          /* Used to point to the temporary file that logged command output */
char ftpData[1024];		/* buffer to send/receive file data to/from client */
int  numBytes = 0; 		/* Used to count the total number of bytes transferred during ftp */
int  fileBytesRead = 0; /* The number of bytes read from a file using fread() function */
int  bytesRecv = 0; 	/* The number of bytes received in a single ftp message. */
int  bytesRead = -1; 	/* The number of bytes read by fread() function */
char fileData[1024];    /* Used to store the byte data obtained by fread() function. */

/* Andy added user login status. */
#define USER_LOGGED_IN -1 
#define USER_LOGGED_OUT 0

/*
 * main
 *
 * Function to listen for connection request from client
 * Receive ftp command one at a time from client
 * Process received command
 * Send a reply message to the client after processing the command with staus of
 * performing (completing) the command
 * On receiving QUIT ftp command, send reply to client and then close all sockets
 *
 * Parameters
 * argc		- Count of number of arguments passed to main (input)
 * argv  	- Array of pointer to input parameters to main (input)
 *		   It is not required to pass any parameter to main
 *		   Can use it if needed.
 *
 * Return status
 *	0			- Successful execution until QUIT command from client 
 *	ER_ACCEPT_FAILED	- Accepting client connection request failed
 *	N			- Failed stauts, value of N depends on the command processed
 */

int main( int argc, char *argv[] )
{
	/* List of local varibale */

	int msgSize;        /* Size of msg received in octets (bytes) */
	int dcListenSocket;   /* listening server ftp socket for client connect request */
	int ccSocket;        /* Control connection socket - to be used in all client communication */
	int dcSocket;       /* Data connection socket - to be used in all server communication */
	int status;


	/*
	 * NOTE: without \n at the end of format string in printf,
         * UNIX will buffer (not flush)
	 * output to display and you will not see it on monitor.
	*/
	printf("Started execution of server ftp\n");


	 /*initialize server ftp*/
	printf("Initialize ftp server\n");	/* changed text */

	status=svcInitServer(&dcListenSocket);
	if(status != 0)
	{
		printf("Exiting server ftp due to svcInitServer returned error\n");
		exit(status);
	}


	printf("ftp server is waiting to accept connection\n");

	/* wait until connection request comes from client ftp */
	ccSocket = accept(dcListenSocket, NULL, NULL);

	printf("Came out of accept() function \n");

	if(ccSocket < 0)
	{
		perror("cannot accept connection:");
		printf("Server ftp is terminating after closing listen socket.\n");
		close(dcListenSocket);  /* close listen socket */
		return (ER_ACCEPT_FAILED);  // error exist
	}

	printf("Connected to client, calling receiveMsg to get ftp cmd from client \n");


	/* Receive and process ftp commands from client until quit command.
 	 * On receiving quit command, send reply to client and 
         * then close the control connection socket "ccSocket". 
	 */
	do
	{
	    /* Receive client ftp commands until */
 	    status=receiveMessage(ccSocket, userCmd, sizeof(userCmd), &msgSize);
	    if(status < 0)
	    {
		printf("Receive message failed. Closing control connection \n");
		printf("Server ftp is terminating.\n");
		break;
	    }


/*
 * Starting Homework#2 program to process all ftp commandsmust be added here.
 * See Homework#2 for list of ftp commands to implement.
 */

		/* Spencer implemented command and argument extraction. */

	    /* Separate command and argument from userCmd */
		char temp[1024];
		char *ptr;
		strcpy(temp, userCmd);
	    	ptr = strtok(temp, " "); 

		if(ptr != NULL)
		{
			//printf("ptr: %s...\n", ptr);
			strcpy(cmd, ptr);//copy ftp command into cmd
		}
		else
		{
			strcpy(replyMsg, "150 Not Vaild ftp Command\n");
		}
		
		ptr = strtok(NULL, " "); //get the argument, NULL gets next token from origional string
		if (ptr != NULL)
		{
			strcpy(argument, ptr); //copy ftp argument into argument array
			printf("ftp argument is: %s \n", argument);
 		}
		else
		{
			argument[0] = NULL;
			printf("No argument entered.\n");
		}
		


		/*
		 * Spencer implemented all ftp commands, except send and recv.
		 * Andy added user account privileges to most ftp commands.
		 */

		/* Command to input a username. */  
		if(strcmp(cmd, "user") == 0)
		{
			if(argument[0] == NULL)
			{
				strcpy(replyMsg,"501 Syntax error. No username entered.\n");
			}
			else
			{
				int i;
				for(i = 0; i < 5; i++)
				{
					if(strcmp(userNames[i], argument) == 0)
					{ // found valid user
						matchIndex = i;
						break;
					}
					else
					{
						matchIndex = -2;
					}			
				}	
				if(matchIndex >= 0)
				{
					strcpy(replyMsg, "Enter password >");
					printf("%d\n", matchIndex);
				}
				else
				{
					strcpy(replyMsg, "154 user not found");
				}
			}			
		}

		/* Command to input password to log in to server. */
		else if(strcmp(cmd, "pass") == 0)
		{
			if(matchIndex >= 0)
			{
				if(strcmp(userPasswords[matchIndex], argument) == 0)
				{
					strcpy(replyMsg, "251 Login successful");
					matchIndex = -1;
				}
				else
				{
					strcpy(replyMsg, "156 Incorrect password");
					printf("%s\n", argument);
				}
			}
			else
			{
				strcpy(replyMsg, "Enter valid user name");
			}
		}

		/* Command to quit and exit the server. */
		else if(strcmp(cmd, "quit") == 0)
		{
			printf("Received quit command from the server");
			strcpy(replyMsg, "250 Closing connection\n");
		}

		/* Command for user to create a directory. */
		else if(strcmp(cmd, "mkdir") == 0)
		{
			if(argument[0] == NULL)
			{
					strcpy(replyMsg,"501 Syntax error. No directory entered.\n");
			}
			else if(matchIndex == USER_LOGGED_IN)
			{
				status = system(userCmd);

				if(status == 0)
				{
					//Reply sent to client.
					strcpy(replyMsg, "mkdir success");
				}
				else
				{
					strcpy(replyMsg, "mkdir cmd failed");
				}
			}
			else
			{
				strcpy(replyMsg, "530 User not logged in.\n");
			}
		}

		/* Command for user to delete or remove a directory. */
		else if(strcmp(cmd, "rmdir") == 0)
		{
			if(argument[0] == NULL)
			{
					strcpy(replyMsg,"501 Syntax error. No directory entered.\n");
			}
			else if(matchIndex == USER_LOGGED_IN)
			{
				status = system(userCmd); // Execute the unix command rmdir using system() function.

				if(status == 0)
				{
					// Reply sent to client.
					strcpy(replyMsg, "rmdir success");
				}
				else
				{
					strcpy(replyMsg, "rmdir failed");
				}
			}
			else
			{
				strcpy(replyMsg, "530 User not logged in.\n");
			}
		}

		/* Command for user to change working directory. */
		else if(strcmp(cmd, "cd") == 0)
		{
			if(matchIndex == USER_LOGGED_IN)
			{
				/* Execute cd command using chdir method avoiding
				   creating child process to execute cd command. */
				if(argument[0] == NULL)
				{
					status = chdir(getenv("HOME"));
					strcpy(argument, "HOME directory");
				}
				else
				{
					status = chdir(argument);
				}
				if(status == 0)
				{
					strcpy(replyMsg, "200 cmd OK\n");
				}
				else
				{
					strcpy(replyMsg, "150 cd cmd faild\n");
				}
			}
			else
			{
				strcpy(replyMsg, "530 User not logged in.\n");
			}
		}

		/* Command for user to delete or remove a file. */
		else if(strcmp(cmd, "dele") == 0)
		{
			if(argument[0] == NULL)
			{
					strcpy(replyMsg,"501 Syntax error. Missing filename.\n");
			}
			else if(matchIndex == USER_LOGGED_IN)
			{
				char tempCmd[] = "rm "; // Change dele command to rm Unix system command.
				strcat(tempCmd, argument);
				status = system(tempCmd); // Execute the unix command rm using system() function
				if(status == 0)
				{
					strcpy(replyMsg, "201 success");
				}
				else
				{
					strcpy(replyMsg, "150 dele cmd failed\n");
				}
			}
			else
			{
				strcpy(replyMsg, "530 User not logged in.\n");
			}
		}

		/* Command to display the current working directory. */
		else if(strcmp(cmd, "pwd") == 0)
		{
			if(matchIndex == USER_LOGGED_IN)
			{
				char unixCmd[] = "pwd > pwdoutput";
				//strcpy(unixCmd, "pwd > pwdoutput");
				status = system(unixCmd);
				if(status == 0)
				{
					filePtr = fopen("pwdoutput", "r");
					if(filePtr != NULL)
					{
						bytesRead = fread(replyMsg, 1, sizeof(replyMsg), filePtr);
						if(bytesRead > 0)
						{
							replyMsg[bytesRead] = '\0';
							strcat(replyMsg, "\n 250 command OK \n");
						}
						else
						{
							if(bytesRead == 0)
							{
								strcpy(replyMsg, "\n Error \n");
							}
							if(bytesRead < 0)
							{
								strcpy(replyMsg, "\n Error < 0 bytes read \n");
							}
						}
						fclose(filePtr);
						status = system("rm pwdoutput");
						if(status != 0)
						{
							printf("unable to delete pwdoutput");
						}
					}
				}
				/* Output file could not be created. Notify user of failure */
				else
				{
					strcpy(replyMsg, "150 command failed");
				}
			}
			else
			{
				strcpy(replyMsg, "530 User not logged in.\n");
			}

		}

		/* Command to list files in the current working directory. */
		else if(strcmp(cmd, "ls") == 0)
		{
			if(matchIndex == USER_LOGGED_IN)
			{
				char unixCmd[] = "ls > lsoutput";
				//strcpy(unixCmd, "ls > lsoutput");
				status = system(unixCmd);
				if(status == 0)
				{
					filePtr = fopen("lsoutput", "r");
					if(filePtr != NULL)
					{
						bytesRead = fread(replyMsg, 1, sizeof(replyMsg), filePtr);
						if(bytesRead > 0)
						{
							replyMsg[bytesRead] = '\0';
							strcat(replyMsg, "\n 250 command OK \n");	
						}
						else
						{
							if(bytesRead == 0)
							{
								strcpy(replyMsg, "\n Error \n");
							}
							if(bytesRead < 0)
							{
								strcpy(replyMsg, "\n Error < 0 bytes read \n");
							}
						}
						fclose(filePtr);
						status = system("rm lsoutput");
						if(status != 0)
						{
							printf("unable to delete lsoutput");
						}
					}
				}
				else
				{
					strcpy(replyMsg, "150 command failed");
				}
			}
			else
			{
				strcpy(replyMsg, "530 User not logged in.\n");
			}

			
		}

		/* Command to receive file content from client and write to local file. */
		else if(strcmp(cmd, "send") == 0)
		{
			status=clntConnect("10.3.200.17", &dcSocket); // Accept data connection from server.
			if(status != 0)
			{
				/* Data connection failed. Close data connection socket. */
				strcpy(replyMsg, "425 Cannot establish data connection. Closing data connection socket.\n");
			}
			else
			{
				/* Check if user is logged in and provided a valid filename. */
				if(argument[0] == NULL)
				{
					strcpy(replyMsg, "501 Syntax error. Missing filename.\n");
				}
				else if(matchIndex == USER_LOGGED_IN)
				{
					/* User is logged in. Check if the specified file can be written. */
					filePtr = NULL;
					filePtr = fopen(argument, "w"); // Open local file in write mode.
					if(filePtr == NULL)
					{
						strcpy(replyMsg, "550 File unavailable.");
					}
					else
					{
						/* If the file can be written. Send file until no bytes received or no message can be retrieved. */
						numBytes = 0; // Initialize byte count.
						printf("Client transfering \"%s\" to server.\n", argument);
						do{
							bytesRecv = 0;
							status = receiveMessage(dcSocket, ftpData, sizeof(ftpData), &bytesRecv);
							fwrite(ftpData, 1, bytesRecv, filePtr); // Write from local file.
							numBytes = numBytes + bytesRecv;
						}while(bytesRecv > 0 && status == OK); /* end data connection read loop */
					
						sprintf(replyMsg, "226 File action successful. Received %d", numBytes);
						strcat(replyMsg, " bytes. Closing data connection.\n");
					}
					fclose(filePtr); // Close local file.
				}
				else
				{
					strcpy(replyMsg, "530 User not logged in. Closing data connection.\n");
				}
			}
			close(dcSocket); // Close data connection socket.

		}
		else if(strcmp(cmd, "stat") == 0)//true = 0, false = 1
		{
			strcpy(replyMsg, "250 File transfer mode is ASCII\n");
		}
		else if(strcmp(cmd, "help") == 0)
		{
			printf("client ftp sent help command");
			strcpy(replyMsg, "Input commands: help, quit, user, pass, mkdir, rmdir, dele, send, recv, help, ls, cd, pwd, stat\n");
		}

		/* Command to send server file to client. */
		else if(strcmp(cmd, "recv") == 0)
		{
			status=clntConnect("10.3.200.17", &dcSocket); // Accept data connection request from server.
			if(status != 0)
			{
				/* Data connection failed. Close data connection socket. */
				strcpy(replyMsg, "425 Cannot establish data connection. Closing data connection socket.\n");
			}
			else
			{
				/* Check if user is logged in and provided a valid filename. */
				if(argument[0] == NULL)
				{
					strcpy(replyMsg, "501 Syntax error. Missing filename.\n");
				}
				else if(matchIndex == USER_LOGGED_IN)
				{				
					/* Check if the specified file can be read. */
					filePtr = NULL;
					filePtr = fopen(argument, "r"); // Open server file in read mode.
					if(filePtr == NULL)
					{
						strcpy(replyMsg, "550 File unavailable.");
					}
					else
					{
						/* If the file is readable. Start sending file until end of file. */
						numBytes = 0; // Initialize byte count.
						printf("Sending file \"%s\".\n", argument);
						do{
							fileBytesRead = 0;
							fileBytesRead = fread(ftpData, 1, 100, filePtr); // Read from local file.
							status = sendMessage(dcSocket, ftpData, fileBytesRead); // Send file content to server.
							numBytes = numBytes + fileBytesRead;
						}while(!feof(filePtr) && status == OK); // End do while loop.
						
						sprintf(replyMsg, "226 File action successful. Sent %d", numBytes);
						strcat(replyMsg, " bytes. Closing data connection.");
					}
					fclose(filePtr); // Close local file.
				}
				else
				{
					strcpy(replyMsg, "530 User not logged in. Closing data connection.\n");
				}
			}
			close(dcSocket); // Close data connection socket.
			
		}

		/* Message if user inputs an invalid command. */
		else
		{
			strcpy(replyMsg, "111 Invalid ftp command");

		}

	    /*
 	     * ftp server sends only one reply message to the client for 
	     * each command received in this implementation.
	     */
	    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);	/* Added 1 to include NULL character in */
				/* the reply string strlen does not count NULL character */
	    if(status < 0)
	    {
		break;  /* exit while loop */
	    }
	}
	while(strcmp(cmd, "quit") != 0);

	printf("Closing control connection socket.\n");
	close (ccSocket);  /* Close client control connection socket */

	printf("Closing listen socket.\n");
	close(dcListenSocket);  /*close listen socket */

	printf("Existing from server ftp main \n");

	return (status);
}

/*
 * clntConnect - Andy copied function from clientftp.
 *
 * Function to create a socket, bind local client IP address and port to the socket
 * and connect to the client.
 *
 * Parameters
 * serverName	- IP address of client in dot notation (input)
 * s		    - Control connection socket number (output)
 *
 * Return status
 *	OK			- Successfully connected to the client
 *	ER_INVALID_HOST_NAME	- Invalid client name
 *	ER_CREATE_SOCKET_FAILED	- Cannot create socket
 *	ER_BIND_FAILED		- bind failed
 *	ER_CONNECT_FAILED	- connect failed
 */
int clntConnect (
	char *serverName, /* client IP address in dot notation (input) */
	int  *s 		  /* control connection socket number (output) */
	)
{
	int sock;	/* local variable to keep socket number */

	struct sockaddr_in clientAddress;  	/* local server IP address */
	struct sockaddr_in serverAddress;	/* client IP address */
	struct hostent	   *serverIPstructure;	/* host entry having server IP address in binary */


	/* Get IP address of client in binary from client name (IP in dot notation) */
	if((serverIPstructure = gethostbyname(serverName)) == NULL)
	{
		printf("%s is unknown server. \n", serverName);
		return (ER_INVALID_HOST_NAME);  /* error return */
	}

	/* Create control connection socket */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket ");
		return (ER_CREATE_SOCKET_FAILED);	/* error return */
	}

	/* initialize server address structure memory to zero */
	memset((char *) &clientAddress, 0, sizeof(clientAddress));

	/* Set local server IP address, and port in the address structure */
	clientAddress.sin_family = AF_INET;	/* Internet protocol family */
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY is 0, which means */
						 /* let the system fill server IP address */
	clientAddress.sin_port = 0;  /* With port set to 0, system will allocate a free port */
			  /* from 1024 to (64K -1) */

	/* Associate the socket with local server IP address and port */
	if(bind(sock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}


	/* Initialize serverAddress memory to 0 */
	memset((char *) &serverAddress, 0, sizeof(serverAddress));

	/* Set ftp client ftp address in serverAddress */
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr, 
			serverIPstructure->h_length);
	serverAddress.sin_port = htons(CLIENT_FTP_PORT);

	/* Connect to the client */
	if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("Cannot connect to client ");
		close (sock); 	/* close the control connection socket */
		return(ER_CONNECT_FAILED);  	/* error return */
	}


	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /* successful return */
}  // end of clntConnect() */


/*
 * svcInitServer
 *
 * Function to create a socket and to listen for connection request from client
 *    using the created listen socket.
 *
 * Parameters
 * s		- Socket to listen for connection request (output)
 *
 * Return status
 *	OK			- Successfully created listen socket and listening
 *	ER_CREATE_SOCKET_FAILED	- socket creation failed
 */

int svcInitServer (
	int *s 		/*Listen socket number returned from this function */
	)
{
	int sock;
	struct sockaddr_in svcAddr;
	int qlen;

	/*create a socket endpoint */
	if( (sock=socket(AF_INET, SOCK_STREAM,0)) <0)
	{
		perror("cannot create socket");
		return(ER_CREATE_SOCKET_FAILED);
	}

	/*initialize memory of svcAddr structure to zero. */
	memset((char *)&svcAddr,0, sizeof(svcAddr));

	/* initialize svcAddr to have server IP address and server listen port#. */
	svcAddr.sin_family = AF_INET;
	svcAddr.sin_addr.s_addr=htonl(INADDR_ANY);  /* server IP address */
	svcAddr.sin_port=htons(SERVER_FTP_PORT);    /* server listen port # */

	/* bind (associate) the listen socket number with server IP and port#.
	 * bind is a socket interface function 
	 */
	if(bind(sock,(struct sockaddr *)&svcAddr,sizeof(svcAddr))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}

	/* 
	 * Set listen queue length to 1 outstanding connection request.
	 * This allows 1 outstanding connect request from client to wait
	 * while processing current connection request, which takes time.
	 * It prevents connection request to fail and client to think server is down
	 * when in fact server is running and busy processing connection request.
	 */
	qlen=1; 


	/* 
	 * Listen for connection request to come from client ftp.
	 * This is a non-blocking socket interface function call, 
	 * meaning, server ftp execution does not block by the 'listen' funcgtion call.
	 * Call returns right away so that server can do whatever it wants.
	 * The TCP transport layer will continuously listen for request and
	 * accept it on behalf of server ftp when the connection requests comes.
	 */

	listen(sock,qlen);  /* socket interface function call */

	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /*successful return */
}


/*
 * sendMessage
 *
 * Function to send specified number of octet (bytes) to client ftp
 *
 * Parameters
 * s		- Socket to be used to send msg to client (input)
 * msg  	- Pointer to character arrary containing msg to be sent (input)
 * msgSize	- Number of bytes, including NULL, in the msg to be sent to client (input)
 *
 * Return status
 *	OK		- Msg successfully sent
 *	ER_SEND_FAILED	- Sending msg failed
 */

int sendMessage(
	int    s,	/* socket to be used to send msg to client */
	char   *msg, 	/* buffer having the message data */
	int    msgSize 	/* size of the message/data in bytes */
	)
{
	int i;


	/* Print the message to be sent byte by byte as character */
	for(i=0; i<msgSize; i++)
	{
		printf("%c",msg[i]);
	}
	printf("\n");

	if((send(s, msg, msgSize, 0)) < 0) /* socket interface call to transmit */
	{
		perror("unable to send ");
		return(ER_SEND_FAILED);
	}

	return(OK); /* successful send */
}


/*
 * receiveMessage
 *
 * Function to receive message from client ftp
 *
 * Parameters
 * s		- Socket to be used to receive msg from client (input)
 * buffer  	- Pointer to character arrary to store received msg (input/output)
 * bufferSize	- Maximum size of the array, "buffer" in octent/byte (input)
 *		    This is the maximum number of bytes that will be stored in buffer
 * msgSize	- Actual # of bytes received and stored in buffer in octet/byes (output)
 *
 * Return status
 *	OK			- Msg successfully received
 *	R_RECEIVE_FAILED	- Receiving msg failed
 */


int receiveMessage (
	int s, 		/* socket */
	char *buffer, 	/* buffer to store received msg */
	int bufferSize, /* how large the buffer is in octet */
	int *msgSize 	/* size of the received msg in octet */
	)
{
	int i;

	*msgSize=recv(s,buffer,bufferSize,0); /* socket interface call to receive msg */

	if(*msgSize<0)
	{
		perror("unable to receive");
		return(ER_RECEIVE_FAILED);
	}

	/* Print the received msg byte by byte */
	for(i=0;i<*msgSize;i++)
	{
		printf("%c", buffer[i]);
	}
	printf("\n");

	return (OK);
}
