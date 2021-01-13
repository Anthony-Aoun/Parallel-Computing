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

On a gagné considérablement en temps de calcul avec un speedup S(n) = 11.

On remarque à présent que la boucle de calcul et la boucle d’affichage prennent un temps similaire.

## Recouvrement calcul / affichage en mémoire partagée

On utilise OpenMP avec sections parallèles dans la boucle while(1) du main du fichier colonisation.cpp.

On utilise nowait qui fait que les threads ne vont pas s'attendre à la fin de la clause pour pouvoir commencer 
la mise à jour alors que l'affichage n'est pas encore terminé. On crèe deux sections dont chacune va être executée
par un thread. La première est celle de l'affochage, alors que la deuxième est celle de la mise à jour. La section de 
mise à jour est elle même multithreadée garce à la parallélisation effectuée dans la partie précédente.

L'execution après recouvrement des entrées/sorties par du calcul donne :

CPU(ms) : calcul 6.057  affichage 4.0176

On constate qu'on perd en accélération puisque le temps d'affichage a augmenté et le temps de calcul est plus long que
celui de la première partie.Il est possible que l'on peut pas accélérer le code puisque l'affichage prend trop de bande
passante mémoire et empêche par conséquent d'avoir une accélération pour calculer la nouvelle génération. Il y a donc 
memory bound.

## Parallélisation en mémoire distribuée

On utilise MPI avec plusieurs processus (taches) avec : un coordinateur (le root) et des travailleurs.

On utilisera le root pour afficher le résultat à l’écran et pour les autres processus, on découpera la grille de pixel 
en plusieurs morceaux, par tranches horizontales. On obtiendra alors des sous-grilles auxquelles on rajoutera sur chaque 
bord des cellules fantômes où on calcule si la planète est habitée, habitable ou inhabitable. Ceci permet aux cellules
du bord de connaitre l'état des cellules voisines.

Ainsi, une cellule “fantôme” de la première grille enverra donc son nouvel état à la seconde grille qui mettra sa cellule 
correspondante à jour. La mise à jour dans les zones critiques est assurée par la méthode insert-vector-fantome. La stratégie
des cellules fantomes permet donc le raccordement des domaines.

L'algorithme à mémoire distribuée travaille comme suit :

- Le processus root envoie la grande matrice à tous les autres processus et recoit les sous-matrice mises à jour. 
Il se charge de raccorde les sous-matrices, ajouter les cellules fantomes et de mettre à jour les zones critiques
pour que tout soit en accord (habitée, habitable ou inhabitable). Ce processus se charge aussi de l'affichage.

- Chaque autre processus recoit la grande matrice, met à jour la sous-matrice qui lui est attribuée et envoie le 
résultat au root.

L'execution après parallélisation en mémoire distribuée avec 4 processus donne :

CPU(ms) : calcul 19.510  affichage 2.416

Nous remarquons que nous n'avons pas gagné en temps de calcul (speedup = 1) et même perdu en temps d'affichage (speedup < 1).
Ceci peut être expliqué par le fait que ma machine a uniquement 2 coeurs et que l'échange de messages au sein d'MPI est
couteux. De même l'existence de zones séquentielles non parallélisables peut rendre difficile l'optimisation en temps de
calcul et d'affichage.

NB: L'envoi et la réception cellule par cellule n'est pas convenable car il a une mauvaise influence sur la granularité.
En effet, il ne faut pas que les sous-taches soient trop petites sinon il y aura beaucoup d'echanges et on perd en efficacité.

## Parallélisation en mémoire partagée et distribuée










