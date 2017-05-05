#include <mpi.h>
#include <iostream>
#include <iomanip>

using namespace std;

void print(int A[][4]);
void print(int x[]);

int main(int argc, char** argv)
{
    // matrix/vector size
    const size_t N = 4;
    int matrix[N][N] =
    {
        {1, 5, 7, 8},
        {7, 2, 3, 4},
        {8, 1, 5, 2},
        {2, 5, 0, 1}
    };
    int vector[N] = {4, 3, 2, 1};
    int result[N];
    
    MPI_Init(&argc, &argv);
    
    int nprocs, pid;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    
    if(pid == 0)
    {
        printf("|*| Matrix A = \n");
        print(matrix);
        printf("|*| Vector x = \n");
        print(vector);
    }
    
    if (nprocs < N*N)
    {
        MPI_Finalize();
        if (pid == 0)
            cout << "Not enough processors..." << endl;
        return -1;
    }
    
    // create communicators with N cpu
    MPI_Group groups[N], rootGroup;
    MPI_Comm comms[N], rootComm;
    
    MPI_Group groupWorld;
    MPI_Comm_group(MPI_COMM_WORLD, &groupWorld);
    
    for (size_t i = 0; i < N; ++i)
    {
        int ranks[N];
        for (size_t j = 0; j < N; ++j)
            ranks[j] = j * N + i;
        
        MPI_Group_incl(groupWorld, N, ranks, &groups[i]);
        MPI_Comm_create(MPI_COMM_WORLD, groups[i], &comms[i]);
    }
    
    int ranks[N];
    for (size_t i = 0; i < N; ++i)
        ranks[i] = i;
    MPI_Group_incl(groupWorld, N, ranks, &rootGroup);
    MPI_Comm_create(MPI_COMM_WORLD, rootGroup, &rootComm);
    
    // send matrix's row and vector into communicator
    int matrixValue;
    int vectorValue;
    
    MPI_Scatter(matrix[pid % N], 1, MPI_INT, &matrixValue, 1, MPI_INT, 0, comms[pid % N]);
    MPI_Scatter(vector, 1, MPI_INT, &vectorValue, 1, MPI_INT, 0, comms[pid % N]);
    
    printf("|*| P%d has matrix element %d\n", pid, matrixValue);
    
    // calculate and send result
    int element;
    int process = matrixValue * vectorValue;

    MPI_Reduce(&process, &element, 1, MPI_INT, MPI_SUM, 0, comms[pid % N]);
    
    if (pid < N)
        MPI_Gather(&element, 1, MPI_INT, result, 1, MPI_INT, 0, rootComm);
    
    MPI_Finalize();
    
    if (pid == 0)
    {
        cout << "\nAfter multiplication," << endl;
        cout << "y  =  ";
        for (size_t i = 0; i < N; ++i)
            cout << result[i] << " ";
        cout << endl << endl;
    }
    return 0;
}

void print(int A[][4])
{
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
            cout<<setw(5)<<A[i][j];
        cout << endl;
    }
    cout << endl;
}

void print(int x[])
{
    for(int j=0;j<4;j++)
        cout << setw(5) << x[j];
    cout << endl << endl;
}

