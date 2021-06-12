#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAXDATASIZE 1024
#define file 0x80000000
#define directory 0x40000000

struct dataInfo {
    uint32_t metadata;
    uint32_t pathLength;
    uint32_t fileSize;
    uint32_t hashNum;
    char fileName[1024];
    char fileBuffer[1024];
};
int CalculateHash(int fd){
 
    struct stat fileStat;
    fstat(fd, &fileStat);
    int hash = fileStat.st_size / MAXDATASIZE;
    return hash* 23478;
}
int file_exists (char *filename) {
  struct stat   fileStat;   
  return (stat (filename, &fileStat) == 0);
}

int biteSized(int size_of_file, int iteration){
    //for(int i = 0; i < size_of_file / MAXDATASIZE; i++){
    //    recv(sockfd);
    //}
    int maxIterations = size_of_file / MAXDATASIZE;
    if(iteration <= maxIterations){
        return size_of_file - maxIterations*MAXDATASIZE;
    }
    else{
        return 0;
    }
    
}
int main(int argc, char *argv[]) {
    int sockfd, newfd,recieve, filefd, iteration = 0, maxIteration, numbytes;
    struct dataInfo* dataInfo;
    socklen_t length;
    struct sockaddr_in saddr, caddr;
    struct dataInfo fileToGet;
    char fileName[256];
    char buffer[MAXDATASIZE];
    uint32_t metadata, pathLength, fileSize, hashNum;

    if(argc != 2) {
        write(0, "Uporaba: TCPtimes vrata (vrata 0-1024 so rezervirana za jedro)\n\0", 25);
        exit(1);
    }

    if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
    }
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(atoi(argv[1]));
    if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("bind");
    }
    if(listen(sockfd, 0) < 0) {
        perror("listen");
    }
    length = sizeof(caddr);



#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    while (1) {
        setvbuf (stdout, NULL, _IONBF, 0);
        if((newfd = accept(sockfd, (struct sockaddr *)&caddr, &length)) < 0) {
            perror("accept");
        }
        while(numbytes != 0) {
            bzero(buffer,MAXDATASIZE); // Cleanse buffer
            numbytes = recv(newfd, buffer, sizeof(struct dataInfo), 0);
            dataInfo = (struct dataInfo *) buffer;
            maxIteration = dataInfo->fileSize / MAXDATASIZE;

            if (dataInfo->metadata == file) {
                if (!file_exists(dataInfo->fileName)) {
                    filefd = open(dataInfo->fileName, O_CREAT | O_RDWR, 0777);
                    if (filefd == -1) {
                        perror("Error while creating file");
                        exit(1);
                    }
                }
                if (iteration >= maxIteration) {
                    struct stat fileStatus;
                    write(filefd, dataInfo->fileBuffer, dataInfo->fileSize - maxIteration*MAXDATASIZE);
                    fstat(filefd, &fileStatus);
                    setvbuf (stdout, NULL, _IONBF, 0);
                    int newHash = CalculateHash(filefd);
                    if(newHash != dataInfo->hashNum){
                        remove(dataInfo->fileName);
                    }
                    close(filefd);
                    printf("USPELA\tZBIRKA\t%s\t%ld\n", dataInfo->fileName, fileStatus.st_size);
                    iteration = 0;
                    break;

                } 
                write(filefd, dataInfo->fileBuffer, MAXDATASIZE);

            }
            else if(dataInfo->metadata == directory){
                struct stat fileStatus;


                if (lstat(dataInfo->fileName, &fileStatus) == -1) { 

                    for (int i = 0; i < strlen(dataInfo->fileName); i++) {
                        if (dataInfo->fileName[i] == '/') 
                        {
                            char* newDirectory = malloc(strlen(dataInfo->fileName));
                            strncpy(newDirectory, dataInfo->fileName, i);
                            //strcpy(command, "mkdir");
                            //strcat(command, " ");
                            //strcat(command, dataInfo->fileName);
                            //system(command); //System executes command in shell
                            newDirectory[i] = '\0';
                            mkdir(newDirectory, 0777);
                           
                        }
                    }
                    // Error when using system command?? 
                    mkdir(dataInfo->fileName, 0777); //0777 gives X,R,W to all users
                    iteration = 0;
                    printf("USPEL\tIMENIK\t%s\t%d\n", dataInfo->fileName, 0);
                    continue;
                }

                printf("SPODLETEL\tIMENIK\t%s\t%d\n", dataInfo->fileName, 0);
            }
            iteration++;

        }
        close(newfd);
    }

#pragma clang diagnostic pop
    return 0;
}
