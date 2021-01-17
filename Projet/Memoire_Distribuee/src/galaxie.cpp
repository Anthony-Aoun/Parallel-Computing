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
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void 
galaxie::update_data(std::vector<int>* mynewvector, int width, int pas)
{
    int size = m_planetes.size();
    for (int i = 0; i<size; i++)
    {
        m_planetes[i] = mynewvector->data()[i + width*pas];
    }
}
//_ ______________________________________________________________________________________________ _
void 
galaxie::insert_vecteur_avant (std::vector<int>* my_vector, int width)
{
    for (int i = 0; i<width; i++)
    {
        //habitee et inhabitee en meme temps
        if (my_vector->data ()[i]==1 && m_planetes[i] == 0)// il n' y a qu'un cas ou ca change: si f=1 et existante = 0; on ne propage ni l'inhabitabilité ni la non habitabilité
        {
            m_planetes[i] = my_vector->data ()[i];
        }
    }
}
//_ ______________________________________________________________________________________________ _
void 
galaxie::insert_vecteur_apres (std::vector<int>* my_vector, int width)
{
    int size = m_planetes.size();
    for (int i = 0; i<width; i++)
    { 
        if (my_vector->data ()[i]==1 && m_planetes[i+size-width] == 0)// il n' y a qu'un cas ou ca change: si f=1 et existante = 0; on ne propageni l'inhabitabilité ni la non habitabilité
        {
            m_planetes[i+size-width] = my_vector->data ()[i];
        }
        
    }
}
//_ ______________________________________________________________________________________________ _
void
galaxie::reset_lignes_fantome ()
{
    int size = m_planetes.size();
    for (int i = 0; i<m_width; i++)
    {
        m_planetes[i] = habitable;
        m_planetes[i+size-m_width] = habitable;
    }
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
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
