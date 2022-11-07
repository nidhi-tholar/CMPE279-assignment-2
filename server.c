// SERVER CODE
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>

#define PORT 8081

int dropPrivilegeToNobodyUser() {
    struct passwd* password;
    pid_t idOfChildProcess, pid;
    idOfChildProcess = fork();
    printf("The child Process ID is : %d\n",idOfChildProcess);
    printf("The parent UID is : %d \n",getuid());

    if(idOfChildProcess == 0) {

        // this means that the Fork is successful
        printf("\n the process of fork is successful \n");

        // set privilege as nobody user for read-write
        password = getpwnam("nobody");

        pid = setuid(password->pw_uid);
        printf("The child UID is : %d \n",getuid());
        if(pid==0){

            // set privilege as successful
            return 1;
        }

        // set privilege as unsuccessful
        return 0;
    }
}
int main(int argc, char const *argv[])
{
    int server_fileDescriptor;
    int message_read;
    int socket_new;
    struct sockaddr_in address;
    int opt = 1;
    int address_length = sizeof(address);
    char buffer_message[1024] = {0};
    char *hello_message = "Message from server: Hello";

    // checking if the running process is same as child 
    if(strcmp(argv[0], "socket") == 0)
    {
        printf("\nThe runnning process is in child.");
        int new_socket = atoi(argv[1]);
        message_read = read(new_socket, buffer_message, 1024);
        printf("\n%s", buffer_message);
        send(new_socket, hello_message, strlen(hello_message), 0);
        printf("\nHello message has been successfully sent to the client.");
        exit(0);
    }

    // Show ASLR
    printf("execve=0x%p\n", execve);

    // Creating socket file descriptor
    if ((server_fileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        // if socket fails
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Here, we attach socket to port 80
    if (setsockopt(server_fileDescriptor, SOL_SOCKET, SO_REUSEADDR ,
                                                  &opt, sizeof(opt)))
    {
        // handle failure
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcibly attach socket to port 80
    if (bind(server_fileDescriptor, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        // handle failure
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fileDescriptor, 3) < 0)
    {
        // handle failure
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((socket_new = accept(server_fileDescriptor, (struct sockaddr *)&address,
                       (socklen_t*)&address_length))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    if (dropPrivilegeToNobodyUser()) {
        
        // process the message
        message_read = read(socket_new, buffer_message, 1024);
        printf("Number of bytes read is %d for the message: %s\n", message_read, buffer_message);
        send(socket_new, hello_message, strlen(hello_message), 0);
        printf("Hello message has been successfully sent\n");

        // duplicating the socket
        int duplicate_socket = dup(socket_new);
        if(duplicate_socket == -1)
        {
          perror("Error: Socket duplication has failed!");
        }
        
        // converting the duplicate socket to a string
        char string_buffer[10];
        sprintf(string_buffer, "%d", duplicate_socket);
        char *args[] = {"socket", string_buffer, NULL};
        execvp(args[0], args);
    }
    wait(NULL);
    return 0;
}
