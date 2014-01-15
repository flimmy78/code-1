#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define LIMIT_CASH 500

#define CASH_TYPE  5

int RMB[] = {100, 50, 20, 10, 5, 2 , 1};
int success;

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
				printf("[%d] %d\n", RMB[j], count[j]);
				makesure += RMB[j] * count[j];
			}
			printf("total : %d\n", makesure);
			printf("================================================\n");
			*/
			continue;
		} else if (total < LIMIT_CASH && (total + RMB[i] <= LIMIT_CASH)){
			count[i] += 1;
			alog(total, count, i);
		} else {
			return 0;
		}
	}
	return 0;

}
int main()
{
	int count[CASH_TYPE] = {1, 1, 1, 1, 1};

	alog(185, count, 0);
	printf("success %d\n", success);
	return 0;
}
