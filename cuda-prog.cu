#include<cuda.h>
#include<stdio.h>
#include<stdlib.h> 

// # qtdTotalSum = A quantidade total de somas parciais que serão feitas, que pode ser calculada por uma P.A de ordem 1
// # iniciando de "col" e indo até "n-1", sendo assim ficando a seguinte equação para o cálculo da soma total:
// # ((col + n - 1)*(n - col)) / 2 =>  n^2 + col - (n + col^2) / 2 
__global__ void calculatePartialSum( int totalOperations, int col, int k, int *arr, int *dp){
	// # Para achar a qual elemento do vetor a thread se refere, é fazer a conta reversa, pois se o id da thread for 5,
	// # então já se passaram 5 contas feitas antes dela, assim podemos usar bhaskara para achar a qual elemento da arr 
	// # a thread se refere com a seguinte conta: "valorVet"=(1 + sqrt(1-4*(col - col^2 - 2*idx))/2, como poderá dar uns
    // # quebrados, pegamos o chão desta conta.

	// # E para achar com qual elemento da array ela vai tentar fazer a soma a gente usa o resultado anterior e calcula 
	// # em qual thread id se iniciou aquele bloco de comparação e faz a subtração desse elemento com o thread id. 
	// # Utilizando a seguinte conta: threadId - (valorVet^2 + col - valorVet - col^2)/ 2.
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if(idx < totalOperations){
        int refValue = (int)(1 + sqrtf((float)(1 - 4 * (col - (col * col) - 2 * idx)))) / 2;
        int compValue = (int)(idx) - (int)(refValue * refValue + col - refValue - col * col) / 2;

        if(arr[compValue] < arr[refValue]){
            if(dp[compValue * (k + 1) + col] != -1){
                int newVal = dp[compValue * (k + 1) + col] + arr[refValue];
                atomicMax(&dp[refValue * (k + 1) + col + 1], newVal);
            }
        }
    }
}

__global__ void calculateAnswer(int n, int k, int *ans, int *dp){
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    if (idx < n){
        if (*ans < dp[ idx * (k + 1) + k]){
            atomicMax(ans, dp[ idx * (k + 1) + k]);
        }
    }
}

int MaxIncreasingSub(int *arr, int n, int k) 
{
	int *dp = (int *)malloc(n * (k + 1) * sizeof(int));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < k + 1; j++) {
            dp[i * (k + 1) + j] = -1;
        }
    }

    for (int i = 0; i < n; i++) {
        dp[i * (k + 1) + 1] = arr[i];
    }

    int *d_arr, *d_dp;

    cudaMalloc((void **)&d_arr, n * sizeof(int));
    cudaMalloc((void **)&d_dp, n * (k + 1) * sizeof(int));
    cudaMemcpy(d_arr, arr, n * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_dp, dp, n * (k + 1) * sizeof(int), cudaMemcpyHostToDevice);
    free(dp);

	for(int i = 1; i < k; i ++){
        int threadsPerBlock = 1024;
        int totalOperations = (n * n + i - (n + i * i)) / 2;
        int blocksPerGrid = (totalOperations + threadsPerBlock - 1) / threadsPerBlock;
        calculatePartialSum<<<blocksPerGrid, threadsPerBlock>>>( totalOperations, i, k, d_arr, d_dp);
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
