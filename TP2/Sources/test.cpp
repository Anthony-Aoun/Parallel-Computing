# include <iostream>
# include <cstdlib>
# include <vector>
# include <chrono>
# include "mpi.h"
using namespace std;

int main(int argc, char *argv[] ) { 
    //MY CODE
    int nbp, rank, root = 0;
    double result = 0.;
    chrono::time_point<chrono::system_clock> start, end;
    //for Gather
    int * recarr = new int[3]; 
    //for Scatter
    int sendarr[3][2] = {
        {3,2},
        {9,1},
        {7,6}
    };
    int recbuf[2];

    start = chrono::system_clock::now();
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nbp);
    
    if (nbp == 3) {
        MPI_Scatter(sendarr, 2, MPI_INT, recbuf, 3, MPI_INT, root, MPI_COMM_WORLD);
        MPI_Gather(&rank, 1, MPI_INT, recarr, 1, MPI_INT, root, MPI_COMM_WORLD);
        
        end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end-start;
        double time = elapsed_seconds.count();
        MPI_Reduce(&time, &result, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);

        //for Scatter
        cout << "Rank " << rank << " : " << recbuf[1] << endl;
        
        //for Gather and Reduce
        if (rank == root) {
            cout << recarr[1] << endl;
            cout << "TOTAL TIME : " << result << endl;
        }
    }
    else {
        cout << "Wrong number of proccesses" << endl;
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
 }