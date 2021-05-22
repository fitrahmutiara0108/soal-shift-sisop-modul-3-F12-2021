# Soal Shift Modul 3 Sisop 2021 (Kelompok F12)
#### Nama anggota kelompok:
- Farhan Arifandi (05111940000061)
- Fitrah Mutiara (05111940000126)
- M. Iqbal Abdi (05111940000151)

## Kendala secara umum selama pengerjaan
Waktu pengerjaan soal shift yang berbarengan dengan minggu ETS menyebabkan pengerjaan tidak maksimal.

## Soal 1
### Kendala dan Error selama pengerjaan
- Sama seperti kendala di atas, ditambah dengan banyaknya command yang harus dikerjakan menyebabkan soal 1 tidak dapat diselesaikan tepat waktu dan baru bisa diunggah pada masa revisi.
- Banyak sekali error yang terjadi saat pengerjaan hingga tidak mampu didokumentasikan, contohnya puluhan kali syntax error, file yang tidak dapat terkirim dan di-download, kesalahan alokasi memori pada program, file **running.log** dan **files.tsv** yang tidak ter-update saat terjadi penambahan/penghapusan file, dan infinite loop saat menampilkan informasi file pada command `see`.

### Poin (a)
1. Ketika program server dijalankan, program akan terlebih dahulu membuat file **akun.txt**, **files.tsv**, **running.log**, dan direktori **FILES** pada folder Server melalui fungsi `check_files`, kemudian membuat koneksi socket untuk komunikasi antara program server dan client. Koneksi pada program client ditutup jika mengalami kegagalan.

**Server**
```c
int create_socket() {
    struct sockaddr_in serv_addr;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), return_val;

    if (fd == -1) {
        printf("\n Socket creation failed. \n");
		return -1;
    }
    printf("Socket creation success with fd: %d\n", fd);

    serv_addr.sin_family = AF_INET;         
    serv_addr.sin_port = htons(8080);     
    serv_addr.sin_addr.s_addr = INADDR_ANY; 

	return_val = bind(fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    if (return_val != 0) {
       	fprintf(stderr, "Binding failed [%s]\n", strerror(errno));
        close(fd); return -1;
    }
    
	return_val = listen(fd, 5);
    if (return_val != 0) {
        fprintf(stderr, "Listen failed [%s]\n", strerror(errno));
        close(fd); return -1;
    }
    return fd;
}

void check_files() {
    if(access("akun.txt", F_OK)) {
		FILE *file = fopen("akun.txt", "w+");
		fclose(file);
	} 
    if(access("files.tsv", F_OK)) {
		FILE *file = fopen("files.tsv", "w+");
        fprintf(file, "Publisher\tTahun Publikasi\tFilepath\n");
		fclose(file);
	}
	if(access("running.log", F_OK )) {
		FILE *file = fopen("running.log", "w+");
		fclose(file);
	}

    struct stat s;
    stat("./FILES", &s);

    if(!S_ISDIR(s.st_mode)) mkdir("./FILES", 0777);
}

...

int main () {
    fd_set read_fd_set;
    struct sockaddr_in new_addr;
    int server_fd, new_fd, isServing=1, i=0,
		return_value, return_value1, return_value2, return_value3, status_value, connections[10];
    char message[100], id[100], password[100], command[100], userdata[256];
    socklen_t addrlen;

    check_files();
    server_fd = create_socket(); 
    if (server_fd == -1) {
        fprintf(stderr, "Server socket creation failed.\n");
        return -1; 
    }   
    
	for (; i<10; i++) connections[i] = -1;
    connections[0] = server_fd;
    ...
```
**Client**
```c
int main(){
    struct hostent *lh;
	struct sockaddr_in server_addr;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), return_val;
    char message[100], command[100], buffer[1000]={0};
    
    if (fd == -1) {
		printf("\n Socket creation failed. \n");
		return -1;
	}
    
	server_addr.sin_family = AF_INET;         
    server_addr.sin_port = htons(8080);
    lh = gethostbyname("127.0.0.1");
    server_addr.sin_addr = *((struct in_addr *)lh->h_addr);
    
    return_val = connect(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (return_val == -1) {
        fprintf(stderr, "Connect failed [%s]\n", hstrerror(errno));
        close(fd);
        return -1;
    }
    ...
}
```
**Dokumentasi**

![Screenshot from 2021-05-22 16-37-04](https://user-images.githubusercontent.com/70105993/119225200-f8b16180-bb34-11eb-9169-000259dd3883.png)

2. Kemudian, program akan menangani kondisi saat terdapat 2 koneksi client atau lebih, client yang terhubung lebih akhir harus menunggu untuk bisa login dan mengakses aplikasi. Server akan mengirim pesan `serve` pada client yang dapat mengakses aplikasi, dan mengirim pesan `wait` pada client yang menunggu untuk mengakses aplikasi.

**Server**
```c
int main () {
    ...
	printf("\nServer is running\n\n");
    int login=0;
    while (1) {
        FD_ZERO(&read_fd_set);
        for (i=0; i<10; i++) {
            if (connections[i] >= 0)  FD_SET(connections[i], &read_fd_set);
        }
        return_value = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        if (return_value >= 0 ) {
            if (FD_ISSET(server_fd, &read_fd_set)) { 
                new_fd = accept(server_fd, (struct sockaddr*)&new_addr, &addrlen);
                if (new_fd >= 0) {
                    printf("New connection incoming:\n\n");
                    for (i=0; i < 10; i++) {
                        if (connections[i] < 0) {
                            connections[i] = new_fd;

                            if(i != isServing)  return_value1 = send(connections[i], "wait",  100, 0);
                            else return_value1 = send(connections[i], "serve",  100, 0);
                            break;
                        }
                    }
                } else fprintf(stderr, "Accepting failed [%s]\n", strerror(errno));
                
                return_value--;
                if (!return_value) continue;
            } 
            ...
}
```

**Client**
```c
int main(){
    ...
    return_val = recv(fd, message, 100, 0);
    while(!strcmp(message, "wait")) {
        printf("\e[31mServer is full, please wait.\e[0m\n");
        return_val = recv(fd, message, 100, 0);
    }
    ...
}
```

**Dokumentasi**

![Screenshot from 2021-05-22 18-47-03](https://user-images.githubusercontent.com/70105993/119225217-0a930480-bb35-11eb-9af6-2ae28547a2c9.png)
![Screenshot from 2021-05-22 18-47-11](https://user-images.githubusercontent.com/70105993/119225220-0bc43180-bb35-11eb-9115-255d8a61fbb3.png)

3. Pada client yang pertama kali terhubung dengan server, saat client tersebut tersambung dengan server, terdapat dua pilihan pertama, yaitu register dan login. Jika memilih register, client akan diminta input id dan passwordnya untuk dikirimkan ke server. User juga dapat melakukan login. Login berhasil jika id dan password yang dikirim dari aplikasi client sesuai dengan list akun yang ada di dalam aplikasi server.

**Server**

- Pada command `register`, server akan mengirim prompt pendaftaran akun, lalu mengecek ID dan password yang dimasukkan client pada fungsi `get_account`. Jika ID yang dimasukkan client sudah ada pada file **akun.txt**, maka server akan mengirim pesan `user_exists` ke client dan kembali meminta pengguna login/register. Jika user ID tidak ditemukan pada file **akun.txt**, server menulis ID dan password client pada file **akun.txt** melalui fungsi `append_account_to_akun_txt`, mengirim pesan `account_get` ke client dan mengizinkan pengguna mengakses aplikasi.
- Pada command `login`, server akan mengecek id dan password yang dimasukkan client pada fungsi `get_account`. Jika user ID/password yang dimasukkan salah atau tidak terdaftar pada file **akun.txt**, maka server akan mengirim pesan `wrong_account` ke client dan kembali meminta pengguna login/register. Jika ID dan password yang dimasukkan client terdaftar pada file **akun.txt**, server mengirim pesan `account_get` ke server dan mengizinkan pengguna mengakses aplikasi.
```c
int get_account(int mode, char id[], char password[]){
    char akun[100], temp[100];
    FILE *file = fopen("akun.txt", "r");
    if(mode) sprintf(akun, "%s:%s", id, password);
    else sprintf(akun, "%s:", id);

    while(fscanf(file,"%s", temp) == 1){
        if(strstr(temp, akun)!=0) {
            fclose(file);
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void append_account_to_akun_txt(const char *id, const char *password) {
    FILE *file = fopen("akun.txt", "a+");
    fprintf(file, "%s:%s\n", id, password);
    fclose(file);
}
...
int main () {
            ...
            for (i=1; i<10; i++) {
                if ((connections[i]>0) && (FD_ISSET(connections[i], &read_fd_set))) {
                    return_value1 = recv(connections[i], command, sizeof(command), 0);
                    printf("Returned fd is %d [index, i: %d]\nCommand: %s\n", connections[i], i, command);
					
                    if (!return_value1) {
                        printf("Closing connection for fd: %d\n", connections[i]);
                        printf("ID: %s\nPassword: %s\n\n", id, password);
                        
                        id[0] = 0; password[0] = 0;
                        close(connections[i]);
                        connections[i] = -1;

                        while(1) {
                            if(isServing == 9) {
                                isServing = 1;
                                break;
                            }
                            if(connections[isServing + 1] != -1) {
                                isServing++;
                                break;
                            }
                            isServing++;
                        }
                        login = 0;
                        if(connections[isServing] != -1) status_value = send(connections[isServing], "serve",  100, 0);
                    } 
                    if (return_value1) {
                        
                        if(!strcmp(command, "register")) {
                            if(login) continue;
                            return_value2 = recv(connections[i], id, sizeof(id), 0);
                            return_value3 = recv(connections[i], password, sizeof(password), 0);
                            if(get_account(0, id, password)) {
                                status_value = send(connections[isServing], "user_exists", 100, 0);
                            } else {
                                login = 1;
                                append_account_to_akun_txt(id, password);
                                status_value = send(connections[isServing], "account_get", 100, 0);
                                sprintf(userdata, "%s:%s", id, password);
                            }
                        } else if(!strcmp(command, "login")) {
                            if(login) {
                                printf("\e[32mYou are already logged in.\n");
                                continue;
                            }
                            return_value2 = recv(connections[i], id, sizeof(id), 0);
                            return_value3 = recv(connections[i], password, sizeof(password), 0);
                            if(!get_account(1, id, password))
                                status_value = send(connections[isServing], "wrong_account", 100, 0);
							else {
                                login = 1;
                                status_value = send(connections[isServing], "account_get", 100, 0);
                            }
                        }
                        ...
}
```
**Client**

Pada fungsi `account_check`, client menerima pesan dari server dan meminta pengguna kembali login/register jika menerima pesan `user_exists` atau `wrong_account`, dan mempersilakan pengguna mengakses aplikasi jika menerima pesan `account_get`.
```c
int account_check(int fd, char command[]){
    int return_val;
	char id[100], password[100];
	
    printf("Enter ID: "); scanf("%s", id);
    return_val = send(fd, id, sizeof(id), 0);
    
    printf("Enter Password: "); scanf("%s", password);
    return_val = send(fd, password, sizeof(password), 0);
    //puts(id);
	
	char msg[100];
    return_val = recv(fd, msg, 100, 0);
    //puts(msg);

    if(!strcmp(msg, "account_get")) return 1;
    else if(!strcmp(msg, "user_exists")) {
    	printf("\e[31mID already exists.\e[0m\n");
        return 0;
    }
    else if(!strcmp(msg, "wrong_account")) {
    	printf("\e[31mID or password is incorrect.\e[0m\n");
        return 0;
    }
	
}
...
int main(){
    ...
    printf("\e[33mServer is now listening to your commands.\e[0m\n");
    
    int login=0, i;
    while(1){
    	while(!login) {
            printf("\e[32mInsert your option: login/register\n>\e[0m ");
            scanf("%s", command); getchar();
            
            for(i=0; i<strlen(command); i++) command[i] = tolower(command[i]);
            
            return_val = send(fd, command, sizeof(command), 0);
            if(!strcmp(command, "login") || !strcmp(command, "register")){
            	if(account_check(fd, command)) {
                	login = 1;
                	break;
				}
			} else {
                return_val = recv(fd, message, 100, 0);
                if(!strcmp(message, "not_logged_in\n")) printf("\e[31mPlease login/register first.\e[0m\n");
                else login = 1;
            }
        }
        ...
}
```

**Dokumentasi**

![Screenshot from 2021-05-22 16-38-04](https://user-images.githubusercontent.com/70105993/119225420-48445d00-bb36-11eb-8afe-8d8a70668a4d.png)


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
