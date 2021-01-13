#include <cstdlib>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>        
#include <SDL2/SDL_image.h>
#include <fstream>
#include <ctime>
#include <iomanip>      // std::setw
#include <chrono>
#include <mpi.h>

#include "parametres.hpp"
#include "galaxie.hpp"
 
int main(int argc, char *argv[])
{
    char commentaire[4096];
    int width, height;
    SDL_Event event;
    SDL_Window   * window;

    parametres param;

    MPI_Init(&argc, &argv);
    // Pour des raison préfère toujours cloner le communicateur global
    // MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    // On interroge le communicateur global pour connaître le nombre de processus
    // qui ont été lancés par l'utilisateur :
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    // On interroge le communicateur global pour connaître l'identifiant qui
    // m'a été attribué ( en tant que processus ). Cet identifiant est compris
    // entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
    // l'utilisateur )
    int rank;
    MPI_Comm_rank(globComm, &rank);
    //lecture des parametres
    if (rank == 0)
    {
        std::ifstream fich("parametre.txt");
        fich >> width;
        fich.getline(commentaire, 4096);
        fich >> height;
        fich.getline(commentaire, 4096);
        fich >> param.apparition_civ;
        fich.getline(commentaire, 4096);
        fich >> param.disparition;
        fich.getline(commentaire, 4096);
        fich >> param.expansion;
        fich.getline(commentaire, 4096);
        fich >> param.inhabitable;
        fich.getline(commentaire, 4096);
        fich.close();
    }
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int pas = int (height/nbp-1);
    //std::cout <<"rank: " << rank << "; "<< pas << std::endl;

    int Etapes [nbp-1];
    std::cout <<"here; "<< pas <<std::endl;
    for (int i = 0; i<nbp-1; i++)
    {
        Etapes[i] = i*pas;
    } 

    Etapes[nbp-1] = height - (nbp-2)*pas;

    galaxie* Galaxies [nbp-1];// le premier processus sera le maitre; il affichera donc; pas de travail sur les galaxies pour lui
    galaxie* Galaxies_next [nbp-1];

    for (int i = 0; i<nbp-1; i++)
    {
        std::cout <<"rank: " << rank << "; i: " << i << "; " << Etapes [i] << "; " << Etapes[i+1] << std::endl;
        galaxie* g = new galaxie (width, i*(Etapes[i+1] - Etapes[i]), param.apparition_civ);
        galaxie* g_next = new galaxie (width, i*pas);
        Galaxies[i] = g;
        Galaxies_next[i] = g_next;
    }
    std::cout <<"here"<< std::endl;

    galaxie grande_g_next(width, height);

    
    

    std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
    std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
    std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
    std::cout << "\t Chance expansion : " << param.expansion << std::endl;
    std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
    std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
    std::srand(std::time(nullptr));

    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

    

    galaxie_renderer gr(window);
    if (rank == 0)
    {
        window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);
        int deltaT = (20*52840)/width;
        std::cout << "Pas de temps : " << deltaT << " années" << std::endl;
        gr.render(grande_g_next);
    }
    

    
    

    std::cout << std::endl;

    
    unsigned long long temps = 0;


    

    // On peut maintenant commencer à écrire notre programme parallèle en utilisant les
    // services offerts par MPI.



    
    //std::cout << "Hello World, I'm processus " << rank << " on " << nbp << " processes.\n";

    // A la fin du programme, on doit synchroniser une dernière fois tous les processus
    // afin qu'aucun processus ne se termine pendant que d'autres processus continue à
    // tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
    // qui ne seront pas encore terminés.
    //std::cout <<"here"<< std::endl;
    std::vector <char> buffer (pas*width);
    std::vector <char> buffer_reception (pas*width);
    std::vector <char> grand_buffer (height*width); // pour la grande galaxie
    //std::cout <<"here"<< std::endl;

    std::chrono::time_point<std::chrono::system_clock> start, end1, end2;
    while (1) {
        start = std::chrono::system_clock::now();
        if (rank == 0)
        {
            //on recupere les infos...
            for (int i = 0; i<nbp-1; i++)
            {
                MPI_Recv (&buffer[0], pas*width, MPI_CHAR, i, 0, MPI_COMM_WORLD, 0 );
                grande_g_next.insert_vector ( Etapes[i], &buffer);
                std::cout <<"here"<< std::endl;
            }
            //... et on en envoie d'autres...
            for (int i = 0; i<nbp-1; i++) // faire deux boucles permet aux autres processus de ne pas travailler pendant que la matrice est encore en train d'etre remplie 
            {
                //grande_g_next.extract_vector (&buffer, Etapes[i]);// inutile
                MPI_Send(grande_g_next.data(), pas*width, MPI_CHAR, i, 0, MPI_COMM_WORLD);
                
            }
            //...et on affiche pendant que les autres travaillent
            gr.render(grande_g_next);

        }
        else 
        {
            MPI_Recv (&grand_buffer[0], pas*width, MPI_CHAR, 0, 0, MPI_COMM_WORLD, 0 );
            std::cout << "here, process " << rank << std::endl;
            grande_g_next.update_data (&grand_buffer);
            //grande_g_next.extract_vector (buffer_reception);
            mise_a_jour(param, width, Etapes[rank-1], Etapes[rank], height,  Galaxies[rank-1]->data(), Galaxies_next[rank-1]->data(), grande_g_next.data());
            Galaxies_next[rank-1]->swap(*Galaxies[rank-1]);
            MPI_Send (Galaxies_next[rank-1]->data(), pas*width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

        }

        // end1 = std::chrono::system_clock::now();
        // end2 = std::chrono::system_clock::now();
        
        // std::chrono::duration<double> elaps1 = end1 - start;
        // std::chrono::duration<double> elaps2 = end2 - end1;
        
        // temps += deltaT;
        // std::cout << "Temps passe : "
        //           << std::setw(10) << temps << " années"
        //           << std::fixed << std::setprecision(3)
        //           << "  " << "|  CPU(ms) : calcul " << elaps1.count()*1000
        //           << "  " << "affichage " << elaps2.count()*1000
        //           << "\r" << std::flush;
        //_sleep(1000);
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
          std::cout << std::endl << "The end" << std::endl;
          break;
        }
    }
    MPI_Finalize();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
