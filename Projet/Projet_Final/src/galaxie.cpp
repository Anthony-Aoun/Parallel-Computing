#include <stdlib.h>
#include <iostream>
#include <SDL2/SDL_image.h>
#include "galaxie.hpp"

//_ ______________________________________________________________________________________________ _
galaxie::galaxie(int width, int height)
    :   m_width(width),
        m_height(height),
        m_planetes(width*height, habitable)
{}
//_ ______________________________________________________________________________________________ _
galaxie::galaxie(int width, int height, double chance_habitee)
    :   m_width(width),
        m_height(height),
        m_planetes(width*height)
{
    int i,j;
    for ( i = 0; i < height; ++i )
        for ( j = 0; j < width; ++j )
        {
            double val = std::rand()/(1.*RAND_MAX);
            if (val < chance_habitee)
            {
                m_planetes[i*width+j] = habitee;
            }
            else
                m_planetes[i*width+j] = habitable;
        }
}
//_ ______________________________________________________________________________________________ _
void 
galaxie::rend_planete_habitee(int x, int y)
{
    m_planetes[y*m_width + x] = habitee;
}
//_ ______________________________________________________________________________________________ _
void
galaxie::rend_planete_inhabitable(int x, int y)
{
    m_planetes[y*m_width + x] = inhabitable;
}

//_ ______________________________________________________________________________________________ _
void 
galaxie::print ()
{
    for (long unsigned int i = 0; i<m_planetes.size(); i++)
    {
        std::cout<<m_planetes[i]<< "; ";
    }
    std::cout <<std::endl;
}

//_ ______________________________________________________________________________________________ _
void
galaxie::rend_planete_inhabitee(int x, int y)
{
    m_planetes[y*m_width + x] = habitable;
}
//_ ______________________________________________________________________________________________ _
void
galaxie::swap(galaxie& g)
{
    g.m_planetes.swap(this->m_planetes);
}
//_ ______________________________________________________________________________________________ _
void galaxie::insert_vector (int position, std::vector<int>* my_vector)
{
    int taille = my_vector->size();
    int n = m_planetes.size ();
    for (int i = 0; i<taille; i++)
    {
        //std::cout << "insertion " << i+position ;
        if (i + position < n)
        {
            m_planetes[i+position] = my_vector->data ()[i];
            if (m_planetes[i+position]!=0 && i>436906)
                std::cout<< i+position<<"insere"<<std::endl;
            //std::cout<< "...inséré";
        }
        //std::cout<< std::endl;

    }
    //std::cout << "position " << position<<std::endl;
    
}
//_ ______________________________________________________________________________________________ _
void galaxie::insert_vector_fantome (int position, std::vector<int>* my_vector, int width)
{
    int taille = my_vector->size() - 2*width;
    int n = m_planetes.size ();
    //insertion normale...
    for (int i = 0; i<taille; i++)
    {
        //std::cout << "insertion " << i+position ;
        if (i + position < n)
        {
            m_planetes[i+position] = my_vector->data ()[i+width];
            // if (m_planetes[i+position]!=0 && i>436906)
            //     std::cout<< i+position<<"insere"<<std::endl;
            //std::cout<< "...inséré";
        }
        //std::cout<< std::endl;

    }
    //insertion des cases fantomes
    if (position>=width)// on regarde la ligne du bas (celle qui est en plus) NB: on ne peut que s'étendre sur des cases fantomes, pas les rendre inhabitables
    {
        for (int i = 0; i<width; i++)
        {
            if (i + position < n)
            {
                if (my_vector->data ()[i]!=0 && m_planetes[i+position-width] == 0)
                {
                    m_planetes[i+position-width] = my_vector->data ()[i];
                }
            }
        }
    }
    if (position<n - width)
    {
        for (int i = taille; i<taille+width; i++)
        {
            if (i + position < n)//redondant avec le premier if normalement
            {
                if (my_vector->data ()[i]!=0 && m_planetes[i+position-width] == 0)
                {
                    m_planetes[i+position] = my_vector->data ()[i];
                }
            }
        }
    }
    
}
//_ ______________________________________________________________________________________________ _
void galaxie::extract_vector (int position,  std::vector<int>* vecteur_cible)
{
    int taille = vecteur_cible->size ();
    int n = m_planetes.size();
    for (int i = 0; i<taille; i++)
    {
        if (i + position < n)
            (*vecteur_cible)[i] = m_planetes[i+position];
    }
}

//_ ______________________________________________________________________________________________ _
void galaxie::update_data (std::vector <int>* new_vector)
{
    m_planetes = *new_vector;
}


//# ############################################################################################## #
galaxie_renderer::galaxie_renderer(SDL_Window* win)
{
    m_renderer = SDL_CreateRenderer(win, -1, 0);
    IMG_Init(IMG_INIT_JPG);
    m_texture = IMG_LoadTexture(m_renderer, "data/galaxie.jpg");
}
//_ ______________________________________________________________________________________________ _
galaxie_renderer::~galaxie_renderer()
{
    SDL_DestroyTexture(m_texture);
    IMG_Quit();
    SDL_DestroyRenderer(m_renderer);
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::rend_planete_habitee(int x, int y)
{
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 64);// Couleur verte
    SDL_RenderDrawPoint(m_renderer, x, y);
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::rend_planete_inhabitable(int x, int y)
{
    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 64);// Couleur rouge
    SDL_RenderDrawPoint(m_renderer, x, y);
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::rend_planete_inhabitee(int x, int y)
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);// Couleur noire
    SDL_RenderDrawPoint(m_renderer, x, y);    
}
//_ ______________________________________________________________________________________________ _
void 
galaxie_renderer::render(const galaxie& g)
{
    int i, j;
    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    const int* data   = g.data();
    int   width  = g.width();
    int   height = g.height();

    for (i = 0; i < height; ++i )
        for (j = 0; j < width; ++j )
        {
            if (data[i*width+j] == habitee)
                rend_planete_habitee(j, i);
            if (data[i*width+j] == inhabitable)
                rend_planete_inhabitable(j, i);
        }

    SDL_RenderPresent(m_renderer);
}

