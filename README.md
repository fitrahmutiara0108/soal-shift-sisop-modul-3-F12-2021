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

### Poin (a) dan (b)
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
}
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

![Screenshot from 2021-05-22 16-27-42](https://user-images.githubusercontent.com/70105993/119225500-b12bd500-bb36-11eb-9ff3-7a40a991d83f.png)

### Poin (c) dan (h)
Ketika server menerima command `add` dari client, server akan menambahkan file yang dikirim client ke dalam folder **FILES**, dan menambah baris baru pada file **files.tsv** sesuai data file yang dikirim client.

**Server**

- Pada fungsi `add_cmd`, server menerima data berupa publisher, tahun publikasi, dan path dari file (buku) yang dikirim client, kemudian dicatat pada file **files.tsv**. File kemudian diterima per baris pada server hingga akhir file, dan setelah file diterima, informasi penambahan file dicatat pada file **running.log**. 
- Fungsi `get_file_name` digunakan untuk mendapatkan file name dari file path yang dikirim client untuk digunakan dalam pencatatan ke file **running.log**. String file path di-traverse dari paling kanan hingga menemukan `/` untuk mengambil file name-nya saja, kemudian karena urutan traversal terbalik (dari kanan ke kiri) maka untuk menghasilkan file name yang tepat, string yang menyimpan file name harus dibalik (`strrev`) setelah traversal selesai. Pada sistem operasi Linux, fungsi `strrev()` tidak dapat dikenali meskipun sudah menggunakan header sehingga harus didefinisikan secara manual seperti yang tertulis di bawah.
- Setelah proses penambahan file selesai dan lognya tercatat, program server akan mencetak baris `ID:Password :: [id pengguna]:[password pengguna]`. 
```c
...
char *strrev(char *str) {
    char *p1, *p2;

    if (!str || ! *str) return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2; *p2 ^= *p1;  *p1 ^= *p2;
    }
    return str;
}

void get_file_name(char filepath[], char filename[]) {
    int i = strlen(filepath) - 1, j = 0;
    while(i) {
        filepath[i+1] = '\0';
        if(filepath[i] == '/') break;
        filename[j++] = filepath[i--];
    }
    filename[j] = '\0';
    strrev(filename);
}
...
void add_cmd(int client, char userdata[128]) {
    char fname[100], publisher[100], tahun[10], fp[100], message[100], fullpath[256], file_data[4096];
    int return_publisher, return_tahun, return_filepath, return_stat, return_receive;
    return_publisher = recv(client, publisher, sizeof(publisher), 0);
    return_tahun = recv(client, tahun, sizeof(tahun), 0);
    return_filepath = recv(client, fp, sizeof(fp), 0);

    get_file_name(fp, fname);
    sprintf(fullpath, "%s%s", S_PATH, fname);
    
    //printf("%s, %s, %s\n", publisher, tahun, fullpath);

    FILE *files_tsv = fopen("files.tsv", "a");
    fprintf(files_tsv, "%s\t%s\t%s\n", publisher, tahun, fullpath);
    fclose(files_tsv);

    FILE *file = fopen(fullpath, "w+");
    while(1) {
        return_receive = recv(client, file_data, 4096, 0);
        //printf("Data: %s", file_data);
    	fflush(stdout);
        if(return_receive != -1){
            if(!strcmp(file_data, "OK")) break;
		}
            
        fprintf(file, "%s", file_data);
        bzero(file_data, 4096);
    }
    fclose(file);

    FILE *log = fopen("running.log", "a");
    fprintf(log, "Tambah : %s (%s)\n", fname, userdata);
    fclose(log);
    fflush(stdout);
}
...
int main () {
   ...
            for (i=1; i<10; i++) {
                if ((connections[i]>0) && (FD_ISSET(connections[i], &read_fd_set))) {
                    return_value1 = recv(connections[i], command, sizeof(command), 0);
                    ...
                    if (return_value1) {
                        ...
			else {
                            if(login) {
                                printf("User access is granted\n");
                                if(!strcmp(command, "add")) add_cmd(connections[isServing], userdata);
                                ...
                        }
                        
                        printf("ID:Password :: %s:%s\n\n", id, password);
                      	fflush(stdout);
			...
}
```

**Client**

Pada fungsi `add_book`, client mengirim data publisher, tahun publikasi, dan file dari path yang dimasukkan. Input data pada client menggunakan `fgets` untuk memungkinkan adanya spasi pada publisher atau file path, kemudian karakter `\n` (newline) pada akhir string dihilangkan.
```c
void add_book(int fd){
    char publisher[100], tahun[10], filepath[128];
    
    printf("Publisher: "); fgets(publisher, sizeof(publisher), stdin);
    publisher[strcspn(publisher, "\n")] = 0;
    
    printf("Tahun Publikasi: "); fgets(tahun, sizeof(tahun), stdin);
    tahun[strcspn(tahun, "\n")] = 0;
    
    printf("Filepath: "); fgets(filepath, sizeof(filepath), stdin);
    filepath[strcspn(filepath, "\n")] = 0;

    int return_val;
    return_val = send(fd, publisher, sizeof(publisher), 0);
    return_val = send(fd, tahun, sizeof(tahun), 0);
    return_val = send(fd, filepath, sizeof(filepath), 0);

    FILE *file = fopen(filepath, "r");
    char file_data[4096] = {0};

    while(fgets(file_data, 4096, file)) {
        if(send(fd, file_data, sizeof(file_data), 0) != -1) bzero(file_data, 4096);
    }
    
    fclose(file);
    printf("\e[32mSuccessfully added file.\e[0m\n");
    send(fd, "OK", 4096, 0);
}
...
int main(){
    	...
        while(1){
            printf("\e[32mPlease input the operation you would like to do: add/download/delete/see/find\n>\e[0m ");
            scanf("%s", command); getchar();
            
            for(i=0; i<strlen(command); i++) command[i] = tolower(command[i]);
            
            return_val = send(fd, command, sizeof(command), 0);
            if(!strcmp(command, "login") || !strcmp(command, "register"))  printf("\e[32mYou are already logged in.\e[0m\n");
            if(!strcmp(command, "add")) add_book(fd);
            ...
        }

        sleep(2);
        if(login) break;
	}
	...
}
```

**Dokumentasi**

- Penambahan total 5 file .txt sesuai arahan pada soal

![Screenshot from 2021-05-22 18-12-29](https://user-images.githubusercontent.com/70105993/119227008-834a8e80-bb3e-11eb-8577-1d22681dc53b.png)
![Screenshot from 2021-05-22 18-12-40](https://user-images.githubusercontent.com/70105993/119227010-847bbb80-bb3e-11eb-8c53-163754ee439a.png)

- Bukti bahwa file pada path asli yang dikirim client (kiri dan pojok kanan atas, path `~`) sama dengan file yang diterima server di folder **FILES**

![Screenshot from 2021-05-22 18-11-45](https://user-images.githubusercontent.com/70105993/119227006-8180cb00-bb3e-11eb-8649-b7c0265ba9ae.png)

### Poin (d)
Ketika server menerima command `download` dari client, server akan mengirim file dalam folder **FILES** menuju client sesuai file name yang dimasukkan client, jika baris informasi dari file dengan file name yang sama dengan yang dimasukkan client terdapat dalam **files.tsv**.

**Server**

Pada fungsi `download_cmd`, server menerima nama file yang ingin didownload dari client, membuat string path file tersebut pada server, kemudian mengecek apakah path tersebut ada pada **files.tsv** atau tidak melalui fungsi `find_file`. Jika ada, maka server membuat file tersebut di folder **FILES**, mengisi file tersebut dengan data dari file aslinya dengan command `fgets`, dan mengirim pesan `OK` ke client. Jika tidak ada, maka server mengirim pesan '404' ke client. Setelah proses download file selesai, program server akan mencetak baris `ID:Password :: [id pengguna]:[password pengguna]`. 
```c
int find_file(char filename[]) {
    FILE *tsv = fopen("files.tsv", "r");
    char check[256];
    
    while(fscanf(tsv,"%s", check) == 1){
        if(strstr(check, filename)!=0) {
                fclose(tsv);
                return 1;
        }
    }
    fclose(tsv);
    return 0;
}
...
void download_cmd(int client) {
    char fpath[512];
    char fname[256];

    int return_fn = recv(client, fname, sizeof(fname), 0);

    sprintf(fpath, "%s%s", S_PATH, fname);
    printf("%s\n", fpath);

    if(find_file(fname)) {
        FILE *book = fopen(fpath, "r");
        char file_data[4096] = {0};

        while(fgets(file_data, 4096, book) != NULL) {
            if(send(client, file_data, sizeof(file_data), 0) != -1)  bzero(file_data, 4096);
        }
        fclose(book);
        printf("\e[32mFile sent successfully.\e[0m\n");
        send(client, "OK", 4096, 0);
    }
	else send(client, "404", 4096, 0);
}
...
int main () {
   ...
            for (i=1; i<10; i++) {
                if ((connections[i]>0) && (FD_ISSET(connections[i], &read_fd_set))) {
                    return_value1 = recv(connections[i], command, sizeof(command), 0);
                    ...
                    if (return_value1) {
                        ...
			else {
                            if(login) {
                                printf("User access is granted\n");
                                ...
				if(!strcmp(command, "download")) download_cmd(connections[isServing]);
                                ...
                        }
                        
                        printf("ID:Password :: %s:%s\n\n", id, password);
                      	fflush(stdout);
			...
}
```

**Client**

Pada fungsi `download_book`, client memasukkan nama file yang ingin didownload, menerima file dari server jika file dengan nama tersebut ada pada server dan menampilkan pesan download sukses jika pesan yang diterima adalah `OK`, serta menampilkan pesan download gagal jika pesan yang diterima adalah `404`.
```c
void download_book(int fd) {
    int return_val, return_rec;
    char filename[100], filePath[500]={0}, file_data[4096];
    printf("\e[0mInput file name\n> \e[36m");
    fgets(filename, sizeof(filename), stdin);
    printf("\e[0m");
    
    filename[strcspn(filename, "\n")] = 0;
    return_val = send(fd, filename, sizeof(filename), 0);

    sprintf(filePath, "%s%s", "/home/farhan/Sisop/Modul3/soal-shift-sisop-modul-3-F12-2021/soal1/Client/", filename);
    while(1) {
        if(recv(fd, file_data, sizeof(file_data), 0) != -1) {
            if(!strcmp(file_data, "404")) {
                printf("\e[31mFile not found.\e[0m\n");
                return;
            }
            if(!strcmp(file_data, "OK")) {
                printf("\e[32mSuccessfully downloaded file.\e[0m\n");
                return;
            }
            
            FILE *file = fopen(filePath, "a");
            fprintf(file, "%s", file_data);
            bzero(file_data, 4096);
            fclose(file);
        }
    }
}
...
int main(){
    	...
        while(1){
            printf("\e[32mPlease input the operation you would like to do: add/download/delete/see/find\n>\e[0m ");
            scanf("%s", command); getchar();
            
            for(i=0; i<strlen(command); i++) command[i] = tolower(command[i]);
            
            return_val = send(fd, command, sizeof(command), 0);
            ...
            if(!strcmp(command, "download")) download_book(fd);
	    ...
        }

        sleep(2);
        if(login) break;
	}
	...
}
```

**Dokumentasi**

![Screenshot from 2021-05-22 20-55-39](https://user-images.githubusercontent.com/70105993/119227412-4b444b00-bb40-11eb-9284-dd1381835373.png)

### Poin (e) dan (h)
Ketika server menerima command `delete` dari client, server akan mengganti nama file yang dimasukkan client menjadi `old-NamaFile.ekstensi`, dan menghapus baris informasi file tersebut pada **files.tsv** jika ada.

**Server**

- Pada fungsi `delete_cmd`, server terlebih dahulu mencari nama file yang dimasukkan client di file **files.tsv**. Tiap baris pada **files.tsv** dimasukkan ke string `tmp` dan dicocokkan dengan nama file yang dimasukkan client. Jika sama, maka baris tidak dimasukkan pada file **temp.tsv** yang nantinya akan dijadikan file **files.tsv** baru setelah penghapusan selesai sehingga baris informasi file tersebut tidak ada lagi, dan mengubah flag `isFound` menjadi 1. Jika tidak sama, maka baris dimasukkan pada **temp.tsv**. Setelah penghapusan selesai, **files.tsv** dihapus dan **temp.tsv** di-rename menjadi **files.tsv**.
- Pada fungsi `delete_cmd`, jika `isFound = 1` (artinya ada file yang dapat dihapus), maka pertama-tama server akan mengirim pesan `OK` ke client tanda nama file yang dimasukkan dapat dihapus. Nama file tersebut pada folder **FILES** diubah menjadi `old-NamaFile.ekstensi`, dan log penghapusan dicatat pada **running.log**. Jika `isFound = 0` maka tidak ada file yang dihapus, dan server mengirim pesan `404` ke client.
- Setelah proses penghapusan file selesai dan lognya tercatat, program server akan mencetak baris `ID:Password :: [id pengguna]:[password pengguna]`. 
```c
int find_in_tsv(int *isFound, char filename[]) {
    FILE *files_tsv = fopen("files.tsv", "r+");
    FILE *temp_tsv = fopen("temp.tsv", "w+");
    char tmp[256], row[256];
    
    while(fgets(row, 256, files_tsv) != 0){
        if(sscanf(row, "%255[^\n]", tmp) != 1) break;
        if(strstr(tmp, filename) != 0)  *isFound = 1;
        else fprintf(temp_tsv, "%s\n", tmp);
    }
    remove("files.tsv");
    rename("temp.tsv", "files.tsv");

    fclose(temp_tsv); fclose(files_tsv);
    return 0;
}
...
void delete_cmd(int client, char userdata[128]) {
    char filename[128], new_path[256], old_path[256];
    int return_client= recv(client, filename, sizeof(filename), 0), isFound=0;
    
    find_in_tsv(&isFound, filename);
    if(isFound) {
        return_client = send(client, "OK", 100, 0);
        sprintf(new_path, "%sold-%s", S_PATH, filename);
        sprintf(old_path, "%s%s", S_PATH, filename);
        rename(old_path, new_path);

        FILE *log = fopen("running.log", "a");
        fprintf(log, "Hapus : %s (%s)\n", filename, userdata);
        fclose(log);
    } else return_client = send(client, "404", 100, 0);
}
...
int main () {
   ...
            for (i=1; i<10; i++) {
                if ((connections[i]>0) && (FD_ISSET(connections[i], &read_fd_set))) {
                    return_value1 = recv(connections[i], command, sizeof(command), 0);
                    ...
                    if (return_value1) {
                        ...
			else {
                            if(login) {
                                printf("User access is granted\n");
                                ...
				if(!strcmp(command, "delete")) delete_cmd(connections[isServing], userdata);
				...
                        }
                        
                        printf("ID:Password :: %s:%s\n\n", id, password);
                      	fflush(stdout);
			...
}
```

**Client**

Pada fungsi `delete_book`, client mengirim nama file yang akan dihapus, kemudian menampilkan pesan penghapusan sukses jika pesan yang diterima dari server adalah `OK`, dan menampilkan pesan penghapusan gagal jika pesan yang diterima adalah `404`.
```c
void delete_book(int fd) {
    int return_value;
    char filename[100], msg[100];

	printf("\e[0mInput file name\n> \e[36m");
    fgets(filename, sizeof(filename), stdin);
    printf("\e[0m");
    
    filename[strcspn(filename, "\n")] = 0;

    return_value = send(fd, filename, sizeof(filename), 0);
    return_value = recv(fd, msg, 100, 0);
    
    if(!strcmp(msg, "OK")) printf("\e[32mSuccessfully deleted file.\e[0m\n");
    if(!strcmp(msg, "404")) printf("\e[31mDeletion error, file not found.\e[0m\n");
}
...
int main(){
    	...
        while(1){
            printf("\e[32mPlease input the operation you would like to do: add/download/delete/see/find\n>\e[0m ");
            scanf("%s", command); getchar();
            
            for(i=0; i<strlen(command); i++) command[i] = tolower(command[i]);
            
            return_val = send(fd, command, sizeof(command), 0);
            ...
	    if(!strcmp(command, "delete")) delete_book(fd);
	    ...
        }

        sleep(2);
        if(login) break;
	}
	...
}
```

**Dokumentasi**

![Screenshot from 2021-05-22 18-14-22](https://user-images.githubusercontent.com/70105993/119227190-53e85180-bb3f-11eb-8d04-4db3eaab4475.png)

### Poin (f)
Ketika server menerima command `see` dari client, server akan menampilkan seluruh isi **files.tsv** sesuai format.

**Server**

Pada fungsi `see_cmd`, bila baris pada **files.tsv** yang sedang diiterasi (kecuali header) tidak kosong, maka informasi tiap-tiap baris dikirim ke client dengan terlebih dahulu diformat. `strtok` mengambil informasi publisher, tahun, dan file path pada server dengan delimiter `\t` (tab), dan mengambil informasi file name serta ekstensinya dengan delimiter `.` dari file path, dengan terlebih dahulu mendapatkan file name dari file path pada server melalui fungsi `get_file_name` (seperti pada poin c), dan kemudian dikirimkan ke client. Setelah proses output selesai, program server akan mencetak baris `ID:Password :: [id pengguna]:[password pengguna]`. 
```c
void see_cmd(int client) {
    FILE *tsv_file = fopen("files.tsv", "r");
    char file_data[1024], file[100], filename[64], publisher[64], tahun[64], ext[64], filepath[256],
		filename_send[1024], publisher_send[1024], tahun_send[1024], ext_send[1024], filepath_send[1024], *p;
    int i=0, ret_c;
    //printf("%s\n\n", file_data);
    
    while(fgets(file_data, 1024, tsv_file) != NULL) {
        if(i != 0) {
            strcpy(publisher, strtok_r(file_data, "\t", &p));
            strcpy(tahun, strtok_r(NULL, "\t", &p));
            strcpy(filepath, strtok_r(NULL, "\t", &p));
            filepath[strlen(filepath)-1] = '\0';
            sprintf(filepath_send, "Filepath: %s\n\n", filepath);

            get_file_name(filepath, file);

            strcpy(filename, strtok_r(file, ".", &p));
            strcpy(ext, strtok_r(NULL, ".", &p));

            sprintf(filename_send, "Nama: %s\n", filename);
            sprintf(publisher_send, "Publisher : %s\n", publisher);
            sprintf(tahun_send, "Tahun publishing: %s\n", tahun);
            sprintf(ext_send, "Ekstensi File: %s\n", ext);
			
	    send(client, "next", 1024, 0);
            send(client, filename_send, 1024, 0);
            send(client, publisher_send, 1024, 0);
            send(client, tahun_send, 1024, 0);
            send(client, ext_send, 1024, 0);
            send(client, filepath_send, 1024, 0);
            printf("%s\n%s\n%s\n%s\n%s\n", filename, publisher, tahun, ext, filepath);
            sleep(1);
        }
        i++;
        bzero(file_data, sizeof(file_data));
    }
    fclose(tsv_file);
	ret_c = send(client, "OK", 1024, 0);
	fflush(stdout);
}
...
int main () {
   ...
            for (i=1; i<10; i++) {
                if ((connections[i]>0) && (FD_ISSET(connections[i], &read_fd_set))) {
                    return_value1 = recv(connections[i], command, sizeof(command), 0);
                    ...
                    if (return_value1) {
                        ...
			else {
                            if(login) {
                                printf("User access is granted\n");
                                ...
				if(!strcmp(command, "see")) see_cmd(connections[isServing]);
				...
                        }
                        
                        printf("ID:Password :: %s:%s\n\n", id, password);
                      	fflush(stdout);
			...
}
```

**Client**

Pada fungsi `see_book`, client menerima data seluruh isi **files.tsv** dari server hingga menerima pesan `OK` yang menandakan seluruh data pada **files.tsv** telah dikirim oleh server.
```c
void see_book(int fd) {
    int return_value;
    char filename[1024], publisher[1024], tahun[1024], ext[1024], filepath[1024], msg[1024];

    while(1){
    	if(recv(fd, msg, sizeof(msg), 0) != -1){
    		if(!strcmp(msg, "OK")) break;
		}
        return_value = recv(fd, filename, 1024, 0);
        printf("%s", filename);
        return_value = recv(fd, publisher, 1024, 0);
        printf("%s", publisher);
        return_value = recv(fd, tahun, 1024, 0);
        printf("%s", tahun);
        return_value = recv(fd, ext, 1024, 0);
        printf("%s", ext);
        return_value = recv(fd, filepath, 1024, 0);
        printf("%s", filepath);
    }
}
...
int main(){
    	...
        while(1){
            printf("\e[32mPlease input the operation you would like to do: add/download/delete/see/find\n>\e[0m ");
            scanf("%s", command); getchar();
            
            for(i=0; i<strlen(command); i++) command[i] = tolower(command[i]);
            
            return_val = send(fd, command, sizeof(command), 0);
            ...
	    if(!strcmp(command, "see")) see_book(fd);
	    ...
        }

        sleep(2);
        if(login) break;
	}
	...
}
```

**Dokumentasi**

![Screenshot from 2021-05-22 18-12-29](https://user-images.githubusercontent.com/70105993/119227008-834a8e80-bb3e-11eb-8577-1d22681dc53b.png)
![Screenshot from 2021-05-22 18-12-40](https://user-images.githubusercontent.com/70105993/119227010-847bbb80-bb3e-11eb-8c53-163754ee439a.png)
![Screenshot from 2021-05-22 18-14-22](https://user-images.githubusercontent.com/70105993/119227190-53e85180-bb3f-11eb-8d04-4db3eaab4475.png)


### Poin (g)
Ketika server menerima command `find` dan kueri pencarian dari client, server akan menampilkan informasi file dari **files.txt** untuk seluruh file yang dalam namanya terkandung string kueri pencarian dari client.

**Server**

Pada fungsi `find_cmd`, server menerima kueri pencarian dari client, dan bila baris pada **files.tsv** yang sedang diiterasi (kecuali header) tidak kosong, maka informasi tiap-tiap baris terlebih dahulu diformat dan dicocokkan sebelum dikirim. `strtok` mengambil informasi publisher, tahun, dan file path pada server dengan delimiter `\t` (tab), dan mengambil informasi file name serta ekstensinya dengan delimiter `.` dari file path, dengan terlebih dahulu mendapatkan file name dari file path pada server melalui fungsi `get_file_name` (seperti pada poin c), dan jika filename mengandung string kueri, maka kemudian data yang telah diformat dikirimkan ke client. Setelah proses output selesai, program server akan mencetak baris `ID:Password :: [id pengguna]:[password pengguna]`. 
```c
void find_cmd(int client) {
    FILE *tsv_file = fopen("files.tsv", "r");
    char query[1024], file_data[1024], file[100], filename[64], publisher[64], tahun[64], ext[64], filepath[256],
		filename_send[1024], publisher_send[1024], tahun_send[1024], ext_send[1024], filepath_send[1024], *p;
    int i=0, ret_c;
    
    ret_c = recv(client, query, 1024, 0);
    
    while(fgets(file_data, 1024, tsv_file) != NULL) {
        if(i != 0) {
            strcpy(publisher, strtok_r(file_data, "\t", &p));
            strcpy(tahun, strtok_r(NULL, "\t", &p));
            strcpy(filepath, strtok_r(NULL, "\t", &p));
            filepath[strlen(filepath)-1] = '\0';
            sprintf(filepath_send, "Filepath: %s\n\n", filepath);

            get_file_name(filepath, file);

            strcpy(filename, strtok_r(file, ".", &p));
            strcpy(ext, strtok_r(NULL, ".", &p));
            
			if(strstr(file, query)) {
	            sprintf(filename_send, "Nama: %s\n", filename);
	            sprintf(publisher_send, "Publisher : %s\n", publisher);
	            sprintf(tahun_send, "Tahun publishing: %s\n", tahun);
	            sprintf(ext_send, "Ekstensi File: %s\n", ext);
				
				send(client, "next", 1024, 0);
	            send(client, filename_send, 1024, 0);
	            send(client, publisher_send, 1024, 0);
	            send(client, tahun_send, 1024, 0);
	            send(client, ext_send, 1024, 0);
	            send(client, filepath_send, 1024, 0);
	            printf("%s\n%s\n%s\n%s\n%s\n", filename, publisher, tahun, ext, filepath);
	            sleep(1);
	    	}
        }
        i++;
        bzero(file_data, sizeof(file_data));
    }
    fclose(tsv_file);
	ret_c = send(client, "OK", 1024, 0);
	fflush(stdout);
}
...
int main () {
   ...
            for (i=1; i<10; i++) {
                if ((connections[i]>0) && (FD_ISSET(connections[i], &read_fd_set))) {
                    return_value1 = recv(connections[i], command, sizeof(command), 0);
                    ...
                    if (return_value1) {
                        ...
			else {
                            if(login) {
                                printf("User access is granted\n");
                                ...
				if(!strcmp(command, "find")) find_cmd(connections[isServing]);
				...
                        }
                        
                        printf("ID:Password :: %s:%s\n\n", id, password);
                      	fflush(stdout);
			...
}
```

**Client**

Pada fungsi `find_book`, client mengirim kueri pencarian ke server, dan menerima informasi file dari **files.tsv** yang ditemukan sesuai kueri pencarian dari server hingga menerima pesan `OK` yang menandakan seluruh data pada **files.tsv** telah dikirim oleh server. Maka, jika tidak ditemukan file yang namanya mengandung string kueri pencarian, maka client tidak menampilkan data apapun.
```c
void find_book(int fd) {
    int return_value;
    char query[1024], filename[1024], publisher[1024], tahun[1024], ext[1024], filepath[1024], msg[1024];
	
	printf("\e[0mInput search query\n> \e[36m");
    fgets(query, sizeof(query), stdin);
    printf("\e[0m");
    
    query[strcspn(query, "\n")] = 0;
    return_value = send(fd, query, sizeof(query), 0);
    
    while(1){
    	if(recv(fd, msg, sizeof(msg), 0) != -1){
    		if(!strcmp(msg, "OK")) break;
		}
        return_value = recv(fd, filename, 1024, 0);
        printf("%s", filename);
        return_value = recv(fd, publisher, 1024, 0);
        printf("%s", publisher);
        return_value = recv(fd, tahun, 1024, 0);
        printf("%s", tahun);
        return_value = recv(fd, ext, 1024, 0);
        printf("%s", ext);
        return_value = recv(fd, filepath, 1024, 0);
        printf("%s", filepath);
    }
}
...
int main(){
    	...
        while(1){
            printf("\e[32mPlease input the operation you would like to do: add/download/delete/see/find\n>\e[0m ");
            scanf("%s", command); getchar();
            
            for(i=0; i<strlen(command); i++) command[i] = tolower(command[i]);
            
            return_val = send(fd, command, sizeof(command), 0);
            ...
	    if(!strcmp(command, "find")) find_book(fd);
        }

        sleep(2);
        if(login) break;
	}
	...
}
```

**Dokumentasi**

![Screenshot from 2021-05-22 18-26-36](https://user-images.githubusercontent.com/70105993/119228417-86954880-bb45-11eb-874f-743b656bf7b5.png)

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
