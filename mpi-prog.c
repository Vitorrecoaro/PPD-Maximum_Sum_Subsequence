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
	int n, k, qtdTaskPerNode, rest, ans;

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

	for (int i = 1; i < k; i++)
	{
		qtdTaskPerNode = (n - i) / (totalNodes - 1);
		rest = (n - i) % (totalNodes - 1);
		int qtdTasksSended = 0;
		int restsSended = 0;

		MPI_Bcast(dp, n, MPI_INT, 0, MPI_COMM_WORLD);

		for (int j = 1; j < totalNodes; j++)
		{
			int qtdTask = qtdTaskPerNode;

			if (rest - restsSended > 0)
			{
				qtdTask += 1;
				restsSended += 1;
			}

			infosNodes[j - 1].qtdTasks = qtdTask;
			infosNodes[j - 1].indexes = malloc(qtdTask * sizeof(int));

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
			MPI_Request *request;
			
			MPI_Isend((void *)infosNodes[j - 1].qtdTasks, 1, MPI_INT, j, 0, MPI_COMM_WORLD, request);
			MPI_Isend((void *)infosNodes[j - 1].indexes, infosNodes[j - 1].qtdTasks, MPI_INT, j, 0, MPI_COMM_WORLD, request);
		}

		if (i < k - 1)
		{
			for (int j = 1; j < totalNodes; j++)
			{
				int *receivedNumbers = malloc(infosNodes[j - 1].qtdTasks * sizeof(int));
				MPI_Recv(receivedNumbers, infosNodes[j - 1].qtdTasks, MPI_INT, j, 0, MPI_COMM_WORLD, NULL);

				for (int k = 0; k < infosNodes[j - 1].qtdTasks; k++)
				{
					dp[infosNodes[j - 1].indexes[k]] = receivedNumbers[k];
				}

				free(infosNodes[j - 1].indexes);
				free(receivedNumbers);
			}
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
	free(infosNodes);
}

void workerFunction()
{
	int *arr, n, k, *dp;
	int max = -1;

	// Recebe os valores do master.
	MPI_Bcast((void *)&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast((void *)&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
	arr = malloc(n * sizeof(int));
	dp = malloc(n * sizeof(int));
	MPI_Bcast((void *)arr, n, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < k; i++)
	{
		int *indexes, qtdTasks, *results;

		MPI_Bcast(dp, n, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Recv(&qtdTasks, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
		indexes = malloc(qtdTasks * sizeof(int));
		results = malloc(qtdTasks * sizeof(int));
		MPI_Recv(indexes, qtdTasks, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);

		for (int j = 0; j < qtdTasks; j++)
		{
			for (k = 0; k < j; k++)
			{
				if (arr[k] < arr[indexes[j]])
				{
					if (dp[k] != -1)
					{
						results[j] = MAX(results[j], dp[k] + arr[j]);
						max = MAX(max, results[j]);
					}
				}
			}
		}

		if (i < k - 1)
		{
			MPI_Send(results, qtdTasks, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Reduce(&max, NULL, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
		}

		free(results);
		free(indexes);
		MPI_Barrier(MPI_COMM_WORLD);
	}

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
		workerFunction();
	}

	MPI_Finalize();

	return 0;
}
