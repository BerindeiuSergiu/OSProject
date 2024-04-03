#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MaxPerms S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH

char globalPath[208] = "/home/bsergiu/TestFiles";

int verifyArgumentsEXIT(int argumentsNumber)
{
    if (argumentsNumber == 3)
    {
        return 0;
    }
    return 1;
}

int verifyName(char *DirectoryName)
{
    struct stat path;
    lstat(DirectoryName, &path);
    return S_ISREG(path.st_mode);
}

void verifyDirEXIT(char *filename)
{
    if ((verifyName(filename)) != 0)
    {
        perror("Fisierul nu este de tip director");
        exit(EXIT_FAILURE);
    }
}


DIR *openDirectory(char *filename)
{
    DIR *dir = NULL;

    if ((dir = opendir(filename)) == NULL)
    {
        return NULL;
    }

    return dir;
}

void printVersion(int fd, struct stat buffer) // a lot of data, mi se pare ca asa e cel mai intuitiv
{
    char st_dev[64];
    sprintf(st_dev, "%ld", buffer.st_dev);
    char st_ino[64];
    sprintf(st_ino, "%ld", buffer.st_ino);
    char st_mode[64];
    sprintf(st_mode, "%d", buffer.st_mode);
    char st_nlink[64];
    sprintf(st_nlink, "%ld", buffer.st_nlink);
    char st_uid[64];
    sprintf(st_uid, "%d", buffer.st_uid);
    char st_gid[64];
    sprintf(st_gid, "%d", buffer.st_gid);
    char st_size[64];
    sprintf(st_size, "%ld", buffer.st_size);

    char data[1024];


	//practic ce ajunge in fisier-ul snapshot
    sprintf(data, "ID: %s\nI-NODE NUMBER: %s\nFile TYPE : %s\nNumber of HARDLINKS: %s\nID OWNER: %s\nID GROUP: %s\nSIZE: %s\n\n", st_dev, st_ino, st_mode, st_nlink, st_uid, st_gid, st_size);



    if (write(fd, data, strlen(data)) == -1)//verificam output write
    {
        perror("Could not write!\n");
        exit(-2);
    }
}

// filename = path

void tree(char *filename)
{
    DIR *directory = NULL;

    if ((directory = openDirectory(filename)) == NULL) // verificam daca s-a deschis directorul corect
    {
        exit(-1);
    }

    char tempFileName[1024];
    struct dirent *directoryInfo;

    while ((directoryInfo = readdir(directory)) != NULL)
    {
        if ((strcmp(directoryInfo->d_name, ".") == 0) || (strcmp(directoryInfo->d_name, "..") == 0)) // trec peste . si ..
        {
            continue;
        }

        if (strstr(directoryInfo->d_name, "snapshot") != NULL) // verific daca are snapshot in nume, daca da, trec peste
        {
            continue;
        }

        sprintf(tempFileName, "%s/%s", filename, directoryInfo->d_name); // creez urmatorul "subdirector in care sa ma duc"
        if (verifyName(tempFileName) == 0)//verific daca e director, pentru a putea continua parcurgerea
        {
            tree(tempFileName);
        }

        char path[1024] = "";
        struct stat buffer;
        int fd = 0;

        sprintf(path, "%s/%s_snapshot", filename, directoryInfo->d_name); // filename pentru locatia lor direct in subdirectorul lor

        if ((fd = open(path, O_WRONLY | O_APPEND | O_CREAT, MaxPerms)) == -1)//verfic file descriptor-ul
        {
            perror("Files could not be created\n");
            exit(EXIT_FAILURE);
        }

        if (lstat(tempFileName, &buffer) == -1)//verific lstat
        {
            perror("Could not get data!\n");
            exit(-1);
        }

        // daca e se poate cu append deschis

        printVersion(fd, buffer); // scrie in fisier, deja e deschis "sanpshot-ul pentru scriere"

        if (close(fd) == -1)
        {
            perror("Could not close the snapshot file\n");
            exit(-3);
        }
    }
    if (closedir(directory) == -1)
    {
        perror("Could not close the directory\n");
        exit(-1);
    }
}




void treeSINGLE(char *filename, char *globalSaveDirectory)//versiunea cu un singur fisier
{
    DIR *directory = NULL;

    if((directory = openDirectory(filename)) == NULL)
    {
        exit(-1);
    }

    char tempFileName[1024];
    struct dirent *directoryInfo;

    printf("%s\n", filename);

    while((directoryInfo = readdir(directory)) != NULL)
    {
        int fd = 0;
    	char path[1024] = "";
    	sprintf(path, "%s/Snapshots.txt", globalSaveDirectory); // filename pentru locatia lor direct in subdirectorul lor

    	if((fd = open(path, O_WRONLY | O_APPEND | O_CREAT, MaxPerms)) == -1)//verfic file descriptor-ul
    	{
      		perror("Files could not be created\n");
      		exit(EXIT_FAILURE);
      	}


        if((strcmp(directoryInfo->d_name, ".") == 0) || (strcmp(directoryInfo->d_name, "..") == 0)) // trec peste . si ..
        {
            continue;
        }

        if(strstr(directoryInfo->d_name, "snapshot") != NULL) // verific daca are snapshot in nume, daca da, trec peste
        {
            continue;
        }

        sprintf(tempFileName, "%s/%s", filename, directoryInfo->d_name); // creez urmatorul "subdirector in care sa ma duc"
        if (verifyName(tempFileName) == 0)//verific daca e director, pentru a putea continua parcurgerea
        {
            treeSINGLE(tempFileName, globalSaveDirectory);
        }

        struct stat buffer;


        if (lstat(tempFileName, &buffer) == -1)//verific lstat
        {
            perror("Could not get data!\n");
            exit(-1);
        }


        // daca e se poate cu append deschis

        printVersion(fd, buffer); // scrie in fisier, deja e deschis "sanpshot-ul pentru scriere"

        if (close(fd) == -1)
        {
            perror("Could not close the snapshot file\n");
            exit(-3);
        }
    }
    if (closedir(directory) == -1)
    {
        perror("Could not close the directory\n");
        exit(-1);
    }
}



int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        perror("Not enough arguments!\n");
        exit(EXIT_FAILURE);
    }
    //while de la 1 la 10
    //argv[11] = directorul unde vrem sa le salvam 
    char tempFileName[CHAR_MAX];
    strcpy(tempFileName, argv[1]);
    

    if(verifyName(argv[2]) != 0)
    {
        perror("Last argument is not a directory");
        exit(-1);
    }


    if (verifyName(argv[1]) == 0)
    {
        treeSINGLE(tempFileName, argv[2]);
    }

    return 0;
}