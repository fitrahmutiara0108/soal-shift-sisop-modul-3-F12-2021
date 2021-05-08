#include<stdio.h>
#include<dirent.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>

char *current;
char *temp_dir;

pthread_t thread[5000];

void* pindah(void *arg);
void* pindah_bbrp(void *arg);
void* pindah_semua(void *arg);
void pindah_file(char source[], char destination[]);

char* get_filename(char string[]);
char* get_ext(char string[]);

void pindah_file(char source[], char destination[]){
    int y;
    FILE *file1, *file2;

    file1 = fopen(source, "r");
    file2 = fopen(destination, "w");
    
    if (!file1) {
        fclose(file2);
        return;
    }

    if(!file2) return;

    while((y = fgetc(file1)) != EOF) {
        fputc(y, file2);
    }

    fclose(file1);
    fclose(file2);
    remove(source);
  
    return;
}

char* get_filename(char string[]){
    char* x;
    char* hasilnya;
  
    x = strchr(string,'/');
    if(x == NULL)
        return string;

    while(x != NULL) {
        hasilnya = x+1;
        x = strchr(x+1,'/');
    }
  
    return hasilnya;
}

char* get_ext(char string[]){
    char* x = get_filename(string);
    char* hasilnya = strchr(x, '.');
  
    if(hasilnya == NULL) return NULL;
    else return (hasilnya+1);
}

void* pindah(void *arg){
    int i;
    pthread_t id = pthread_self();
    char *ext_fldr;
    char destination_folder[5000];

    ext_fldr = get_ext((char *)arg);
    if(ext_fldr == NULL){
        strcpy(destination_folder, "Unknown");
    }
    else{
        strcpy(destination_folder, ext_fldr);
        for(i = 0; i < strlen(destination_folder); i++){
            if(destination_folder[i] < 91 && destination_folder[i] > 64)
                destination_folder[i] = destination_folder[i] + 32;
        }
    }
    
    if(mkdir(destination_folder, 0777) == -1);

    char destination_path[5000];
  
    snprintf(destination_path, 5000, "%s/%s/%s", current, destination_folder, get_filename((char *)arg));
    pindah_file((char *)arg, destination_path);
}

void* pindah_semua(void *arg){
    pthread_t id = pthread_self();
    char *ext_fldr;
    char destination_folder[5000];

    ext_fldr = get_ext((char *)arg);
    if(ext_fldr == NULL){
        strcpy(destination_folder, "Unknown");
    }
    else{
        strcpy(destination_folder, ext_fldr);
        int i;
        for(i = 0; i < strlen(destination_folder); i++){
            if(destination_folder[i] < 91 && destination_folder[i] > 64)
                destination_folder[i] = destination_folder[i] + 32;
        }
    }

    if(mkdir(destination_folder, 0777) == -1);

    char destination_path[5000];
    char source_path[5000];
  
    snprintf(source_path, 5000, "%s/%s", current, (char *)arg);
    snprintf(destination_path, 5000, "%s/%s/%s", current, destination_folder, get_filename((char *)arg));
    pindah_file(source_path, destination_path);
}

void* pindah_bbrp(void *arg){
    int i;
    pthread_t id = pthread_self();
    char *ext_fldr;
    char destination_folder[5000];

    ext_fldr = get_ext((char *)arg);

    if(ext_fldr == NULL)
        strcpy(destination_folder, "Unknown");
    else{
        strcpy(destination_folder, ext_fldr);
        for(i=0; i<strlen(destination_folder); i++)
            if(destination_folder[i] < 91 && destination_folder[i] > 64)
                destination_folder[i] = destination_folder[i] + 32;
    }
            
    if(mkdir(destination_folder, 0777) == -1);

    char destination_path[5000];
    char source_path[5000];
    snprintf(source_path, 5000, "%s/%s", temp_dir, (char *)arg);
    snprintf(destination_path, 5000, "%s/%s/%s", current, destination_folder, get_filename((char *)arg));
    pindah_file(source_path, destination_path);
}

int main(int argc, char **argv){
    int i=2, j;
    char check[1000];
    current = getcwd(check, 1000);

    if(strcmp(argv[1], "-f") == 0){
    	
        while(argv[i] != NULL){
        	
           int cekin = pthread_create(&(thread[i-2]), NULL, &pindah, (void *)argv[i]);
          
           if(cekin==0)printf("File %d : Berhasil Dikategorikan\n", i-1);
           else printf("File % : Sad, Gagal :(\n", i-1);
           i++;
        }
        
        for(j = 0; j < (i-1); j++) pthread_join(thread[j], NULL);
        
    }
}
