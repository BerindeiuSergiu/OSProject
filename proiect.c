#include <stdio.h>
#include <stdlib.h>

int verifyArguments(int argumentsNumber)
{
    if(argumentsNumber != 2)
    {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if(verifyArguments(argc) == 1)
    {
        perror("Insufficient arguments!\n");
        exit(1);
        
    }




    return 0;
}