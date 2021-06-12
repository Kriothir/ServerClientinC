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
#include <dirent.h>

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

int checkType(char *filePath) {
    struct stat fileInfo;
    stat(filePath, &fileInfo);
    return S_ISREG(fileInfo.st_mode);
}

int calculateHash(int fd) { 
 
    struct stat fileStat;
    fstat(fd, &fileStat);
    int hash = fileStat.st_size / MAXDATASIZE;
    return hash * 23478;
}

void removeSubstring(char *string, char *sub) {
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match + len);
    }
}

int main(int argc, char **argv) {
    int sockfd, numbytes;
    int filefd;
    int sendfd;

    uint32_t metadata, pathLength, fileSize, hashNum;
    
    struct sockaddr_in their_addr;
    struct hostent *he;
    struct dirent* dirent_stat;

    char DIRECTORYPATH[MAXDATASIZE];
    char DirString[MAXDATASIZE];
    char buffer[MAXDATASIZE];
    char *filePath;
    strcpy(DIRECTORYPATH, "./");

        
    if (argc != 4) {
        write(0, "Insufficient amount of arguments", 25);
        exit(1);
    }
    he = gethostbyname(argv[1]);
    if (he == NULL) {
        herror("Error while retrieving hostname");
        exit(1);
    }
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Error while creating socket");
        exit(1);
    }
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(atoi(argv[2]));
    their_addr.sin_addr = *((struct in_addr *) he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);

    if (connect(sockfd, (struct sockaddr *) &their_addr, sizeof(struct sockaddr)) == -1) {
        perror("Error while connecting to server");
        exit(1);
    }
    if (checkType(argv[3])) {

        if (strstr(argv[3], "/") == NULL) {
            filefd = open(argv[3], O_RDONLY);
            struct stat fileInfo;
            fstat(filefd, &fileInfo);
            struct dataInfo dataToSend;
            if (filefd == -1) {
                perror("Error occurred while opening file");
                exit(1);
            }
            dataToSend.metadata = file;
            dataToSend.fileSize = fileInfo.st_size;
            dataToSend.pathLength = strlen(argv[3]);
            dataToSend.hashNum = calculateHash(filefd);
            strncpy(dataToSend.fileName, argv[3], strlen(argv[3]));
            dataToSend.fileName[strlen(argv[3])] = '\0';

            while (read(filefd, dataToSend.fileBuffer, MAXDATASIZE) != 0) {
                numbytes = send(sockfd, &dataToSend, sizeof(dataToSend), 0);
                if (numbytes == -1) {
                    perror("Error occurred while sending file");
                    exit(1);
                }
            }
                            printf("USPEL\tZBIRKA\t%s\t%d\n", dataToSend.fileName, dataToSend.fileSize);

            close(sockfd);
        } else {
            struct dataInfo folderInfo;
            char *match = malloc(8);
            char *folderPath = argv[3];
            for (int i = 0; i < strlen(folderPath); i++) {
                if (folderPath[i] == '/') {
                    strncpy(match, folderPath, i);
                    match[i] = '\0';
                    break;
                }
            }
            //Send folder
            folderInfo.metadata = directory;
            folderInfo.hashNum = 0;
            folderInfo.fileSize = 0;
            folderInfo.pathLength = strlen(match);
            strcpy(folderInfo.fileName, match);
            folderInfo.fileName[strlen(match)] = '\0';
            numbytes = send(sockfd, &folderInfo, sizeof(folderInfo), 0);
            if (numbytes == -1) {
                perror("Error ocurred while sending folder");
                exit(1);
            }
            printf("USPEL\tIMENIK\t%s\t%d\n", folderInfo.fileName, 0);
            //

            // Actual file
            filefd = open(argv[3], O_RDONLY);
            struct stat fileInfo;
            fstat(filefd, &fileInfo);
            struct dataInfo dataToSend;
            if (filefd == -1) {
                perror("Error occurred while opening file");
                exit(1);
            }
            dataToSend.metadata = file;
            dataToSend.fileSize = fileInfo.st_size;
            dataToSend.pathLength = strlen(argv[3]);
            dataToSend.hashNum = calculateHash(filefd);
            strncpy(dataToSend.fileName, argv[3], strlen(argv[3]));
            dataToSend.fileName[strlen(argv[3])] = '\0';

            while (read(filefd, dataToSend.fileBuffer, MAXDATASIZE) != 0) {
                numbytes = send(sockfd, &dataToSend, sizeof(dataToSend), 0);
                if (numbytes == -1) {
                    perror("Error occurred while sending file");
                    exit(1);
                }
            }
            printf("USPEL\tZBIRKA\t%s\t%d\n", dataToSend.fileName, dataToSend.fileSize);
            close(sockfd);
        }

    }
    else{
        struct dataInfo folderInfo;
        char *match = malloc(8);
        char *folderPath = argv[3];
        for (int i = 0; i < strlen(folderPath); i++) {
            if (folderPath[i] == '/') {
                strncpy(match, folderPath, i);
                match[i] = '\0';
                break;
            }
        }

        //Send folder
        folderInfo.metadata = directory;
        folderInfo.hashNum = 0;
        folderInfo.fileSize = 0;
        folderInfo.pathLength = strlen(match);
        strcpy(folderInfo.fileName, match);
        folderInfo.fileName[strlen(match)] = '\0';
        numbytes = send(sockfd, &folderInfo, sizeof(folderInfo), 0);
        if (numbytes == -1) {
            perror("Error ocurred while sending folder");
            exit(1);
        }
        printf("USPEL\tIMENIK\t%s\t%d\n", folderInfo.fileName, 0);

        strcat(DIRECTORYPATH, match);
        DIR* dir = opendir(DIRECTORYPATH);
        if (dir == NULL) {
            perror("Directory doesn't exist!");
            exit(1);
        }

        while ((dirent_stat = readdir(dir)) != NULL) {
            strcpy(DirString, match);
            strcat(DirString, "/");
            strcat(DirString, dirent_stat->d_name);

            if (!checkType(DirString)) {
                folderInfo.metadata = directory;
                folderInfo.hashNum = 0;
                folderInfo.fileSize = 0;
                folderInfo.pathLength = strlen(DirString);
                strcpy(folderInfo.fileName, DirString);
                folderInfo.fileName[strlen(DirString)] = '\0';
                numbytes = send(sockfd, &folderInfo, sizeof(folderInfo), 0);
                if (numbytes == -1) {
                    perror("Error ocurred while sending folder");
                    exit(1);
                }
                //NOTICE: Ta pogoj se uporablja izključno samo zato, da ne izpiše current in home directory v konzolo. Je zgolj astetičen dodatek, da se ujema 
                // z želenim izpisom, tako kot je na sistemu za vaje. V primeru, da vsebuje ime directorya ".", potem je potrebno samo ta del odstraniti.
                //
                if(!strstr(DirString, ".")){
                printf("USPEL\tIMENIK\t%s\t%d\n", folderInfo.fileName, 0);
                }
            }
            
            else if (checkType(DirString)) {
                filefd = open(DirString, O_RDONLY);
                struct stat fileInfo;
                fstat(filefd, &fileInfo);
                struct dataInfo dataToSend;
                if (filefd == -1) {
                    perror("Error occurred while opening file");
                    exit(1);
                }
                dataToSend.metadata = file;
                dataToSend.fileSize = fileInfo.st_size;
                dataToSend.pathLength = strlen(DirString);
                dataToSend.hashNum = calculateHash(filefd);
                strncpy(dataToSend.fileName, DirString, strlen(DirString));
                dataToSend.fileName[strlen(DirString)] = '\0';

                while (read(filefd, dataToSend.fileBuffer, MAXDATASIZE) != 0) {
                    numbytes = send(sockfd, &dataToSend, sizeof(dataToSend), 0);
                    if (numbytes == -1) {
                        perror("Error occurred while sending file");
                        exit(1);
                    }
                }
                printf("USPEL\tZBIRKA\t%s\t%d\n", dataToSend.fileName, dataToSend.fileSize);
            }
        }
    }
    close(sockfd);

    return 0;
}
