/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/


#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/parse.h"
#include <fcntl.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

char *bad_response="HTTP/1.1 400 Bad Request\r\n\r\n";
                            

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{

    // Retrive the command line arguments
    char* arg1=argv[1];
    char* arg2=argv[2];
    char* arg3=argv[3];
    char* arg4=argv[4];
    char* arg5=argv[5];
    char* arg6=argv[6];
    char* arg7=argv[7];
    char* arg8=argv[8];

    // The port to be used for HTTP requests
    int http_port=atoi(arg1);

    /*
    * TODO: fill the rest of the arguments
    */  

    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(http_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 10))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    // Create the master and read fd set
    fd_set master;    
    fd_set read_fds;  

    // Maximum fd for select()
    int fdmax;       

    // Clear the sets
    FD_ZERO(&master);    
    FD_ZERO(&read_fds);

    // add the listener socket to the master set
    FD_SET(sock, &master);

    // The biggest file descriptor
    fdmax = sock; 

    int i;
    // Run the server in an infinite loop 
    while (1)
    {
        read_fds = master; 
        if(select(fdmax+1,&read_fds,NULL,NULL,NULL)==-1)
        {
            // select() failed. Don't exit the server
            perror("select");
        }
        /*
        * Code logic adapted from Beej's guide of select()
        * https://beej.us/guide/bgnet/examples/selectserver.c 
        */
        for(i=0;i<=fdmax;i++)
        {
            if(FD_ISSET(i,&read_fds))
            {
                if(i==sock)
                {
                    cli_size = sizeof(cli_addr);
                    if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,&cli_size)) == -1)
                    {
                        close(sock);
                        fprintf(stderr, "Error accepting connection.\n");
                        return EXIT_FAILURE;
                    }
                    else
                    {
                        // add the client socket to master set
                        FD_SET(client_sock,&master);
                        if(client_sock > fdmax)
                        {   // update the max socket
                            fdmax = client_sock;
                        }
                    }  
                }
                else
                { //handle data from client
                    readret=0;
                    //fcntl(i, F_SETFL, fcntl(i, F_GETFL,0) | O_NONBLOCK);
                    if((readret = recv(i, buf, BUF_SIZE, 0)) > 0)
                    {
                        Request *request = parse(buf,readret,i);
                        if(request==NULL)
                        {
                            // Bad request received
                            if(send(i, bad_response, strlen(bad_response), 0)==-1)
                            {
                                fprintf(stderr, "Error sending to client.\n");
                            }
                        }
                        else 
                        {
                            // Good request received
                            if (send(i, buf, readret, 0) != readret)
                            {
                                fprintf(stderr, "Error sending to client.\n");
                            }
                            //fcntl(i, F_SETFL, fcntl(i, F_GETFL,0) &  ~O_NONBLOCK);
                        }
                    } 
                    if(readret<=0)
                    {   
                        // Close the client socket and clear the fd set
                        close_socket(i);
                        FD_CLR(i,&master);
                    }  
                }
            }
        }
    }
    close_socket(sock);

    return EXIT_SUCCESS;
}
