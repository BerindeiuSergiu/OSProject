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
#define MaxPerms S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH

char izolationPath[1024];

int verifyName(char *DirectoryName)
{
    struct stat path;
    if(lstat(DirectoryName, &path) == -1)
    {
        perror("Could not get data!\n");
        exit(-1);
    }
    return S_ISDIR(path.st_mode);
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
    
    path[strcspn(path, "\n")] = '\0';

    if (lseek(fd, 0, SEEK_SET) == -1) //incep verificarea de la inceputul fisierului
    {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    char buf[1024];
    int bytes_read = read(fd, buf, sizeof(buf));

    if (bytes_read == -1) 
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    buf[bytes_read] = '\0';

    if(atoi(buf) != buffer.st_dev)
    {
        return 1;
    }

    char *aux = strtok(buf, "\n");

    aux = strtok(NULL, "\n");

    aux = strtok(NULL, "\n");
    if(atoi(aux) != buffer.st_mode)
    {
        return 1;
    }

    aux = strtok(NULL, "\n");
    if(atoi(aux) != buffer.st_nlink)
    {
        return 1;
    }

    aux = strtok(NULL, "\n");
    if(atoi(aux) != buffer.st_uid)
    {
        return 1;
    }

    aux = strtok(NULL, "\n");
    if(atoi(aux) != buffer.st_gid)
    {
        return 1;
    }

    aux = strtok(NULL, "\n");
    if(atoi(aux) != buffer.st_size)
    {
        return 1;
    }

    return 0;
}

int verifyPermissions(struct stat buffer) //functie pentru verificare de permisiuni
{
    if(buffer.st_mode & S_IRUSR)
        return 1;
    if(buffer.st_mode & S_IWUSR)
        return 1;
    if(buffer.st_mode & S_IXUSR)
        return 1;
    if(buffer.st_mode & S_IRGRP)
        return 1;
    if(buffer.st_mode & S_IWGRP)
        return 1;
    if(buffer.st_mode & S_IXGRP)
        return 1;
    if(buffer.st_mode & S_IROTH)
       return 1;
    if(buffer.st_mode & S_IWOTH)
        return 1;
    if(buffer.st_mode & S_IXOTH)
        return 1;
    
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

    sprintf(data, "%s ID\n%s I-NODE NUMBER\n%s File TYPE\n%s Number of HARDLINKS\n%s ID OWNER\n%s ID GROUP\n%s SIZE\n", st_dev, st_ino, st_mode, st_nlink, st_uid, st_gid, st_size);


    if (write(fd, data, strlen(data)) == -1)//verificam output write
    {
        perror("Could not write!\n");
        exit(EXIT_FAILURE);
    }
}

// filename = path
///pt functia de verificare, deschid snapshot, compar cu ceea ce am pe moent in i-node-ul file-ului si daca s-a modificat adaug un modificat?///

void treeSINGLE(char *filename, char *globalSaveDirectory, int *nrFisiereCorupte)
{
    DIR *directory = NULL;
    if((directory = openDirectory(filename)) == NULL)
    {
        exit(EXIT_FAILURE);
    }

    char tempFileName[1024];
    struct dirent *directoryInfo;


    while((directoryInfo = readdir(directory)) != NULL)
    {
        int fd = 0;
    	char path[1024];
       
        if((strcmp(directoryInfo->d_name, ".") != 0) || (strcmp(directoryInfo->d_name, "..") != 0))
    	    sprintf(path, "%s/%s_snapshot", globalSaveDirectory, directoryInfo->d_name); // filename pentru locatia lor direct in subdirectorul lor


        if((strcmp(directoryInfo->d_name, ".") == 0) || (strcmp(directoryInfo->d_name, "..") == 0)) // trec peste . si ..
        {
            continue;
        }


        sprintf(tempFileName, "%s/%s", filename, directoryInfo->d_name); // creez urmatorul "subdirector in care sa ma duc"
        if (verifyName(tempFileName) == 1)//verific daca e director, pentru a putea continua parcurgerea
        {
            treeSINGLE(tempFileName, globalSaveDirectory, nrFisiereCorupte);
        }


        struct stat buffer;
        if (lstat(tempFileName, &buffer) == -1)//verific lstat
        {
            perror("Could not get data!\n");
            exit(EXIT_FAILURE);
        }

        if(verifyPermissions(buffer) == 0)
        {
            
            int pfd[2];
            int pid;
            FILE *stream;

            if(pipe(pfd) < 0)
            {
                perror("Eroare la pipe\n");
                exit(EXIT_FAILURE);
            }

            if((pid = fork()) < 0)
            {
                perror("Eroare la creearea copilului\n");
                exit(EXIT_FAILURE);
            }

            if(pid == 0)
            {
                close(pfd[0]);
                dup2(pfd[1], 1);

                execlp("./script.sh", "./script.sh", tempFileName, izolationPath, NULL);
                perror("Eroare la exec\n");
                exit(EXIT_FAILURE);

            }
            //parinte

            close(pfd[1]);

            stream = fdopen(pfd[0], "r"); //trimit prin pipe calea fisierului corupt
            char string[1024];
            if(stream == NULL)
            {
                perror("Eroare citire pipe\n");
                exit(EXIT_FAILURE);
            }
            fscanf(stream, "%s", string);

            close(pfd[0]);
            fclose(stream);

            int status;
            wait(&status);

            if(WIFEXITED(status) == 0)
            {
                perror("Eroare la copil\n");
                exit(EXIT_FAILURE);
            }

            string[strcspn(string, "\n")] = '\0';
            if(strcmp(string, "Safe") != 0)
            {
                //printf("calea fisierului corupt este : %s\n", string); //string = calea fisierului corupt

                char *fileName = strrchr(string, '/'); //ultima aparitie '/'
                fileName++; // trecem de /

                //printf("%s\n", fileName);

                char locatieIzolare[2048];

                sprintf(locatieIzolare, "%s/%s", izolationPath, fileName);

                if(rename(string, locatieIzolare) != 0)
                {
                    perror("Eroare rename\n");
                    exit(-1);
                }

                (*nrFisiereCorupte)++; //cresc numar de fisiere corupte

                continue; // pt a nu face snapshot
            }
            
        }


    	if((fd = open(path, O_RDWR | O_CREAT | O_EXCL, MaxPerms)) == -1)//verfic file descriptor-ul, exc => -1 daca deja exista
    	{
            if((fd = open(path, O_RDWR, MaxPerms)) == -1) //redeschid pentru scriere si pentru verificare in caz de eroare
            {
                perror("naspa\n");
                exit(EXIT_FAILURE);
            }
      		if(verify_snapshot(fd, buffer, path) == 1) //verific daca nu s-a modificat
            {
                printf("FISIERUL: %s s-a modificat\n", path);
                if((fd = open(path, O_RDWR | O_TRUNC | O_CREAT, MaxPerms)) == -1) //redeschid pentru suprascriere
                {
                    perror("naspa");
                    exit(EXIT_FAILURE);
                }
                printVersion(fd, buffer); // intr-un final scriu
            }
      	}else {
            printVersion(fd, buffer);
        }

        if (close(fd) == -1)
        {
            perror("Could not close the snapshot file\n");
            exit(EXIT_FAILURE);
        }

    }
    if (closedir(directory) == -1)
    {
        perror("Could not close the directory\n");
        exit(EXIT_FAILURE);
    }
}



int main(int argc, char *argv[]) // programul este conceput a.i sa primeasca calea absoulta in ./run
{
    if(argc > 15)
    {
        perror("Too many arguments!\n");
        exit(EXIT_FAILURE);
    }

    if(argc < 2)
    {
        perror("Not enough arguments!\n");
        exit(EXIT_FAILURE);
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
        if(strcmp(argv[i], "-x") == 0)
        {
            strcpy(izolationPath, argv[i+1]);
            break;
        }
        strcpy(izolationPath, "No argument provided");
    }

    int cnt_wait = 0;
    
    for(int i = 1; i < argc; i++)
    {
        
        if(strcmp(argv[i], "-o") == 0)
        {
            break;
        }

        if(verifyName(argv[i]) == 0)
        {
            continue;
        }
        
        cnt_wait++;
        int pid = 0;
        if((pid = fork()) < 0)
        {
            perror("Eroare la creearea fiului!\n");
            exit(EXIT_FAILURE);
        }
        
        if(pid == 0)
        {
            int nrFisiereCorupte = 0;
            if(strcmp(snapshotsPath, "No argument provided") != 0)
                treeSINGLE(argv[i], snapshotsPath, &nrFisiereCorupte);
            else
            {
                perror("No save file provided!\n");
                exit(EXIT_FAILURE);
            }
            exit(nrFisiereCorupte);
        }
    }
    printf("\n");

    for(int i = 0 ; i < cnt_wait; i++)
    {
        int status;
        pid_t pid = wait(&status);
        if(WIFEXITED(status))
        {
            printf("IN PARENT : Child process %d terminated with pid %d and exit code %d(nr. de fisiere corupte)\n", i+1, pid, WEXITSTATUS(status));
        }
        else {
            perror("Eroare la copil in main\n");
            exit(EXIT_FAILURE);
        } 
    }
    return 0;
}
