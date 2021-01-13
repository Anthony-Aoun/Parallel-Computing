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
 
int main(int argc, char *argv[]) {
    char commentaire[4096];
    int width, height;
    SDL_Event event;
    SDL_Window   * window;

    parametres param;

    MPI_Init(&argc, &argv);
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    int rank;
    MPI_Comm_rank(globComm, &rank);


    float apparition_civ; 
    float disparition; 
    float expansion;
    float inhabitable;
    
    
    //lecture des parametres par root
    if (rank == 0)
    {
        std::ifstream fich("parametre.txt");
        fich >> width;
        fich.getline(commentaire, 4096);
        fich >> height;
        fich.getline(commentaire, 4096);
        fich >>  apparition_civ; 
        fich.getline(commentaire, 4096);
        fich >>  disparition; 
        fich.getline(commentaire, 4096);
        fich >>  expansion; 
        fich.getline(commentaire, 4096);
        fich >>  inhabitable;
        fich.getline(commentaire, 4096);
        fich.close();
    }
    //envoi des paramètres à tous les processus
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&apparition_civ, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&disparition, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&expansion, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&inhabitable, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    param.apparition_civ = apparition_civ;
    param.disparition = disparition;
    param.expansion = expansion;
    param.inhabitable = inhabitable;


    int pas = int (height/(nbp-1));

    int Etapes [nbp-1];

    for (int i = 0; i<nbp-1; i++) {
        Etapes[i] = i*pas;
    } 

    Etapes[nbp-1] = height ;

    // le premier processus sera le maitre; il affichera donc; pas de travail sur les galaxies pour lui
    galaxie* Galaxies [nbp-1];
    galaxie* Galaxies_next [nbp-1];

    galaxie grande_g_next(width, height, param.apparition_civ);

    for (int i = 0; i<nbp-1; i++) {
        //pn déclare g et g_next
        //on rajoute une ligne avant et une après. Comme ca on est tranquil
        galaxie* g = new galaxie (width, (Etapes[i+1] - Etapes[i])+2);
        galaxie* g_next = new galaxie (width, pas+2);
      
        //on remplie les petites g et g_next dans des tableaux
        Galaxies[i] = g; //pour les petites
        Galaxies_next[i] = g_next; //pour les petites
        //initialisation des petites matrices...
        for (int compteur = Etapes[i-1]*width; compteur< Etapes[i]*width; compteur++)
        {
            Galaxies[i]->data()[compteur+width] = grande_g_next.data()[compteur+Etapes[i-1]*width];
        }
    }


    std::vector <int> buffer (pas*(width+2),0);
    std::vector <int> buffer_reception (pas*(width+2), 0); // pour la petite galaxie
    std::vector <int> grand_buffer (height*width, 0); // pour la grande galaxie
    

    if (rank == 0) {
        std::cout << "Resume des parametres (proba par pas de temps): " << std::endl;
        std::cout << "\t Chance apparition civilisation techno : " << param.apparition_civ << std::endl;
        std::cout << "\t Chance disparition civilisation techno: " << param.disparition << std::endl;
        std::cout << "\t Chance expansion : " << param.expansion << std::endl;
        std::cout << "\t Chance inhabitable : " << param.inhabitable << std::endl;
        std::cout << "Proba minimale prise en compte : " << 1./RAND_MAX << std::endl;
        std::srand(std::time(nullptr));

        SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);
        galaxie_renderer gr(window);
        int deltaT = (20*52840)/width;
        std::cout << "Pas de temps : " << deltaT << " années" << std::endl;
        gr.render(grande_g_next);
        unsigned long long temps = 0;

        //envoie peties galaxies aux processus
        for (int i = 1; i<nbp; i++) {
            MPI_Send(Galaxies[i-1]->data(), pas*width, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        std::chrono::time_point<std::chrono::system_clock> start, end1, end2;
        while (1) {
            start = std::chrono::system_clock::now();
            //on recupere les petites galaxies + remplissage de la grande galaxie avec cellules fantomes...
            for (int i = 1; i<nbp; i++) {
                MPI_Recv (&buffer[0], pas*width, MPI_INT, i, 0, MPI_COMM_WORLD, 0 );
                grande_g_next.insert_vector_fantome ( Etapes[i-1]*width , &buffer, width);
            }
            //... puis on envoie la grande galaxie à tout le monde ...
            // faire deux boucles permet aux autres processus de ne pas travailler pendant que la matrice est encore en train d'etre remplie
            for (int i = 1; i<nbp; i++) {
                MPI_Send(grande_g_next.data(), height*width, MPI_INT, i, 0, MPI_COMM_WORLD); 
            }
            end1 = std::chrono::system_clock::now();
            gr.render(grande_g_next);
            end2 = std::chrono::system_clock::now();
            
            std::chrono::duration<double> elaps1 = end1 - start;
            std::chrono::duration<double> elaps2 = end2 - end1;
            
            temps += deltaT;
            std::cout << "Temps passe : "
                      << std::setw(10) << temps << " années"
                      << std::fixed << std::setprecision(3)
                      << "  " << "|  CPU(ms) : calcul " << elaps1.count()*1000
                      << "  " << "affichage " << elaps2.count()*1000
                      << "\r" << std::flush;
            //_sleep(1000);
            if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
              std::cout << std::endl << "The end" << std::endl;
              break;
            }
        }
        
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    else 
    { 
        while (1)
        {
            //recoit grande matrice
            MPI_Recv (&grand_buffer[0], height*width, MPI_INT, 0, 0, MPI_COMM_WORLD, 0 );
          
            //charge ses données dans m_planetes
            grande_g_next.update_data (&grand_buffer);
            
            //met à jour la partie de la matrice dont la tache est reponsable
            mise_a_jour(param, width, Etapes[rank-1], Etapes[rank], height,  Galaxies[rank-1]->data(), Galaxies_next[rank-1]->data(), grande_g_next.data());
            Galaxies_next[rank-1]->swap(*Galaxies[rank-1]);
            
            //envoie la nouvelle petite matrice
            MPI_Send (Galaxies[rank-1]->data(), pas*width, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }      
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
