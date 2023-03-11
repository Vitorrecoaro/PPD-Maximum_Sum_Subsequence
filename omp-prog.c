
#include<stdio.h>
#include<stdlib.h> 
#define MAX(a,b) a > b ? a : b

int MaxIncreasingSub(int *arr, int n, int k) 
{
	int **dp, ans = -1;
	dp = malloc(n * sizeof(int*));

	#pragma omp parallel
	{
		int j;

		#pragma omp for
		for(int i = 0; i < n; i++)
			dp[i] = malloc((k+1) * sizeof(int));

		#pragma omp for private(j)
		for(int i = 0; i < n; i++){
			for(j = 0; j < k; j++){
				dp[i][j] = -1;
			}
		}

		#pragma omp for
		for (int i = 0; i < n; i++) { 
			dp[i][1] = arr[i]; 
		} 
	}

	#pragma omp parallel
	{
		int j, l;

		#pragma omp for private(j,l)
		for (int i = 1; i < n; i++) { 
			for ( j = 0; j < i; j++) {
				if (arr[j] < arr[i]) {
					for (l = 1; l <= k - 1; l++) { 
						if (dp[j][l] != -1) { 
							dp[i][l + 1] = MAX(dp[i][l + 1],dp[j][l] + arr[i]); 
						} 
					} 
				}
			} 
		} 
	}

	# pragma omp parallel for reduction(max : ans)
	for (int i = 0; i < n; i++) { 
		if (ans < dp[i][k]) 
			ans = dp[i][k]; 
	}

	return (ans == -1) ? 0 : ans; 
} 

int main() 
{ 
	int n, k;
    scanf(" %d", &n);
    scanf(" %d", &k);
    int *arr = malloc(n * sizeof(int));
	for (int i = 0; i<n; i++)
		scanf(" %d", &arr[i]);
	int ans = MaxIncreasingSub(arr, n, k); 
	printf("%d\n", ans); 
	return 0;
} 
