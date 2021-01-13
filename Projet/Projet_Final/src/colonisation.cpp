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
    int provided;
    parametres param;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if(provided < MPI_THREAD_MULTIPLE)
    {
        printf("The threading support level is lesser than that demanded.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
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


    float apparition_civ; 
    float disparition; 
    float expansion;
    float inhabitable;
    
    
    //lecture des parametres
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
        // std::cout<<"parametres : " << apparition_civ << "; "
        //                         << disparition << "; "
        //                         << expansion << "; "
        //                         << inhabitable<< "; " << std::endl;
        
    }
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
    //std::cout <<"rank: " << rank << "; "<< pas << std::endl;

    int Etapes [nbp-1];
    //std::cout <<"here; "<< pas <<std::endl;
    for (int i = 0; i<nbp-1; i++)
    {
        Etapes[i] = i*pas;
    } 

    Etapes[nbp-1] = height ;

    // if (rank == 0)
    // {
    //     std::cout << "Etapes: ";
    //     for (int i = 0; i <nbp; i++)
    //         std::cout << Etapes[i]<< "; ";
    //     std::cout << std::endl;
    // }

    galaxie* Galaxies [nbp-1];// le premier processus sera le maitre; il affichera donc; pas de travail sur les galaxies pour lui
    galaxie* Galaxies_next [nbp-1];

    galaxie grande_g_next(width, height, param.apparition_civ);

    for (int i = 0; i<nbp-1; i++)
    {
        //std::cout <<"rank: " << rank << "; i: " << i << "; " << Etapes [i] << "; " << Etapes[i+1] << std::endl;
        galaxie* g = new galaxie (width, (Etapes[i+1] - Etapes[i])+2);//on rajoute une ligne avant et une après. Comme ca on est tranquille 
        galaxie* g_next = new galaxie (width, pas+2);
        // if (rank == 0)
        // {
        //     std::cout << "etape : " << i << "; Etapes[i]: " <<  Etapes[i] << "Etapes[i+1] : " << Etapes[i+1] << "; taille : " << g_next->m_planetes.size()<< std::endl;
        //     //g->print ();
        // }
        Galaxies[i] = g;
        Galaxies_next[i] = g_next;
        //initialisation des petites matrices...
        for (int compteur = Etapes[i-1]*width; compteur< Etapes[i]*width; compteur++)
        {
            Galaxies[i]->data()[compteur+width] = grande_g_next.data()[compteur+Etapes[i-1]*width];
        }
    }
    //std::cout <<"here"<< std::endl;

    
    //std::cout <<"here"<< std::endl;
    std::vector <int> buffer (pas*(width+2),0);
    std::vector <int> buffer_reception (pas*(width+2), 0);
    std::vector <int> grand_buffer (height*width, 0); // pour la grande galaxie
    
    //std::cout <<"here"<< std::endl;
    

    
    if (rank == 0)
    {
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

        for (int i = 1; i<nbp; i++)
        {
            //std::cout<< "first send"<< std::endl;
            //Galaxies[i-1]->print();
            MPI_Send(Galaxies[i-1]->data(), pas*width, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        std::chrono::time_point<std::chrono::system_clock> start, end1, end2;
        while (1) {
            start = std::chrono::system_clock::now();
            //std::cout <<"start"<< std::endl;
            //on recupere les infos...
            for (int i = 1; i<nbp; i++)
            {
                
                MPI_Recv (&buffer[0], pas*width, MPI_INT, i, 0, MPI_COMM_WORLD, 0 );
                //std::cout<< " i: "<< i<< std::endl;
                grande_g_next.insert_vector_fantome ( Etapes[i-1]*width , &buffer, width);
                //std::cout <<"here 1"<< i<< ", Etapes[i-1] : " << Etapes [i-1] << std::endl; 
                // if (i == 2)
                // {
                //     std::cout << "cases occupees recues" <<std::endl;
                //     for (int k = 0; k<Etapes[i] - Etapes[i-1]; k++)
                //     {
                //         for (int j = 0; j<width; j++)
                //         {
                //             if (buffer[k*width+j] == 1 )
                //                 std::cout <<  k << "; " << j << std::endl;
                //         }
                //     }
                // }

            }
            //exit (-1);
            // std::cout << "cases occupees recues" <<std::endl;
            // for (int i = 0; i<height; i++)
            // {
            //     for (int j = 0; j<width; j++)
            //     {
            //         if (grande_g_next.data()[i*width+j] == 1 && i>341)
            //             std::cout <<  i << "; " << j << std::endl;
            //     }
            // }
            //... et on en envoie d'autres...
            for (int i = 1; i<nbp; i++) // faire deux boucles permet aux autres processus de ne pas travailler pendant que la matrice est encore en train d'etre remplie 
            {
                //grande_g_next.extract_vector (&buffer, Etapes[i]);// inutile
                MPI_Send(grande_g_next.data(), height*width, MPI_INT, i, 0, MPI_COMM_WORLD);
                //std::cout <<"here 2"<< std::endl;
                
            }
            //...et on affiche pendant que les autres travaillent
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
            MPI_Recv (&grand_buffer[0], height*width, MPI_INT, 0, 0, MPI_COMM_WORLD, 0 );
            // if (rank == 1)
            // {
            //     std::cout << "cases occupees recues -- tableau" <<std::endl;
            //     for (int i = 0; i<height; i++)
            //     {
            //         for (int j = 0; j<width; j++)
            //         {
            //             if (grand_buffer[i*width+j] == 1)
            //                 std::cout <<  i << "; " << j << std::endl;
            //         }
            //     }
            // }
            //exit (-1);
            grande_g_next.update_data (&grand_buffer);
            //grande_g_next.extract_vector (buffer_reception);
            // if (rank == 1)
            // {
            //     std::cout << "cases occupees recues" <<std::endl;
            //     for (int i = 0; i<height; i++)
            //     {
            //         for (int j = 0; j<width; j++)
            //         {
            //             if (grande_g_next.m_planetes[i*width+j] == 1)
            //                 std::cout <<  i << "; " << j << std::endl;
            //         }
            //     }
            // }
            mise_a_jour(param, width, Etapes[rank-1], Etapes[rank], height,  Galaxies[rank-1]->data(), Galaxies_next[rank-1]->data(), grande_g_next.data());
            

            Galaxies_next[rank-1]->swap(*Galaxies[rank-1]);
            //std::cout << "fin m a j, process "<< rank << std::endl;
            //std::cout << "here, process " << rank << ";"<< Galaxies_next[rank-1]->data()<< std::endl;
            //grande_g_next.extract_vector (buffer_reception);
            // std::cout <<"cases envoi"<< std::endl;
            // if (rank == 2)
            // {
            //     //std::cout << "cases occupees recues" <<std::endl;
            //     for (int i = 0; i<(Etapes[rank]-Etapes[rank-1]); i++)
            //     {
            //         for (int j = 0; j<width; j++)
            //         {
            //             if (Galaxies[rank-1]->data()[i*width+j] == 1 )
            //                 std::cout <<  i << "; " << j << std::endl;
            //         }
            //     }
            // }
            MPI_Send (Galaxies[rank-1]->data(), pas*width, MPI_INT, 0, 0, MPI_COMM_WORLD);
            //std::cout << "here, process " << rank << ";"<< " a envoyé " << std::endl;
        }      
        
        

    }
    MPI_Finalize();
    
    

    

    return EXIT_SUCCESS;
}
