# Rapport du projet "Simulation d'une colonisation d'une galaxie" - Anthony AOUN

## Informations sur ma machine

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

L'execution séquentielle donne les résultats suivants : 

CPU(ms) : calcul 21.710  affichage 1.892

## Parallélisation de boucle en mémoire partagée

On utilise OpenMP sur la boucle for de la méthode de mise à jour dans le fichier parametres.cpp.

D'abord la graine utilisée par rand() et servant d'initialiser une suite à comportement aléatoire est une variable 
globale protégée pour le multithreading. Chaque appel de rand() utilise des mutex qui rendent la zone du code en 
question séquentielle en faisant passer les threads 1 à 1. Il y a une grosse pénalité pour notre code à mémoire partagée.

On constate par conséquent qu'il y a data race sur sur l'appel de la fonction rand(); pour cela on crèe un random engine
commun à toute la zone parallèle pour générer des variables aléatoires passées comme paramètre d'entrée des
méthodes qui ont besoin d'en créer une (il faut penser à faire #include < random >). De ce fait, les méthodes : 
calcul-expansion, calcul-depeuplement, calcul-inhabitable et apparition-technologie prennent un paramètre d'entrée 
supplémentaire qui est :
(double)(rand-thread()/(1.*RAND-MAX)) où rand-thread() est ujn objet de classe default-random-engine, 
prenant comme paramètre un objet de classe random-device qui produit des nombres aléatoires non déterministes
unioformément distribués selon un modèle stochastique.

plus d'infos sur default-random-engine : http://www.cplusplus.com/reference/random/default_random_engine/
plus d'infos sur random-device : https://www.cplusplus.com/reference/random/random_device/

L'execution après parallélisation de boucle en mémoire partagée donne à présent, pour 4 threads :

CPU(ms) : calcul 1.968  affichage 1.7388

Nous avons gagné considérablement en temps de calcul avec un speedup S(n) = 11.

Nous remarquons qu'à présent que la boucle de calcul et la boucle d’affichage prennent un temps similaire.

## Recouvrement calcul / affichage en mémoire partagée

On utilise des sections parallèles d'OpenMP dans le fichier colonisation.cpp.


