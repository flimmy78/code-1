#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define LIMIT_CASH 1000

#define CASH_TYPE  5

int RMB[] = {100, 50, 20, 10, 5};
int success;

int sample(void)
{
	success = 0;
	int i,j,k,x,y;
	for(i = 1; i < (LIMIT_CASH/RMB[0] + 1); i++)
		for(j = 1; j < (LIMIT_CASH/RMB[1] + 1); j++)
			for(k = 1; k < (LIMIT_CASH/RMB[2] + 1); k++)
				for(x = 1; x < (LIMIT_CASH/RMB[3] + 1); x++)
					for(y = 1 ; y < (LIMIT_CASH/RMB[4] + 1); y++){
						if ( i * RMB[0] + j * RMB[1] + k * RMB[2] + x * RMB[3] + y * RMB[4] == LIMIT_CASH ){
							success++;
							/*
							printf("[100] %d\t", i);
							printf("[50] %d\t", j);
							printf("[20] %d\t", k);
							printf("[10] %d\t", x);
							printf("[5] %d\t", y);
							printf("\n");
							*/
						}
					}
}
int alog(int money, int *record, int x)
{
	int i, j;
	int count[CASH_TYPE];
	int total ;
	int makesure = 0;


	for(i = x; i < CASH_TYPE; i++){
		memcpy(count, record, sizeof(count));
		total = money + RMB[i];

		if (total < LIMIT_CASH && ( total + RMB[i] > LIMIT_CASH)){ // total < 500; total + RMB[i] > 500; grep next crash type, so x++;
			count[i] += 1;
			alog(total, count, i + 1);

		} else if ( total == LIMIT_CASH){
			success++;
			count[i] += 1;
			/*
			for(j = 0; j < CASH_TYPE; j++){
				printf("[%d] %d\t", RMB[j], count[j]);
			}
			printf("*\n");
			*/
			continue;
		} else if (total < LIMIT_CASH && (total + RMB[i] <= LIMIT_CASH)){
			count[i] += 1;
			alog(total, count, i);
		} else {
			continue;
		}
	}
	return 0;

}
int main()
{
	int count[CASH_TYPE] = {1, 1, 1, 1, 1};

	alog(185, count, 0);
    //sample();
	printf("success %d\n", success);
	return 0;
}
