#include <cstdlib>
#include <cassert>
#include <ctime>
#include <iostream>
#include <omp.h>
#include <random>
#include "galaxie.hpp"
#include "parametres.hpp"

expansion calcul_expansion(const parametres& c, double val)
{
    if (val < 0.01*c.expansion)     // parmi c.expansion, on a 1% de chance d'expansion isotrope...
        return expansion_isotrope;
    if (val < c.expansion)          // ... et 99% de chance d'expansion dans 1 seule direction
        return expansion_unique;
    return pas_d_expansion;
}
//_ ______________________________________________________________________________________________ _
bool calcul_depeuplement(const parametres& c, double val)
{
    if (val < c.disparition)
        return true;
    return false;   
}
//_ ______________________________________________________________________________________________ _
bool calcul_inhabitable(const parametres& c, double val)
{
    if (val < c.inhabitable)
        return true;
    return false;
}
//_ ______________________________________________________________________________________________ _
bool apparition_technologie(const parametres& p, double val)
{
    if (val < p.apparition_civ)
        return true;
    return false;
}
//_ ______________________________________________________________________________________________ _
bool a_un_systeme_proche_colonisable(int i, int j, int width, int height, const int* galaxie)
{
    assert(i >= 0);
    assert(j >= 0);
    assert(i < height);
    assert(j < width);

    if ( (i>0) && (galaxie[(i-1)*width+j] == habitable)) return true;
    if ( (i<height-1) && (galaxie[(i+1)*width+j] == habitable)) return true;
    if ( (j>0) && (galaxie[i*width+j-1] == habitable)) return true;
    if ( (j<width-1) && (galaxie[i*width+j+1] == habitable)) return true;

    return false;
}
//_ ______________________________________________________________________________________________ _
void 
mise_a_jour(const parametres& params, int width, int height_debut, int height_fin, int height,  int* galaxie_previous,  int* galaxie_next,  int* galaxie_previous_full)
{
    int i, j;
    int height_difference = height_fin - height_debut;
    memcpy(galaxie_next, galaxie_previous, width*height_difference*sizeof(int));
    // std::cout<< "tour"<<std::endl;
    // std::cout<<"parametres : " << params.apparition_civ << "; "
    //                             << params.disparition << "; "
    //                             << params.expansion << "; "
    //                             << params.inhabitable<< "; " << std::endl;
    omp_set_num_threads(1);
    #pragma omp parallel 
    {
        std::random_device thread_device;
        std::default_random_engine rand_thread(thread_device());
        #pragma omp for
        for ( i = height_debut; i < height_fin; ++i )
          {
            //int i_local = i - height_debut +1;
            for ( j = 0; j < width; ++j )
            {
                if (galaxie_previous_full[i*width+j] == 1)
                {
                    //std::cout <<"la case " << i << ", "<<  j << " est habitee; height_debut = "<< height_debut <<std::endl;
                    if ( a_un_systeme_proche_colonisable(i, j, width, height, galaxie_previous_full) )
                    {
                        expansion e = calcul_expansion(params, (double)(rand_thread()/(1.*RAND_MAX)));                        //if (height_debut!=0)
                            //std::cout <<"la case " << i << ", "<<  j << " a un systeme proche colonisable, e = "<< e  << "; height_debut = "<< height_debut<<std::endl;
                        if (e == expansion_isotrope)
                        {
                            //std::cout <<"la case " << i << ", "<<  j << "s'etend de maniere isotrope" <<std::endl;
                          if ( (i>0) && (galaxie_previous_full[(i-1)*width+j] != inhabitable) )
                            {
                                galaxie_next[((i - height_debut +1)-1)*width+j] = habitee;
                            }
                          if ( (i<height) &&  (galaxie_previous_full[(i+1)*width+j] != inhabitable) )
                            {
                                galaxie_next[((i - height_debut +1)+1)*width+j] = habitee;
                            }
                          if ( (j > 0) && (galaxie_previous_full[i*width+j-1] != inhabitable) )
                            {
                                galaxie_next[(i - height_debut +1)*width+j-1] = habitee;
                            }
                          if ( (j < width-1) && (galaxie_previous_full[i*width+j+1] != inhabitable) )
                            {
                                galaxie_next[(i - height_debut +1)*width+j+1] = habitee;
                            }
                            
                        }
                        else if (e == expansion_unique)
                        {
                            //std::cout <<"la case " << i << ", "<<  j << "s'etend de maniere non isotrope" <<std::endl;
                            // Calcul de la direction de l'expansion :
                            int ok = 0;
                            do
                            {
                                int dir = rand_thread()%4;
                                if ( (0 == dir) && (galaxie_previous_full[(i-1)*width+j] != inhabitable) )
                                {
                                    galaxie_next[((i - height_debut +1)-1)*width+j] = habitee;
                                    ok = 1;
                                }
                                if ( (1 == dir) && (galaxie_previous_full[(i+1)*width+j] != inhabitable) )
                                {
                                    galaxie_next[((i - height_debut +1)+1)*width+j] = habitee;
                                    ok = 1;
                                }
                                if ( (j>0) && (2 == dir) && (galaxie_previous_full[i*width+j-1] != inhabitable) )
                                {
                                    galaxie_next[(i - height_debut +1)*width+j-1] = habitee;
                                    ok = 1;
                                }
                                if ( (j<width-1) && (3 == dir) && (galaxie_previous_full[i*width+j+1] != inhabitable) )
                                {
                                    galaxie_next[(i - height_debut +1)*width+j+1] = habitee;
                                    ok = 1;
                                }
                            } while (ok == 0);
                        }// End if (e == expansion_unique)
                    }// Fin si il y a encore un monde non habite et habitable
                    if (calcul_depeuplement(params,(double)(rand_thread()/(1.*RAND_MAX))))
                    {
                        //std::cout <<"la case " << i << ", "<<  j << "se dépeuple" <<std::endl;
                        galaxie_next[(i - height_debut +1)*width+j] = habitable;
                    }
                    if (calcul_inhabitable(params, (double)(rand_thread()/(1.*RAND_MAX))))
                    {
                        //std::cout <<"la case " << i << ", "<<  j << "devient inhabitable" <<std::endl;
                        galaxie_next[(i - height_debut +1)*width+j] = inhabitable;
                    }
                }  // Fin si habitee
                else if (galaxie_previous_full[i*width+j] == habitable)
                {
                    if (apparition_technologie(params, (double)(rand_thread()/(1.*RAND_MAX))))
                        galaxie_next[(i - height_debut +1)*width+j] = habitee;
                }
                else { // inhabitable
                  // nothing to do : le systeme a explose
                }
                // if (galaxie_previous...)
            }// for (j)
          }// for (i)
    }

}
//_ ______________________________________________________________________________________________ _
