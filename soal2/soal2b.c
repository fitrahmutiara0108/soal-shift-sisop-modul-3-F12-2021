#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define baris 4
#define kolom 6

typedef long long ll;
int A[baris][kolom], selisih, i, j, isZero = 0;

ll factorial(int n) {
    if (n == 0)
        return 1;
    else
        return n * factorial(n - 1);
}

ll diff_f(int n){
	if (n == selisih) return 1;
        else return n * diff_f(n - 1);
}

void *kondisi(void* arg){
	ll angka = *(ll*)arg;
	if(isZero) printf("0 "); //0
	else if(selisih<1) printf("%lld ", factorial(angka)); //<1
	else printf("%lld ", diff_f(angka)); //>1
}

int main(){
	key_t key = 1010;
    int (*value)[kolom];
    int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
    value = shmat(shmid, NULL, 0);

    printf("Input matriks 4x6\n");
	for (i=0; i<baris; i++) {
    	for (j=0; j<kolom; j++) scanf("%d", &A[i][j]);
  	}

    pthread_t tid[baris*kolom];

    int index=0;
    for(i = 0; i < baris; i++){
        for(j = 0; j < kolom; j++){
        	isZero=0;
            ll *val = malloc(sizeof(ll[4][6]));

            *val = value[i][j];
            selisih = value[i][j] - A[i][j];

            if(value[i][j]==0 || A[i][j]==0) isZero=1;

            pthread_create(&tid[index], NULL, &kondisi, val);
            sleep(1);
            index++;
        }
        printf("\n");
    }
    for (i = 0; i<index; i++)
        pthread_join(tid[i], NULL);

    shmdt(value);
    shmctl(shmid, IPC_RMID, NULL);

	return 0;
}