/* Hypercube mergesort using MPI **********************************************/
#include <iostream>
#include "mpi.h"
#include <cmath>
#include <fstream>

using namespace std;

#define N 10000 /* Maximum list size */

int nprocs, dim, pid; /* Cube size, dimension, & my node ID */

void init(int data[]);

/* Sequential mergesort (either ascending or descending) */
void mergesort(int list[], int left, int right, int descending)
{
    int i, j, k, t, middle, temp[N];

    if (left < right) {
        middle = (left + right)/2;
        mergesort(list, left, middle, descending);
        mergesort(list, middle+1, right, descending);
        
        k = i = left; j = middle+1;
        if (descending)
            while (i<=middle && j<=right)
                temp[k++] = list[i]>list[j] ? list[i++] : list[j++];
        else
            while (i<=middle && j<=right)
                temp[k++] = list[i]<list[j] ? list[i++] : list[j++];
        t = i>middle ? j : i;
        while (k <= right) temp[k++] = list[t++];
        for (k=left; k<=right; k++) list[k] = temp[k];
    }
}

/* Parallel mergesort */
void parallel_mergesort(int pid,int list[],int n)
{
    int listsize, l, m, bitl = 1, bitm, partner, i;
    MPI_Status status;
    
    listsize = n/nprocs;
    mergesort(list,0,listsize-1,pid & bitl);
    
    for (l=1; l <= dim; l++) {
        bitl = bitl << 1;
        for (bitm=1, m=0; m<l-1; m++) bitm *= 2;
        for (m=l-1; m>=0; m--)
        {
            partner = pid ^ bitm;

            MPI_Send(list, listsize, MPI_INT, partner, l * dim + m, MPI_COMM_WORLD);
            MPI_Recv(&list[listsize], listsize, MPI_INT, partner, l * dim + m, MPI_COMM_WORLD, &status);
            mergesort(list,0,2*listsize-1,pid & bitl);
            
            if (pid & bitm)
                for (i=0; i<listsize; i++) list[i] = list[i+listsize];
                    bitm = bitm >> 1;
        }
    }
}

int main(int argc, char *argv[])
{
    int list[10000];
    int n = 80, i;
    
    MPI_Init(&argc,&argv); /* Initialize the MPI environment */
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    dim = log(nprocs+1e-10)/log(2.0);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);
    
    init(list);
    
    int stream[n/nprocs];
    int begin = pid * n/nprocs;
    
    for(int i = begin; i < (begin + n/nprocs); i++)
        stream[i - begin] = list[i];

    parallel_mergesort(pid, stream, n);
    
    printf("***** P%d *****\n",pid);
    for (i = 0; i < n/nprocs; i++) printf(" %d ",stream[i]);
    printf("\n\n");
    
    MPI_Finalize();
    
    return 0;
}

void init(int data[])
{
    int i = 0;
    
    ifstream file("keys.txt");
    
    while (i < 10000)
    {
        file >> data[i];
        i++;
    }
}

