# include <iostream>
# include <cstdlib>
# include <mpi.h>
using namespace std;

int main( int nargs, char* argv[] ) {
	int nbp, rank , dest , source, tag =1;
	int inmsg, outmsg;
	MPI_Status Stat;
	MPI_Init( &nargs, &argv );

	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	MPI_Comm_size(globComm, &nbp);
	MPI_Comm_rank(globComm, &rank);

	if (rank == 0) {
		source = nbp - 1;
		dest = 1;
		outmsg = rank*2;
		MPI_Send(&outmsg, 1, MPI_INT, dest, tag, globComm);
		MPI_Recv(&inmsg, 1, MPI_INT, source, tag, globComm, &Stat);
		inmsg ++;
		cout << "Processus " << rank << " : recoit le jeton " << inmsg << endl;
	}
	else if (rank == nbp - 1) {
		source = rank - 1;
		dest = 0;
		outmsg = rank*2;
		MPI_Send(&outmsg, 1, MPI_INT, dest, tag, globComm);
		MPI_Recv(&inmsg, 1, MPI_INT, source, tag, globComm, &Stat);
		inmsg ++;
		cout << "Processus " << rank << " : recoit le jeton " << inmsg << endl;
	}
	else {
		source = rank - 1;
		dest = rank + 1;
		outmsg = rank*2;
		MPI_Send(&outmsg, 1, MPI_INT, dest, tag, globComm);
		MPI_Recv(&inmsg, 1, MPI_INT, source, tag, globComm, &Stat);
		inmsg ++;
		cout << "Processus " << rank << " : recoit le jeton " << inmsg << endl;
	}

	MPI_Finalize();
	return EXIT_SUCCESS;
}
