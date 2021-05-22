#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h> 
#include <unistd.h> 
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

// Path bertujuan memudahkan testing, bisa diganti sesuai arahan pada soal
#define S_PATH "/home/farhan/Sisop/Modul3/soal-shift-sisop-modul-3-F12-2021/soal1/Server/FILES/"

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

void delete_cmd(int client, char userdata[128]) {
    char filename[128], new_path[256], old_path[256];
    int return_client= recv(client, filename, sizeof(filename), 0), isFound=0;
    
    find_in_tsv(&isFound, filename);
    if(isFound) {
        return_client = send(client, "OK", 100, 0);
        sprintf(new_path, "%sold-%s", S_PATH, filename);
        sprintf(old_path, "%s%s", S_PATH, filename);
        rename(old_path, new_path);
    } else return_client = send(client, "404", 100, 0);

    FILE *log = fopen("running.log", "a");
    fprintf(log, "Hapus : %s (%s)\n", filename, userdata);
    fclose(log);
}

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
                        } else {
                            if(login) {
                                printf("User access is granted\n");
                                if(!strcmp(command, "add")) add_cmd(connections[isServing], userdata);
                                if(!strcmp(command, "download")) download_cmd(connections[isServing]);
                                if(!strcmp(command, "delete")) delete_cmd(connections[isServing], userdata);
                                if(!strcmp(command, "see")) see_cmd(connections[isServing]);
                                if(!strcmp(command, "find")) find_cmd(connections[isServing]);
                            } else {
                                status_value = send(connections[isServing], "not_logged_in", 100, 0);
                                continue;
                            }
                        }
                        
                        printf("ID:Password :: %s:%s\n\n", id, password);
                        fflush(stdout);
                    }
                    if (return_value1 == -1 || return_value2 == -1 || return_value3 == -1) {
                        printf("Receiving failed for fd: %d\n", connections[i]);
                        break;
                    }
                }
                return_value1--;
                if (!return_value1) continue;
                
            }
		}
    }

    for(i=0; i<10;i++) if (connections[i]) close(connections[i]);
    return 0;
}
