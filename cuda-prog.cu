#include<cuda.h>
#include<stdio.h>
#include<stdlib.h> 

__global__ void calculatePartialSum( int totalOperations, int lin, int n, int *arr, int *dp){
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if(idx < totalOperations){
        int refValue = (int)(1 + sqrtf((float)(1 - 4 * ((lin + 1) - ((lin + 1) * (lin + 1)) - 2 * idx)))) / 2;
        int compValue = (int)(idx) - (int)(refValue * refValue + (lin + 1) - refValue - (lin + 1) * (lin + 1)) / 2;

        if(arr[compValue] < arr[refValue]){
            if(dp[lin * n + compValue] != -1){
                int newVal = dp[lin * n + compValue] + arr[refValue];
                atomicMax(&dp[(lin + 1) * n + refValue ], newVal);
            }
        }
    }
}

__global__ void calculateAnswer(int n, int k, int *ans, int *dp){
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if (idx < n){
        if (*ans < dp[ (k - 1) * n + idx]){
            atomicMax(ans, dp[ (k - 1) * n + idx]);
        }
    }
}

int MaxIncreasingSub(int *arr, int n, int k) 
{
	int *dp = (int *)malloc(n * k * sizeof(int));
    for (int i = 0; i < n * k; i++) {
        dp[i] = -1;
    }

    for (int i = 0; i < n; i++) {
        dp[i] = arr[i];
    }

    int *d_arr, *d_dp;

    cudaMalloc((void **)&d_arr, n * sizeof(int));
    cudaMalloc((void **)&d_dp, n * k * sizeof(int));
    cudaMemcpy(d_arr, arr, n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_dp, dp, n * k * sizeof(int), cudaMemcpyHostToDevice);
    free(dp);

	for(int i = 0; i < (k - 1); i ++){
        int threadsPerBlock = 1024;
        int totalOperations = (n * n + (i + 1) - (n + (i + 1) * (i + 1))) / 2;
        int blocksPerGrid = (totalOperations + threadsPerBlock - 1) / threadsPerBlock;
        calculatePartialSum<<<blocksPerGrid, threadsPerBlock>>>( totalOperations, i, n, d_arr, d_dp);
        cudaDeviceSynchronize();
	}
    cudaFree(d_arr);

    int ans, *d_ans;

    ans = -1;
    cudaMalloc((void **)&d_ans, sizeof(int));
    cudaMemcpy(d_ans, &ans, sizeof(int), cudaMemcpyHostToDevice);

    int threadPerBlock = 1024;
    int blockPerGrid = ( n + threadPerBlock - 1 ) / threadPerBlock;
    calculateAnswer<<<blockPerGrid, threadPerBlock>>>(n, k, d_ans, d_dp);
    cudaDeviceSynchronize();
    cudaMemcpy(&ans, d_ans, sizeof(int), cudaMemcpyDeviceToHost);
    cudaFree(d_dp);
    cudaFree(d_ans);

    return (ans == -1) ? 0 : ans; 
} 

int main() 
{ 
	int n, k;
    scanf(" %d", &n);
    scanf(" %d", &k);
    int *arr = (int *) malloc(n * sizeof(int));
	for (int i = 0; i<n; i++)
		scanf(" %d", &arr[i]);
	int ans = MaxIncreasingSub(arr, n, k); 
	printf("%d\n", ans); 
	return 0;
} 
