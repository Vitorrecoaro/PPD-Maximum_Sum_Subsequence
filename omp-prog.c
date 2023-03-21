#include <stdio.h>
#include <stdlib.h>
#define MAX(a, b) a > b ? a : b

int MaxIncreasingSub(int *arr, int n, int k)
{
	int *dp = (int *) malloc(n * k * sizeof(int));
	int ans = -1;

	int i, l, j;

#pragma omp parallel for
	for (i = 0; i < n * k; i++)
	{
		dp[i] = -1;
	}

#pragma omp parallel for
	for (i = 0; i < n; i++)
	{
		dp[i] = arr[i];
	}

	for (int i = 0; i < (k - 1); i++)
	{
#pragma omp parallel for private(l)
		for (int j = i + 1; j < n; j++)
		{
			for (int l = 0; l < j; l++)
			{
				if (arr[l] < arr[j])
				{
					if (dp[i * n + l] != -1)
					{
						dp[(i + 1) * n + j] = MAX(dp[(i + 1) * n + j], dp[i * n + l] + arr[j]);
					}
				}
			}
		}
	}

#pragma omp parallel for reduction(max: ans)
	for (i = 0; i < n; i++)
	{
		if (ans < dp[(k - 1) * n + i])
			ans = dp[(k - 1) * n + i];
	}

	free(dp);
	return (ans == -1) ? 0 : ans;
}

int main()
{
	int n, k;
	scanf(" %d", &n);
	scanf(" %d", &k);
	int *arr = malloc(n * sizeof(int));
	for (int i = 0; i < n; i++)
		scanf(" %d", &arr[i]);
	int ans = MaxIncreasingSub(arr, n, k);
	printf("%d\n", ans);
	return 0;
}
