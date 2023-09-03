#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>



#define PORT 7001 //Port number 
#define BUFSIZE 1024 //buffer


// Function to search for a file in directories
void npsd_explore_directory(char* npsd_targetPath, char* npsd_flname, char* npsd_commd) 
{
    DIR* dir;                      // Directory structure
    struct dirent* entry;          // Directory entry structure
    struct stat st;                // File status structure

    if ((dir = opendir(npsd_targetPath)) == NULL) {  // Open the directory
        printf("Not able to open the directory %s\n", npsd_targetPath);
        return;
    }

    while ((entry = readdir(dir)) != NULL)   // Traverse the directory tree
    {  
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)  // Skip "." and ".." directories
        {
            continue;
        }

        char npsd_entire_path[1024];   // Constructing complete path to the file or directory
        snprintf(npsd_entire_path, sizeof(npsd_entire_path), "%s/%s", npsd_targetPath, entry->d_name);

        if (lstat(npsd_entire_path, &st) == 0)  // Checking whether entry is a file or a directory
        {
            if (S_ISDIR(st.st_mode)) 
            {
                npsd_explore_directory(npsd_entire_path, npsd_flname, npsd_commd);  // Recurse into the subdirectory
            } 
            else if (S_ISREG(st.st_mode)) 
            {
                if (strcmp(entry->d_name, npsd_flname) == 0) // Checking whether the file has the desired name
                {
                    time_t c_time = st.st_ctime;
                    sprintf(npsd_commd, "%s, %ld, %s\n", npsd_entire_path, st.st_size, ctime(&c_time));
                    closedir(dir);
                    return;
                }
            }
        } 
        else 
        {
            printf("Couldn't get information about this file %s\n", npsd_entire_path);
        }
    }

    closedir(dir);  // Closing the directory
}




// Function to process client requests
void processclient(int sockfd) 
{
    char npsd_buf[1024] = {0};         // Receive buffer
    char npsd_commd[1024] = {0};      // Command buffer
    char npsd_tf[1024] = {0};           // Temporary buffer

    while (1) 
    {
        memset(npsd_buf, 0, sizeof(npsd_buf));
        memset(npsd_commd, 0, sizeof(npsd_commd));
        memcpy(npsd_tf, npsd_buf, sizeof(npsd_buf));

        int npsd_rvalue = read(sockfd, npsd_buf, sizeof(npsd_buf));
        npsd_buf[npsd_rvalue] = '\0';

        char* npsd_token = strtok(npsd_buf, " ");   // Parse command
        if (npsd_token == NULL) 
        {
            sprintf(npsd_commd, "Entered syntax is not valid. Please Enter again.\n");
        } 
        else if (strcmp(npsd_token, "filesrch") == 0)  // Execution of filesrch command
        { 
            char* npsd_flname = strtok(NULL, " ");
            if (npsd_flname == NULL) {
                sprintf(npsd_commd, "Entered syntax is not valid. Please Enter again.\n");
            } 
            else 
            {
                char npsd_primary_path[1024];
                sprintf(npsd_primary_path, "/home/");

                char npsd_commd_buf[BUFSIZE]; // Search file in the root directory
                sprintf(npsd_commd_buf, "find %s -maxdepth 1 -name %s -printf \"%%s,%%Tc\n\"",
                        npsd_primary_path, npsd_flname);
                FILE* fp = popen(npsd_commd_buf, "r");
                char line[BUFSIZE];
                if (fgets(line, BUFSIZE, fp) != NULL) 
                {
                    sprintf(npsd_commd, "%s, %s", npsd_flname, line); // File found in root directory
                } else 
                {
                    sprintf(npsd_commd_buf, "find %s -name %s -printf \"%%s,%%Tc\n\"", npsd_primary_path, npsd_flname); // Searching for file in all subdirectories
                    fp = popen(npsd_commd_buf, "r");
                    if (fgets(line, BUFSIZE, fp) != NULL) 
                    {
                        sprintf(npsd_commd, "%s, %s", npsd_flname, line);   // File found in subdirectory
                    } else 
                    {
                        sprintf(npsd_commd, "Unable to find the file\n");  // prints File not found
                    }
                }
                pclose(fp);
            }
        } 
        // Execution of tarfgetz command

        //tarfgetz command
        else if (strcmp(npsd_token, "tarfgetz") == 0)  
        { 
            char* npsd_s1_str = strtok(NULL, " ");
            char* npsd_s2_str = strtok(NULL, " ");
            char* npsd_unzip_status = strtok(NULL, " ");

            if (npsd_s1_str == NULL || npsd_s2_str == NULL) 
            {
                sprintf(npsd_commd, "Entered syntax is not valid. Please Enter again.\n");
            } else 
            {
                int size1 = atoi(npsd_s1_str);
                int size2 = atoi(npsd_s2_str);
                if (size1 < 0 || size2 < 0 || size1 > size2) 
                {
                    sprintf(npsd_commd, "Please enter valid size range.\n");
                } else {
                    char npsd_primary_path[1024];
                    sprintf(npsd_primary_path, "/home/");

                    char npsd_commd_buf[BUFSIZE]; // Finding files matching the size range
                    sprintf(npsd_commd_buf, "find %s -type f -size +%dk -size -%dk -print0 | xargs -0 tar -czf temp.tar.gz",
                            npsd_primary_path, size1, size2);
                    system(npsd_commd_buf);

                    if (npsd_unzip_status != NULL && strcmp(npsd_unzip_status, "-u") == 0)  // Check whether the unzip flag is provided
                    {     
                        printf("Unzipping due to uzip flag.\n");
                        system("tar -xzf temp.tar.gz -C /home/shreyanshdalwadi/ProjectASP"); // Unzip the file in the current directory
                         printf("Unzipping DOne\n");
                        sprintf(npsd_commd, "Files are retrieved and has been successfully unzipped.\n");
                    } 
                    else 
                    {
                        sprintf(npsd_commd, "Files are retrieved successfully. Now use -u flag to unzip the files.\n");
                    }
                }
            }
        } 
        // Execution of getdirf command

        //getdirf command
        else if (strcmp(npsd_token, "getdirf") == 0) 
        {  
            char* npsd_d1_str = strtok(NULL, " ");
            char* npsd_d2_str = strtok(NULL, " ");
            char* npsd_unzip_status = strtok(NULL, " ");

            if (npsd_d1_str == NULL || npsd_d2_str == NULL) {
                sprintf(npsd_commd, "Entered syntax is not valid. Please Enter again.\n");
            } 
            else 
            {
                char npsd_primary_path[1024];
                sprintf(npsd_primary_path, "/home/");

                char npsd_commd_buf[BUFSIZE]; // Finding files that match the date range
                sprintf(npsd_commd_buf, "find %s -type f -newermt \"%s\" ! -newermt \"%s\" -print0 | xargs -0 tar -czf temp.tar.gz",
                        npsd_primary_path, npsd_d1_str, npsd_d2_str);
                system(npsd_commd_buf);

                if (npsd_unzip_status != NULL && strcmp(npsd_unzip_status, "-u") == 0)  // Checking if the unzip flag is provided
                {
                      printf("Unzipping due to uzip flag.\n");
                    system("tar -xzf temp.tar.gz -C /home/shreyanshdalwadi/ProjectASP"); // Unzip the file in the current directory
                      printf("Unzipping DOne\n");
                    sprintf(npsd_commd, "Files are retrieved and has been successfully unzipped.\n");
                } else {
                    sprintf(npsd_commd, "Files are retrieved successfully. Now use -u flag to unzip the files.\n");
                }
            }
        } 
        else if (strcmp(npsd_token, "fgets") == 0) // Execution of fgets command
        { 
            char* npsd_fl1 = strtok(NULL, " ");
            char* npsd_fl2 = strtok(NULL, " ");
            char* npsd_fl3 = strtok(NULL, " ");
            char* npsd_fl4 = strtok(NULL, " ");
          

            char npsd_primary_path[1024];
            sprintf(npsd_primary_path, "/home/");

            char npsd_commd_buf[BUFSIZE]; // Check whether any of the specified files are present
            sprintf(npsd_commd_buf, "find %s -type f \\( -iname \"%s\" -o -iname \"%s\" -o -iname \"%s\" -o -iname \"%s\" -o -iname \"%s\" -o -iname \"%s\" \\) -print0 | xargs -0 tar -czf temp.tar.gz",
                    npsd_primary_path, npsd_fl1, npsd_fl2, npsd_fl3, npsd_fl4);
            sprintf(npsd_commd, npsd_commd_buf);
            sprintf(npsd_commd, "%s", npsd_commd_buf);
            int npsd_status = system(npsd_commd_buf);
            // Checking whether the files were found
            if (npsd_status == 0) 
            {
          
               sprintf(npsd_commd, "Files are retrieved successfully. %s %s %s", npsd_fl2, npsd_commd_buf);
                
            } 
            else 
            {
                sprintf(npsd_commd, "File-- not found.\n");
            }
        } 


         // Execution of targzf command
        else if (strcmp(npsd_token, "targzf") == 0)
        { 
            char* npsd_exten1 = strtok(NULL, " ");
            char* npsd_exten2 = strtok(NULL, " ");
            char* npsd_exten3 = strtok(NULL, " ");
            char* npsd_exten4 = strtok(NULL, " ");
            char* npsd_unzip_status = strtok(NULL, " ");
            int uzip = 0;
            if (strcmp(npsd_exten1, "-u") == 0 || strcmp(npsd_exten2, "-u") == 0 || strcmp(npsd_exten3, "-u") == 0 || strcmp(npsd_exten4, "-u") == 0)
                uzip = 1;
            char npsd_primary_path[1024];
            sprintf(npsd_primary_path, "/home/");

            char npsd_commd_buf[BUFSIZE];  // Checking whether any of the specified files are present
            sprintf(npsd_commd_buf, "find %s -type f \\( ", npsd_primary_path);
            if (npsd_exten1 != NULL) sprintf(npsd_commd_buf + strlen(npsd_commd_buf), "-iname \"*.%s\" -o ", npsd_exten1);
            if (npsd_exten2 != NULL) sprintf(npsd_commd_buf + strlen(npsd_commd_buf), "-iname \"*.%s\" -o ", npsd_exten2);
            if (npsd_exten3 != NULL) sprintf(npsd_commd_buf + strlen(npsd_commd_buf), "-iname \"*.%s\" -o ", npsd_exten3);
            if (npsd_exten4 != NULL) sprintf(npsd_commd_buf + strlen(npsd_commd_buf), "-iname \"*.%s\" -o ", npsd_exten4);
            sprintf(npsd_commd_buf + strlen(npsd_commd_buf), "-false \\) -print0 | xargs -0 tar -czf temp.tar.gz");

            int npsd_status = system(npsd_commd_buf);

            if (npsd_status == 0)  // Check if the files were found
            {
                if (strcmp(npsd_tf, "-u") == 0)  // Check whether the unzip flag is provided
                {
                    system("tar -xzf temp.tar.gz -C /home/shreyanshdalwadi"); // Unzip the file in the current directory

                    sprintf(npsd_commd, "Files are retrieved and has been successfully unzipped.\n %s %s", npsd_exten2, npsd_commd_buf);
                } else {
                    if(uzip==1)  
                {
                printf("Unzipping due to uzip flag.\n");
               // system("/home/shreyanshdalwadi/ProjectASP -xzf temp.tar.gz -C /home/shreyanshdalwadi");
               system("tar -xzf temp.tar.gz -C /home/shreyanshdalwadi/ProjectASP > extraction_log.txt 2>&1");
                printf("Unzipping DOne\n");
               // system("tar -xzf temp.tar.gz -C /home/shreyanshdalwadi");
                  break;
                }
                else
                        sprintf(npsd_commd, "Files are retrieved successfully. Now use -u flag to unzip the files.\n");
                }
            } 
            else 
            {
                sprintf(npsd_commd, "No file found.\n");
            }
        } else if (strcmp(npsd_token, "quit") == 0) 
        {
            sprintf(npsd_commd, "Bye!!!.\n");
            break;
        } 
        else 
        {
            sprintf(npsd_commd, "Entered syntax is not valid. Please Enter again.\n");
        }
        send(sockfd, npsd_commd, strlen(npsd_commd), 0); // Send response to client
    }

    close(sockfd);
    exit(0);
}

//MAin method for mirror server
int main(int argc, char const *argv[]) {
    int npsd_server_fd, npsd_new_soc;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creatinf socket file descriptor
    if ((npsd_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port 8080
    if (setsockopt(npsd_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //BIND()
    if (bind(npsd_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //LISTEN()
    if (listen(npsd_server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //INFINTE loop
    while (1) {
        if ((npsd_new_soc = accept(npsd_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("The New client is connected --->. Forking child process...\n");
         printf("Mirror Server");

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {  // Child process
            close(npsd_server_fd);
            processclient(npsd_new_soc);
        } else {  // Parent process
            close(npsd_new_soc);
            while (waitpid(-1, NULL, WNOHANG) > 0);  // Clean up zombie processes
        }
    }

    return 0;
}
