# Rapport du projet "Simulation d'une colonisation d'une galaxie" - Anthony AOUN

## Information sur ma machine

Architecture:                    x86_64
CPU op-mode(s):                  32-bit, 64-bit
Byte Order:                      Little Endian
Address sizes:                   39 bits physical, 48 bits virtual
CPU(s):                          4
On-line CPU(s) list:             0-3
Thread(s) per core:              2
Core(s) per socket:              2
Socket(s):                       1
NUMA node(s):                    1
Vendor ID:                       GenuineIntel
CPU family:                      6
Model:                           142
Model name:                      Intel(R) Core(TM) i7-7500U CPU @ 2.70GHz
Stepping:                        9
CPU MHz:                         588.090
CPU max MHz:                     3500.0000
CPU min MHz:                     400.0000

## Execution en séquentiel

Sur ma machine, l'execution séquentielle donne les résultats suivants : 

CPU(ms) : calcul 21.710  affichage 1.892

## Parallélisation de boucle en mémoire partagée

On utilise OpenMP sur la boucle for de la méthode de mise à jour dans le fichier parametres.cpp.
On constate qu'il y a data race sur sur l'appel de la fonction random; pour cela on crèe un random engine
commun à toute la zone parallèle pour générer des variables aléatoires passées comme paramètre d'entrée des
méthodes qui ont besoin d'en créer une.

## Recouvrement calcul / affichage en mémoire partagée
