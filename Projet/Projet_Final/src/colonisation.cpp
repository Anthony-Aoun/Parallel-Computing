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
 
int main(int argc, char ** argv)
{
    char commentaire[4096];
    int width, height;
    SDL_Event event;
    SDL_Window   * window;

    parametres param;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
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
    MPI_Bcast(&width, 1, MPI_INT, 0, globComm);
    MPI_Bcast(&height, 1, MPI_INT, 0, globComm);

    MPI_Bcast(&apparition_civ, 1, MPI_FLOAT, 0, globComm);
    MPI_Bcast(&disparition, 1, MPI_FLOAT, 0, globComm);
    MPI_Bcast(&expansion, 1, MPI_FLOAT, 0, globComm);
    MPI_Bcast(&inhabitable, 1, MPI_FLOAT, 0, globComm);
    
    param.apparition_civ = apparition_civ;
    param.disparition = disparition;
    param.expansion = expansion;
    param.inhabitable = inhabitable;

    int pas;
    if (height%(nbp-1) == 0)
        pas = int (height/(nbp-1));
    else
        pas  = int (height/(nbp-1))+1;

    galaxie grande_galaxie(width, height, param.apparition_civ);// celle qu'on va afficher
    galaxie g(width, pas+2); //le +2 pour les cellules fantomes
    galaxie g_next(width, pas+2); //la derniere sera trop grande mais ca ne fait rien 

    std::vector<int> premiere_ligne (width, 0);
    std::vector<int> derniere_ligne (width, 0);

    std::vector<int> grand_buffer ((nbp)*pas*width, 0);
    int i = 0;
    while  ( i<pas*width)// on rajoute des cases vides a la fin // pour le processus 0 qui aura aussi une partie^^
    {
        grand_buffer[i] = 0;
        i++;
    } 
    while  ( i<width*height)
    {
        grand_buffer[i] = grande_galaxie.data()[i];
        i++;
    } 
    while  ( i<grand_buffer.size())// on rajoute des cases vides a la fin 
    {
        grand_buffer[i] = 0;
        i++;
    } 

    //on distribue les taches (sous-matrices) sur les différents processus
    MPI_Scatter(grand_buffer.data(), pas*width, MPI_INT, &g.data()[width], pas*width, MPI_INT, 0, globComm);

    int deltaT = (20*52840)/width;

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
    
        std::cout << "Pas de temps : " << deltaT << " années" << std::endl;

        std::cout << std::endl;

    }
    unsigned long long temps = 0;

    std::chrono::time_point<std::chrono::system_clock> start, end1, end2;
    
    if (rank!=0)
    {
        while (1)
        {   
            mise_a_jour(param, width, pas, g.data(), g_next.data());
            g_next.swap(g);

            //echange des info
            if (rank%2==0)//pour les noeuds pairs (hors 0)
            {
                //recoit sa première ligne de l'impair qui le précède
                MPI_Recv (&premiere_ligne[0], width, MPI_INT, rank-1, 0, globComm, 0 );
                //si pas dernier processus
                if (rank <=nbp-2)
                {
                    //envoie sa derniere ligne (fantome) au prochain impair
                    MPI_Send (&g.data()[(pas+1)*width],width, MPI_INT, rank+1, 0, globComm);
                }
                //si pas dernier processus
                if (rank <=nbp-2)
                {
                    //recoit sa derniere ligne du prochain impair
                    MPI_Recv (&derniere_ligne[0],width, MPI_INT, rank+1, 0, globComm, 0);
                }
                //envoie sa premiere ligne (fantome) a l'impair qui le précède
                MPI_Send (g.data(),width, MPI_INT, rank-1, 0, globComm); //rank -1 existe car rank pair et !=0

                //raccord avant et après selon les vecteurs fantomes qu'il a  recu 
                g.insert_vecteur_avant (&premiere_ligne, width);
                g.insert_vecteur_apres (&derniere_ligne, width);
            }

            if (rank%2==1)//pour les noeuds impairs
            {
                if (rank <=nbp-2)
                {
                    MPI_Send (&g.data()[(pas+1)*width],width, MPI_INT, rank+1, 0, globComm);
                }
                if (rank>1)
                {
                    MPI_Recv (&premiere_ligne[0], width, MPI_INT, rank-1, 0, globComm, 0 );
                }  
                if (rank>1)
                {
                    MPI_Send (g.data(),width, MPI_INT, rank-1, 0, globComm); //rank -1 existe car rank pair et !=0
                }                 
                if (rank <=nbp-2)
                {
                    MPI_Recv (&derniere_ligne[0],width, MPI_INT, rank+1, 0, globComm, 0);
                }                
                
                g.insert_vecteur_avant (&premiere_ligne, width);
                g.insert_vecteur_apres (&derniere_ligne, width);
            }

            //on reset les lignes fantomes
            g.reset_lignes_fantome();
            g_next.reset_lignes_fantome();
       
            //on rassemble les sous-matrices dans la grande matrice
            MPI_Gather (&g.data()[width], pas*width, MPI_INT, grand_buffer.data(), pas*width, MPI_INT, 0, globComm);
    
        }

        
        
    }
    //root
    else 
    {
        //affichage
        window = SDL_CreateWindow("Galaxie", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  width, height, SDL_WINDOW_SHOWN);
        galaxie_renderer gr(window);
        gr.render(g);
        while (1)
        {
            start = std::chrono::system_clock::now();
            //on rassemble les sous-matrices dans la grande matrice
            MPI_Gather (&g.data()[width], pas*width, MPI_INT, grand_buffer.data(), pas*width, MPI_INT, 0, globComm);
            end1 = std::chrono::system_clock::now();
            //on passe les données à m_planetes et on affiche
            grande_galaxie.update_data (&grand_buffer, width, pas);
            gr.render(grande_galaxie);
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
              SDL_DestroyWindow(window);
                SDL_Quit();
                //MPI_Abort(globComm);
              break;
            }
        }
    }
        
    
    SDL_DestroyWindow(window);
    SDL_Quit();
    MPI_Finalize ();
    return EXIT_SUCCESS;
}
