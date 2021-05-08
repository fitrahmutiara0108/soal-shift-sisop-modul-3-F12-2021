#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define M1_baris 4
#define M1_kolom 3
#define M2_baris 3
#define M2_kolom 6

int main() {

    int i,j,k;
    int A[M1_baris][M1_kolom];
    int B[M2_baris][M2_kolom];

    printf("Input matrix 4x3\n");
	for (i = 0; i < M1_baris; i++) {
    	for (j = 0; j < M1_kolom; j++) {
      		scanf("%d", &A[i][j]);
   	 	}
  	}
  	printf("Input matrix 3x6\n");
	for (i = 0; i < M2_baris; i++) {
    	for (j = 0; j < M2_kolom; j++) {
      		scanf("%d", &B[i][j]);
   	 	}
  	}

	key_t key = 1010;
  	int (*hasil)[M2_kolom];

  	int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
  	hasil = shmat(shmid, NULL, 0);

  	printf("Hasil Perkalian Matriks \n");
  	for (i = 0; i < M1_baris; i++) {
    	for (j = 0; j < M2_kolom; j++) {
    		for (k = 0; k< 3; k++){
      			hasil[i][j] += A[i][k] * B[k][j];
			}
      		printf("%d ", hasil[i][j]);
   	 	}
    	printf("\n");
  	}
  	shmdt(hasil);
  	return 0;
}
