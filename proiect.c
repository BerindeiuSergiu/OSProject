#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

//pt fiecare argument primit se va creea un proces separat
//daca nu e director nu voi creea proces, daca e -o nu creem director etc...
//procesul face ce va face parintele, adica fiecare fiu in parte o sa se ocupe de un singur argument
//acel fiu se va ocupa de tot, parintele creeaza procese separate si e responsabil sa astepte sa se termine fii, cate procese am creeat, dupa atatea trebuie sa astept:)


#define MaxPerms S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH

char globalPath[208] = "/home/bsergiu/SnapShotsGLOBAL";

int verifyName(char *DirectoryName)
{
    struct stat path;
    lstat(DirectoryName, &path);
    return S_ISREG(path.st_mode);
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

int verify_snapshot(int fd, struct stat buffer, char path[]) 
{
    // char st_dev[64];
    // char st_ino[64];
    // char st_mode[64];
    // char st_nlink[64];
    // char st_uid[64];
    // char st_gid[64];
    // char st_size[64];


    // sprintf(st_dev, "%ld", buffer.st_dev);
    // sprintf(st_ino, "%ld", buffer.st_ino);
    // sprintf(st_mode, "%d", buffer.st_mode);
    // sprintf(st_nlink, "%ld", buffer.st_nlink);
    // sprintf(st_uid, "%d", buffer.st_uid);
    // sprintf(st_gid, "%d", buffer.st_gid);
    // sprintf(st_size, "%ld", buffer.st_size);

    


    path[strcspn(path, "\n")] = '\0';
    
    // char *x = "/home/bsergiu/Snapshots/dir3_4_3_snapshot";
    
    // printf("%s\n", x);
    // int da = open(x, O_RDWR, MaxPerms);


    if (lseek(fd, 0, SEEK_SET) == -1) 
    {
        perror("lseek");
        return -1; 
    }



    char buf[1024];

    int bytes_read = read(fd, buf, sizeof(buf));

    if (bytes_read == -1) 
    {
        perror("read");
        return -1; 
    }
    buf[bytes_read] = '\0';

    printf("%s\n", path);
    printf("%d\n", atoi(buf));



    return 0;
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
    //sprintf(data, "ID: %s\nI-NODE NUMBER: %s\nFile TYPE : %s\nNumber of HARDLINKS: %s\nID OWNER: %s\nID GROUP: %s\nSIZE: %s\n\n", st_dev, st_ino, st_mode, st_nlink, st_uid, st_gid, st_size);

    sprintf(data, "%s ID\n%s I-NODE NUMBER\n%s File TYPE\n%s Number of HARDLINKS\n%s ID OWNER\n%s ID GROUP\n%s SIZE\n", st_dev, st_ino, st_mode, st_nlink, st_uid, st_gid, st_size);


    if (write(fd, data, strlen(data)) == -1)//verificam output write
    {
        perror("Could not write!\n");
        exit(-2);
    }
}

// filename = path
///pt functia de verificare, deschid snapshot, compar cu ceea ce am pe moent in i-node-ul file-ului si daca s-a modificat adaug un modificat?///

void treeSINGLE(char *filename, char *globalSaveDirectory)
{
    DIR *directory = NULL;

    if((directory = openDirectory(filename)) == NULL)
    {
        exit(-1);
    }

    char tempFileName[1024];
    struct dirent *directoryInfo;


    while((directoryInfo = readdir(directory)) != NULL)
    {
        int fd = 0;
    	char path[1024];
       
        if((strcmp(directoryInfo->d_name, ".") != 0) || (strcmp(directoryInfo->d_name, "..") != 0))
    	    sprintf(path, "%s/%s_snapshot", globalSaveDirectory, directoryInfo->d_name); // filename pentru locatia lor direct in subdirectorul lor

        //introducere existsSnapshot?


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


    	if((fd = open(path, O_RDWR | O_CREAT | O_EXCL, MaxPerms)) == -1)//verfic file descriptor-ul
    	{
            if((fd = open(path, O_RDWR, MaxPerms)) == -1)
            {
                perror("naspa");
                exit(EXIT_FAILURE);
            }
      		verify_snapshot(fd, buffer, path);
      	}else 
        {
            printVersion(fd, buffer); // scrie in fisier, deja e deschis "snapshot-ul pentru scriere"
        }


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
    if(argc > 13)
    {
        perror("Too many arguments!\n");
        exit(-1);
    }

    if(argc < 2)
    {
        perror("Not enough arguments!\n");
        exit(-2);
    }

    char snapshotsPath[1024];

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-o") == 0)
        {
            strcpy(snapshotsPath, argv[i+1]);
            break;
        }
        strcpy(snapshotsPath, "No argument provided");
    }


    
    for(int i = 1; i < argc; i++)
    {
        int pid = 0;
        if(strcmp(argv[i], "-o") == 0)
        {
            break;
        }

        if((pid = fork()) < 0)
        {
            perror("Eroare la creearea fiului!\n");
            exit(-3);
        }
        
        if(pid == 0)
        {
            if(strcmp(snapshotsPath, "No argument provided") != 0)
                treeSINGLE(argv[i], snapshotsPath);
            else
                treeSINGLE(argv[i], globalPath);
            exit(1);
        }
        wait(NULL);

    }
}
