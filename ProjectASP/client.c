 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080     // Port number for main server
#define mirror_port 7001 // Port number for mirror server



// Method to check valid date is entered or not
int npsd_check_valid_date(char* date) {
    // Check the format of the date YYYY-MM-DD
    int yr, mnt, day;
    if (sscanf(date, "%d-%d-%d", &yr, &mnt, &day) != 3) {
        return 0;
    }
    if (yr < 1 || yr > 9999 || mnt < 1 || mnt > 12 || day < 1 || day > 31) {
        return 0;
    }
    return 1;
}


// Method to check valid filelist
int npsd_check_valid_filelist(char* filelist) {
    // CheckÂ there is at least one file in the file list
    if (strlen(filelist) == 0) {
        return 0;
    }
    return 1;
}


// Method to check valid extension
int check_valid_extensions(char* extensions) {
    // CheckÂ at least one extension is present in the extensions list
    if (strlen(extensions) == 0) {
        return 0;
    }
    return 1;
}

//Main Function
int main(int argc, char const *argv[]) {

   //Intilizing necessay variables
    int npsd_sockfd;
    struct sockaddr_in serv_addr, mirror_addr; 
    char buff[1024] = {0};
    char command[1024] = {0};
    char gf[2048]={0};
    int valid_syntax;

    //SOcket()
    if ((npsd_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error in creating Socket\n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converting addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n Invalid Address / Address  --- not supported \n");
        return -1;
    }

    if (connect(npsd_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection -- Failed \n");
        return -1;
    }

    printf("Connected - to -server.\n");
    printf("Enter command or 'quit' to exit:\n");

    //Infinite Loop
    while (1) {
        valid_syntax = 1;

      //  *memset copies the character c (an unsigned char) to the first n characters of the string pointed to, by the argument str.
        memset(buff, 0, sizeof(buff));
        memset(command, 0, sizeof(command));
        fgets(buff, sizeof(buff), stdin);
        memcpy(gf, buff, sizeof(buff));
        // Remove newline character
        buff[strcspn(buff, "\n")] = 0;

        // Parse command
        char* token = strtok(buff, " ");
        if (token == NULL) {
            valid_syntax = 0;
        } else if (strcmp(token, "filesrch") == 0) { //Function -- with filesrch command
            char* fname = strtok(NULL, " ");
            if (fname == NULL) {
                valid_syntax = 0;
           
                } else {
            sprintf(command, "filesrch %s", fname);
        }
    } else if (strcmp(token, "tarfgetz") == 0) {  //Function -- with tarfgetz command
        char* size1 = strtok(NULL, " ");
        char* size2 = strtok(NULL, " ");
        if (size1 == NULL || size2 == NULL || atoi(size1) < 0 || atoi(size2) < 0) {
            valid_syntax = 0;
             printf("Enter Valid");
        } else {
            char* unzip = strtok(NULL, " ");
            if (unzip != NULL && strcmp(unzip, "-u") == 0) {
                sprintf(command, "tarfgetz %s %s -u", size1, size2);
            } else {
                sprintf(command, "tarfgetz %s %s", size1, size2);
            }
        }
    } else if (strcmp(token, "getdirf") == 0) {  //Function -- with getdirf command
        char* date1 = strtok(NULL, " ");
        char* date2 = strtok(NULL, " ");
        if (date1 == NULL || date2 == NULL || !npsd_check_valid_date(date1) || !npsd_check_valid_date(date2)) {
            valid_syntax = 0;
               printf("Enter Valid ");
        } else {
            char* unzip = strtok(NULL, " ");
            if (unzip != NULL && strcmp(unzip, "-u") == 0) {
                sprintf(command, "getdirf %s %s -u", date1, date2);
            } else {
                sprintf(command, "getdirf %s %s", date1, date2);
            }
        }
    } 
    
    else if (strcmp(token, "fgets") == 0) {  
    
            char* npsd_fl1 = strtok(NULL, " ");
            char* npsd_fl2= strtok(NULL, " ");
            char* npsd_fl3 = strtok(NULL, " ");
            char* npsd_fl4 = strtok(NULL, " ");
            char* npsd_f15 = strtok(NULL, " ");
            if(npsd_fl1 == NULL || npsd_f15 != NULL )
    {
        printf("Please Enter proper extension");
    }
    
    //Function -- with fgets command
       
        gf[strcspn(gf, "\n")]='\0';
        sprintf(command, gf);
        printf("%s", command);
    } 
    
    else if (strcmp(token, "targzf") == 0) {
    char* npsd_exten1 = strtok(NULL, " ");
    char* npsd_exten2 = strtok(NULL, " ");
    char* npsd_exten3 = strtok(NULL, " ");
    char* npsd_exten4 = strtok(NULL, " ");
    if(npsd_exten1 == NULL && npsd_exten2 ==NULL)
    {
        printf("Please Enter proper extension");
    }
    //Function -- with targzf command
        gf[strcspn(gf, "\n")]='\0';
        sprintf(command, gf);
        printf("%s", command);
    } else if (strcmp(token, "quit") == 0) {
        sprintf(command, "quit");
    } else {
        valid_syntax = 0;
    }

    if (!valid_syntax) {
        printf("Invalid syntax. Pleasetry again.\n");
        continue;
    }

    // Send command to server
    send(npsd_sockfd, command, strlen(command), 0);

    // Handle response from server
    //handle_response(sockfd);
    char resp[1024]={0};
    int valueread=read(npsd_sockfd, resp, sizeof(resp));
    printf("%s\n", resp);
    resp[strcspn(resp, "\n")] = '\0';
    if (strcmp(resp, "7001") == 0)
    {
        close(npsd_sockfd);   // closing the current server connection
        printf("In here\n");
        // Creates new socket for the mirror server
        npsd_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (npsd_sockfd == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        memset(&mirror_addr, '\0', sizeof(mirror_addr));
        mirror_addr.sin_family = AF_INET;
        mirror_addr.sin_port = htons(mirror_port);
        mirror_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Connect to the mirror server
        if (connect(npsd_sockfd, (struct sockaddr *)&mirror_addr, sizeof(mirror_addr)) == -1) {
            perror("connect");
            exit(EXIT_FAILURE);
        }
     
    }
    else 
    printf("%s\n", resp);
    if (strcmp(command, "quit") == 0) {
        break;
    }

    printf("Enter a command 'quit' to exit \n");
}

//Close connection
close(npsd_sockfd);
printf("Connection is closed.\n");

return 0;
}
