#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

long long N, P;
long long sum = 0;
pthread_mutex_t lock;

typedef struct
{
	long long start;
	long long end;
	long long limit;
} interval;

void *find_factors(void *arg)
{
	interval *range = (interval *)arg;
	long long local_sum = 0;
	long long limit = range->limit;
	for (long long i = range->start; i <= range->end && i <= limit; i++)
	{
		if (N % i == 0)
		{
			local_sum += i;
			long long pair = N / i;
			if (pair != i && pair != N)
				local_sum += pair;
		}
	}
	pthread_mutex_lock(&lock);
	sum += local_sum;
	pthread_mutex_unlock(&lock);
	free(range);
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
		return 1;
	N = atoll(argv[1]);
	P = atoll(argv[2]);
	if (N <= 1 || P <= 0)
		return 1;
	pthread_mutex_init(&lock, NULL);
	long long limit = (long long)sqrt((double)N);
	if (P > limit)
		P = limit;
	long long quotient = limit / P;
	long long remainder = limit % P;
	pthread_t threads[P];
	long long current = 1;
	for (long long i = 0; i < P; i++)
	{
		interval *range = malloc(sizeof(interval));
		range->start = current;
		range->end = current + quotient - 1 + (i < remainder ? 1 : 0);
		range->limit = limit;
		current = range->end + 1;
		pthread_create(&threads[i], NULL, find_factors, range);
	}
	for (long long i = 0; i < P; i++)
		pthread_join(threads[i], NULL);
	pthread_mutex_destroy(&lock);
	sum == N ? printf("Perfect Number\n") : printf("Not a perfect number\n");
	return 0;
}

