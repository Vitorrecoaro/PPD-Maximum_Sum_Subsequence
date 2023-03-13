
#include<stdio.h>
#include<stdlib.h> 
#define MAX(a,b) a > b ? a : b

int MaxIncreasingSub(int *arr, int n, int k) 
{
	int **dp, ans = -1;
	dp = malloc(n * sizeof(int*));

	int i, l, j;

	#pragma omp parallel for
	for(i = 0; i < n; i++)
		dp[i] = malloc((k+1) * sizeof(int));

	#pragma omp parallel for collapse(2)
	for(i = 0; i < n; i++){
		for(j = 0; j < k; j++){
			dp[i][j] = -1;
		}
	}

	#pragma omp parallel for
	for (i = 0; i < n; i++) { 
		dp[i][1] = arr[i]; 
	}

	for (int i = 1; i < k; i++) { 
		#pragma omp parallel for collapse(1) private(l)
		for (int j = i; j < n; j++) { 
			for (int l = 1; l < j; l++) { 
				if (arr[l] < arr[j]) { 
					if (dp[l][i] != -1) { 
						dp[j][i + 1] = MAX(dp[j][i + 1],dp[l][i] + arr[j]); 
					} 
				} 
			}
		} 
	}  

	# pragma omp parallel for reduction(max : ans)
	for (i = 0; i < n; i++) { 
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
