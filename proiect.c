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

int verifyDirectory(char *DirectoryName)
{
    struct stat path;
    stat(DirectoryName, &path);
    return S_ISREG(path.st_mode);
}


int main(int argc, char *argv[])
{
    if(verifyArguments(argc) == 1)
    {
        perror("Insufficient arguments!\n");
        exit(1);
        
    }

    DIR *directorCurent;


    return 0;
}