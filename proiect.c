#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>


int verifyArguments(int argumentsNumber)
{
    if(argumentsNumber != 2)
    {
        return 1;
    }
    return 0;
}


int verifyName(char *DirectoryName)
{
    struct stat path;
    stat(DirectoryName, &path);
    return S_ISREG(path.st_mode);
}


void verifyDirEXIT(char *filename)
{
    if((verifyName(filename)) != 0)
    {
        perror("Fisierul nu este de tip director");
        exit(EXIT_FAILURE);
    }
}


void parseDirectory(DIR *dir)
{
    struct dirent *date = NULL;
    while((date = readdir(dir)) != NULL)
    {
        printf("%s\n", date->d_name);
    }
}


DIR *openDirectory(char *filename)
{
    DIR *dir = NULL;
    if((dir = opendir(filename)) == NULL)
    {
        return NULL;
    }

    return dir;
}

void tree(char *filename)
{
    DIR *directory = NULL;

    if((directory = openDirectory(filename)) == NULL)
    {
        return;
    }


    char tempFileName[CHAR_MAX];
    struct dirent *directoryInfo;


    printf("Ne aflam in directorul:%s\n", filename);
    while((directoryInfo = readdir(directory)) != NULL)
    {
        if(strcmp (directoryInfo->d_name, "." ) != 0 && strcmp(directoryInfo->d_name, "..") != 0)
        {
            printf("%s\n", directoryInfo->d_name);

            strcpy(tempFileName, filename);
            strcat(tempFileName, "/");
            strcat(tempFileName, directoryInfo->d_name);

            tree(tempFileName);
        }
    }

    closedir(directory);
}

int main(int argc, char *argv[])
{
    //DIR *directory = NULL;

    if(verifyArguments(argc) == 1)
    {
        perror("Insufficient arguments!\n");
        exit(1);
        
    }

    verifyDirEXIT(argv[1]);

    
    if(verifyName(argv[1]) == 0)
    {
        char tempFileName[CHAR_MAX];
        
        strcpy(tempFileName, argv[1]);
        printf("%s\n", tempFileName);
    


        tree(tempFileName);
    }


    return 0;
}