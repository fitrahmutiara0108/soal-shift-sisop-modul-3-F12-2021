#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define row 4
#define column 6

typedef long long ll;
int A[row][column], diff, i, j, isZero = 0;

ll fact(int n) {
    if (n == 0) return 1;
    return n*fact(n-1);
}

ll diff_f(int n){
	if (n == diff) return 1;
        return n*diff_f(n-1);
}

void *print(void* argv){
	ll n = *(ll*)argv;
	if(isZero) printf("0 ");
	else if(diff<1) printf("%lld ", fact(n)); 
	else printf("%lld ", diff_f(n));
}

int main(){
	key_t key = 1010;
    int (*hasil)[column];
    int shmid = shmget(key, sizeof(int[4][6]), IPC_CREAT | 0666);
    hasil = shmat(shmid, NULL, 0);

    printf("Input matriks 4x6\n");
	for (i=0; i<row; i++) {
    	for (j=0; j<column; j++) scanf("%d", &A[i][j]);
  	}

    pthread_t thread_id[row*column];

    int count=0;
    for(i = 0; i < row; i++){
        for(j = 0; j < column; j++){
        	isZero=0;
            ll *val = malloc(sizeof(ll[4][6]));

            *val = hasil[i][j];
            diff = hasil[i][j] - A[i][j];

            if(hasil[i][j]==0 || A[i][j]==0) isZero=1;

            pthread_create(&thread_id[count], NULL, &print, val);
            sleep(1);
            count++;
        }
        printf("\n");
    }
    for (i = 0; i<count; i++) pthread_join(thread_id[i], NULL);

    shmdt(hasil);
    shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
