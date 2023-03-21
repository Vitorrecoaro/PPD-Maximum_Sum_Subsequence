#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define MAX(a, b) a > b ? a : b

typedef struct infoNode
{
	int *indexes;
	int qtdTasks;
} infoNode;

void masterFunction(int totalNodes)
{
	int n, k, qtdTaskPerNode, rest, ans, maxTasks;

	scanf(" %d", &n);
	scanf(" %d", &k);

	int *arr = malloc(n * sizeof(int));
	int *dp = malloc(n * sizeof(int));
	infoNode *infosNodes = malloc((totalNodes - 1) * sizeof(infoNode));

	for (int i = 0; i < n; i++)
	{
		scanf(" %d", &arr[i]);
		dp[i] = arr[i];
	}

	// Envia o vetor a todos os nós pois será utilizado por todos para leitura.
	MPI_Bcast((void *)&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast((void *)&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast((void *)arr, n, MPI_INT, 0, MPI_COMM_WORLD);

	maxTasks = (n) / (totalNodes - 1) + 1;

	// Inicializa o vetor de indices de cada nó.
	for (int i = 0; i < totalNodes - 1; i++)
	{
		infosNodes[i].indexes = malloc(maxTasks * sizeof(int));
	}

	for (int i = 1; i < k; i++)
	{
		qtdTaskPerNode = (n - i) / (totalNodes - 1);
		rest = (n - i) % (totalNodes - 1);
		int qtdTasksSended = 0;

		MPI_Bcast(dp, n, MPI_INT, 0, MPI_COMM_WORLD);

		for (int j = 1; j < totalNodes; j++)
		{
			int qtdTask = qtdTaskPerNode;

			if (i < rest)
			{
				qtdTask += 1;
			}

			infosNodes[j - 1].qtdTasks = qtdTask;

			for (int k = 0; k < qtdTask; k++)
			{
				if (qtdTasksSended % 2 == 0)
				{
					infosNodes[j - 1].indexes[k] = i + qtdTasksSended++ / 2;
				}
				else
				{
					infosNodes[j - 1].indexes[k] = n - 1 - qtdTasksSended++ / 2;
				}
			}
			MPI_Send((void *)&qtdTask, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
			MPI_Send((void *)infosNodes[j - 1].indexes, infosNodes[j - 1].qtdTasks, MPI_INT, j, 0, MPI_COMM_WORLD);
		}

		if (i < k - 1)
		{
			int *receivedNumbers = malloc(maxTasks * sizeof(int));

			for (int j = 1; j < totalNodes; j++)
			{
				MPI_Recv(receivedNumbers, infosNodes[j - 1].qtdTasks, MPI_INT, j, 0, MPI_COMM_WORLD, NULL);

				for (int k = 0; k < infosNodes[j - 1].qtdTasks; k++)
				{
					dp[infosNodes[j - 1].indexes[k]] = receivedNumbers[k];
				}
			}
			free(receivedNumbers);
		}
		else
		{
			int max = 0;
			MPI_Reduce(MPI_IN_PLACE, &ans, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
			if (ans == -1)
			{
				ans = 0;
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	printf("%d\n", ans);
	free(arr);
	free(dp);

	for (int i = 0; i < totalNodes - 1; i++)
	{
		free(infosNodes[i].indexes);
	}

	free(infosNodes);
}


void resetArray(int tam, int *arr){
	for (int i = 0; i < tam; i++){
		arr[i] = 0;
	}
}

void workerFunction(int rank, int totalNodes)
{
	int *arr, n, k, *dp, *results, *indexes, maxTasks;
	int max = -1;

	// Recebe os valores do master.
	MPI_Bcast((void *)&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast((void *)&k, 1, MPI_INT, 0, MPI_COMM_WORLD);

	arr = malloc(n * sizeof(int));
	dp = malloc(n * sizeof(int));
	maxTasks = (n) / (totalNodes - 1) + 1;

	MPI_Bcast((void *)arr, n, MPI_INT, 0, MPI_COMM_WORLD);

	indexes = (int *) malloc(maxTasks * sizeof(int));
	results = (int *) calloc(maxTasks, sizeof(int));

	for (int i = 1; i < k; i++)
	{
		int qtdTasks;

		MPI_Bcast(dp, n, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Recv(&qtdTasks, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
		MPI_Recv(indexes, qtdTasks, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

		for (int j = 0; j < qtdTasks; j++)
		{
			for (int l = 0; l < indexes[j]; l++)
			{
				if (arr[l] < arr[indexes[j]])
				{
					if (dp[l] != 0)
					{
						results[j] = MAX(results[j], dp[l] + arr[indexes[j]]);
						
						if (i == k - 1){
							max = MAX(max, results[j]);
						}
					}
				}
			}
		}

		if (i < k - 1)
		{
			MPI_Send((void *)results, qtdTasks, MPI_INT, 0, 0, MPI_COMM_WORLD);
			resetArray(maxTasks, results);
		}
		else
		{
			MPI_Reduce(&max, NULL, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	free(results);
	free(indexes);
	free(arr);
	free(dp);
}

int main(int argc, char *argv[])
{
	int totalNodes, rank;
	int status = MPI_Init(&argc, &argv);

	if (status != MPI_SUCCESS)
	{
		printf("Erro ao iniciar o ambiente paralelo com MPI.\n");
		MPI_Abort(MPI_COMM_WORLD, status);
		return -1;
	}

	MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		masterFunction(totalNodes);
	}
	else
	{
		workerFunction(rank, totalNodes);
	}

	MPI_Finalize();

	return 0;
}
