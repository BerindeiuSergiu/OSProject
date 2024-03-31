#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


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



int main(int argc, char *argv[])
{
    if(verifyArguments(argc) == 1)
    {
        perror("Insufficient arguments!\n");
        exit(1);
        
    }
    printf("%s\n", argv[1]);
    verifyDirEXIT(argv[1]);
    //DIR *directorCurent = NULL;


    return 0;
}