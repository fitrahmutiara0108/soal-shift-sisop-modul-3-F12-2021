#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>

char file_list[2048][PATH_MAX];

void get_ext(char* fileName, char *exten){
    char *ext = strchr(fileName, '.');
    if (ext == NULL) {
        strcpy(exten,"Unknown");
    } 
    else if (ext == fileName){
        strcpy(exten,"Hidden");
    }
    else{
        strcpy(exten,ext+1);
    }
}

bool checkExistAndRegFile(char *basePath){
    struct stat buff;
    int exist = stat(basePath,&buff);
    if(exist == 0){
        if( S_ISREG(buff.st_mode) ) return true;
        else return false;
    }
    else  
        return false;
}

void *pindah_file( void *arg ){
    char basePath[PATH_MAX];
    strcpy(basePath,(char *) arg);

    if(checkExistAndRegFile(basePath)){
        const char *p="/";
        char *a,*b;
        char fullPath[PATH_MAX];
        strcpy(fullPath,(char *) arg);

        char fileName[100];

        for( a=strtok_r(fullPath,p,&b) ; a!=NULL ; a=strtok_r(NULL,p,&b)){
            memset(fileName,0,sizeof(fileName));
            strcpy(fileName,a);
        }

        char ext[PATH_MAX];
        get_ext(fileName,ext);

        if(strcmp(ext,"Hidden") != 0 && strcmp(ext,"Unknown") != 0){
            for(int i = 0; i<strlen(ext); i++){
                ext[i] = tolower(ext[i]);
            }
        }
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL){
            perror("getcwd() error");
            return (void *) 0;
        }

        char destDir[PATH_MAX];
        sprintf(destDir,"%s/%s",cwd,ext);
        mkdir(destDir,0777);

        char dest[PATH_MAX];
        sprintf(dest,"%s/%s/%s",cwd,ext,fileName);
        rename(basePath,dest);
        return (void *) 1;
    }
    else return (void *) 0;
}

int listFilesRecursively(char *basePath, int *fileCount){
    char path[PATH_MAX];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
        return 0;

    while ((dp = readdir(dir)) != NULL){
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
            char fullPath[PATH_MAX];
            sprintf(fullPath,"%s/%s",basePath,dp->d_name);
            if(checkExistAndRegFile(fullPath)){
                sprintf(file_list[*fileCount],"%s",fullPath);
                *fileCount += 1;
            }
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);

            listFilesRecursively(path,fileCount);
        }
    }
    closedir(dir);
    return 1;
}

//fungsi main
int main(int argc,char* argv[]){
    if(argc<2){
        printf ("invalid argumen\n");
        return 0;
    }

    char baseDir[PATH_MAX];
    if(!strcmp(argv[1],"*")){
        if(argc != 2){
            printf ("* Didn't Need Another Argument\n");
            exit(1);
            return 0;
        }
        if (getcwd(baseDir, sizeof(baseDir)) == NULL) {
            perror("getcwd() error");
            return 0;
        }
    }
    else if(!strcmp(argv[1],"-f")){
        if(argc<3){
            printf ("Need Minimal 1 Path to File\n");
            exit(1);
            return 0;
        }

        pthread_t tid[argc-2];

        for(int i = 2; i<argc; i++){
            pthread_create( &(tid[i-2]), NULL, pindah_file, (void*) argv[i]);
        }

        for (int i = 0; i < argc-2; i++){
            int returnValue;
            void *ptr;
            pthread_join( tid[i], &ptr);
            returnValue = (int) ptr;
            if(returnValue) printf("File %d : Berhasil Dikategorikan\n", i+1);
            else printf("File %d : Sad, gagal :(\n", i+1);
        }
        
        return 0;
    }
    else if(!strcmp(argv[1],"-d")){
        if(argc != 3){
            printf ("Only Need 1 Path to Directory\n");
            exit(1);
            return 0;
        } 
        strcpy(baseDir,argv[2]);
    }
    else printf ("invalid argumen\n");

    int fileCount = 0;
    if(!listFilesRecursively(baseDir, &fileCount)){
        printf("Yah, gagal disimpan :(\n");
        return 0;
    }
    pthread_t tid[fileCount];
    for(int i = 0; i<fileCount; i++){
        pthread_create( &(tid[i]), NULL, pindah_file, (void*) file_list[i]);
    }

    for (int i = 0; i < fileCount; i++){
        void *ptr;
        pthread_join( tid[i], &ptr);
    }

    if(!strcmp(argv[1],"-d")) printf("Direktori sukses disimpan!\n");
    return 0;
}
