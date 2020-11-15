# include <chrono>
# include <random>
# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>
using namespace std;

// Attention , ne marche qu'en C++ 11 ou supérieur :
double approximate_pi( int nbSamples) {
    typedef chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now();
    myclock::duration d = beginning.time_since_epoch();
    unsigned seed = d.count();
    default_random_engine generator(seed);
    uniform_real_distribution <double> distribution ( -1.0 ,1.0);
    unsigned long nbDarts = 0;
    // Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
    for ( int sample = 0 ; sample < nbSamples ; ++ sample ) {
        double x = distribution(generator);
        double y = distribution(generator);
        // Test if the dart is in the unit disk
        if ( x*x+y*y<=1 ) nbDarts ++;
    }
    // Number of nbDarts throwed in the unit disk
    double ratio = double(nbDarts)/double(nbSamples);
    return 4*ratio;
}

int main( int nargs, char* argv[] ) {
	int nbDarts, nbSamples, nbDarts_tot, nbSamples_tot, nbp, rank, tag = 1, tag2 = 2;
	double time, total_time;

	MPI_Status Stat;
	//MPI_Request request;
	MPI_Init( &nargs, &argv );
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);

	MPI_Comm_size(globComm, &nbp);
	MPI_Comm_rank(globComm, &rank);

	// Création d'un fichier pour ma propre sortie en écriture :
	stringstream fileName;
	fileName << "Output" << setfill('0') << setw(5) << rank << ".txt";
	ofstream output( fileName.str().c_str() );

	//mon code
	nbSamples_tot = atoi(argv[1]);
	nbSamples = nbSamples_tot / nbp;
	nbSamples_tot = nbp * nbSamples;

	if (rank == 0) {
		double start = MPI_Wtime();

		typedef chrono::high_resolution_clock myclock;
		myclock::time_point beginning = myclock::now();
		myclock::duration d = beginning.time_since_epoch();
		unsigned seed = d.count();
		default_random_engine generator(seed);
		uniform_real_distribution <double> distribution ( -1.0 ,1.0);
		// Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
		for ( int sample = 0 ; sample < nbSamples ; ++ sample ) {
			double x = distribution(generator);
			double y = distribution(generator);
			// Test if the dart is in the unit disk
			if ( x*x+y*y<=1 ) nbDarts ++;
		}

		double end = MPI_Wtime();
		time = end - start;
		
		output << nbDarts << "/" << nbSamples << " sont dans le disque" << endl;
		output << "Temps mis par le processeur " << rank << " : " << time << "s" << endl;

		nbDarts_tot = nbDarts;
		total_time = time;
		for (int proc = 1; proc < nbp; proc++) {
			MPI_Recv(&nbDarts, 1, MPI_INT, proc, tag, globComm, &Stat);
			MPI_Recv(&time, 1, MPI_DOUBLE, proc, tag2, globComm, &Stat);
			nbDarts_tot += nbDarts;
			total_time += time;
		}
		double ratio = double(nbDarts_tot)/double(nbSamples_tot);
    	cout << "PI : " << 4*ratio << endl;
		cout << "temps total : " << total_time << "s" << endl;
	}
	else {
		double start = MPI_Wtime();

		typedef chrono::high_resolution_clock myclock;
		myclock::time_point beginning = myclock::now();
		myclock::duration d = beginning.time_since_epoch();
		unsigned seed = d.count();
		default_random_engine generator(seed);
		uniform_real_distribution <double> distribution ( -1.0 ,1.0);
		// Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
		for ( int sample = 0 ; sample < nbSamples ; ++ sample ) {
			double x = distribution(generator);
			double y = distribution(generator);
			// Test if the dart is in the unit disk
			if ( x*x+y*y<=1 ) nbDarts ++;
		}

		double end = MPI_Wtime();
		time = end - start;

		output << nbDarts << "/" << nbSamples << " sont dans le disque" << endl;
		output << "Temps mis par le processeur " << rank << " : " << time << "s" << endl;

		MPI_Send(&nbDarts, 1, MPI_INT, 0, tag, globComm);
		MPI_Send(&time, 1, MPI_DOUBLE, 0, tag2, globComm);

	}




	output.close();

	MPI_Finalize();
	return EXIT_SUCCESS;
}
