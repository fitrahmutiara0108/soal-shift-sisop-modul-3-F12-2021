# Soal Shift Modul 3 Sisop 2021 (Kelompok F12)
#### Nama anggota kelompok:
- Farhan Arifandi (05111940000061)
- Fitrah Mutiara (05111940000126)
- M. Iqbal Abdi (05111940000151)

## Soal 1
## Soal 2
## Soal 3
- fungsi `get_ext` untuk mendapatkan eksistensi dari file
- menggunakan strchr untuk mncari `.` pada file
- jika `ext==NULL` maka Unknown akan dicopy ke exten
- jika `ext==fileName` maka Hidden akan docopy ke exten
- selain itu ext+1 agar tanda ext(.) tidak ikut ke dalam exten
```c
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
```
- fungsi `checkExistAndRegFile` digunakan untuk mengecek apakah file termasuk file regular atau tidak
- apabila file yang tersedia merupakan file regular makan return true
```c
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
```
- fungsi `pindah_file` digunakan untuk memindahkan file ke dalam suatu direktori di dalam working directory
- untuk mendapatkan working direcory dilakukan `getcwd(cwd, sizeof(cwd))`
- setelah itu dilakukan sprintf untuk menggabungkan ext dan filename ke dalam destDir 
- lalu di sprintf kembali utnuk menggabungkan ext,cwd, dan filename ke dalam dest
- kemudian dest dipindahkan ke basePath
```c
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
```
- digunakan untuk mengecek file yang ada dalam folder secara rekursif dengan cara membuka direktori dari basePath
- setelah itu di cek jika file bukan directory makan akan mengembalikkan 0
- lalu dilakukan perulangan untuk mengecek file
- count+=1 digunakan untuk menghitung banyaknya file
- `sprintf(fileList[*fileCount],"%s",fullPath);` `*fileCount += 1;` untuk memindahkan fullPath ke dalam fileList sehingga semua file yang akan dicek nantinya akan masuk ke dalam fileList
```c
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
}
```
### Poin (a)
- terdapat pada fungsi main
- digunakan untuk memindahkan file ke dalam folder yang sesuai dengan eksistensinya secara manual
- apabila berhasil maka akan keluar `File %d : Berhasil Dikategorikan`
- apabila gagal maka akan keluar `File %d : Sad, gagal :(`

```c
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
```
### Poin (b)
- terdapat pada fungsi main
- digunakan untuk memindahkan file dari suatu direktori ke dalam folder yang terletak di working direktori
- jika berhasil akan keluar `Direktori sukses disimpan`
- jika gagal akan keluar `Yah, gagal disimpan`
```c
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
```
### Poin (c)
- terdapat pada fungsi main
- digunakan untuk memindahkan file ke dalam folder yang sesuai dengan eksistensi secara otomatis
```c
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
```
### Poin (d) dan (e)
- poin d dijelaskan pada fungsi `get_ext` diatas
- 1 file yang dikategorikan dioperasikan oleh 1 thread
### Output
- melakukan `gcc -pthread -o soal3 soal3.c` terlebih dahulu
- ketika dicoba menggunakan input `./soal3 -f b.c` dan `./soal3 -f IZONE_Logo.png nako.zip` 
![Screenshot (1103)](https://user-images.githubusercontent.com/81247727/119217459-917bb900-bb04-11eb-8cde-313c6b9de5ed.png)
- Dapat dilihat bahwa terbentuk folder c, png, dan zip
![Screenshot (1104)](https://user-images.githubusercontent.com/81247727/119217516-e61f3400-bb04-11eb-9cd7-0ddbaf932b92.png)
- ketika dicoba menggunakan input `./soal3 -d /home/tiara/Modul3/yuhu`
![Screenshot (1105)](https://user-images.githubusercontent.com/81247727/119217559-31394700-bb05-11eb-8077-3cfa25b14f4e.png)
- Dapat dilihat bahwa pada direktori tersebut sukses sehingga output yang keluar Direktoti sukses disimpan
![Screenshot (1106)](https://user-images.githubusercontent.com/81247727/119217616-8b3a0c80-bb05-11eb-8f43-ce2c52e9b8bf.png)
- ketika dicoba menggunakan input `./soal3 \*`
![Screenshot (1107)](https://user-images.githubusercontent.com/81247727/119217642-a86edb00-bb05-11eb-8d3e-0c12fc7c42ba.png)
- Dapat dilihat bahwa semua file secara otomatis masuk ke dalam folder yang sesuai dengan eksistensinya
![Screenshot (1108)](https://user-images.githubusercontent.com/81247727/119217661-c9373080-bb05-11eb-93b3-e2aaa295f647.png)


### Kendala dan Error selama pengerjaan
![Screenshot (1102)](https://user-images.githubusercontent.com/81247727/119217398-2df18b80-bb04-11eb-9b50-15b1875b138a.png)
- Muncul peringatan warning di terminal 
