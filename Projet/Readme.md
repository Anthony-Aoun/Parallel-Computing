# Rapport du projet "Simulation d'une colonisation d'une galaxie" - Anthony AOUN

## Informations sur ma machine

Architecture:                    x86_64 \
CPU op-mode(s):                  32-bit, 64-bit \
Byte Order:                      Little Endian \
Address sizes:                   39 bits physical, 48 bits virtual \
CPU(s):                          4 \
On-line CPU(s) list:             0-3 \
Thread(s) per core:              2 \
Core(s) per socket:              2 \
Socket(s):                       1 \
NUMA node(s):                    1 \
Vendor ID:                       GenuineIntel \
CPU family:                      6 \
Model:                           142 \
Model name:                      Intel(R) Core(TM) i7-7500U CPU @ 2.70GHz \
Stepping:                        9 \
CPU MHz:                         588.090 \
CPU max MHz:                     3500.0000 \
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

L'execution après parallélisation de boucle en mémoire partagée donne à présent :

- pour 2 threads : CPU(ms) : calcul 3.227  affichage 1.708
- pour 3 threads : CPU(ms) : calcul 2.453  affichage 1.736
- pour 4 threads : CPU(ms) : calcul 1.968  affichage 1.738

On a gagné considérablement en temps de calcul avec un speedup S(n) = 11 pour 4 threads.

Il faut noter qu'à partir de 5 threads, le temps de calcul se stabilise à environ 2ms. L'optimisation maximale est 
donc atteinte pour 4 threads.
 
On remarque à présent que la boucle de calcul et la boucle d’affichage prennent un temps similaire.

## Recouvrement calcul / affichage en mémoire partagée

On utilise OpenMP avec sections parallèles dans la boucle while(1) du main du fichier colonisation.cpp.

On utilise nowait qui fait que les threads ne vont pas s'attendre à la fin de la clause pour pouvoir commencer 
la mise à jour alors que l'affichage n'est pas encore terminé. On crèe deux sections dont chacune va être executée
par un thread. La première est celle de l'affochage, alors que la deuxième est celle de la mise à jour. La section de 
mise à jour est elle même multithreadée garce à la parallélisation effectuée dans la partie précédente.

L'execution après recouvrement des entrées/sorties par du calcul donne :

- pour 2 threads : CPU(ms) : calcul 5.852  affichage 3.8912
- pour 3 threads : CPU(ms) : calcul 5.844  affichage 3.683
- pour 4 threads : CPU(ms) : calcul 6.057  affichage 4.0176

On constate qu'on perd en accélération puisque le temps d'affichage a augmenté et le temps de calcul est plus long que
celui de la première partie. Il est possible que l'on ne peut pas accélérer le code puisque l'affichage prend trop de bande
passante mémoire et empêche par conséquent d'avoir une accélération pour calculer la nouvelle génération. Il y a donc 
memory bound.

## Parallélisation en mémoire distribuée

On utilise MPI avec plusieurs processus (taches) avec : un coordinateur (le root) et des travailleurs.

On utilisera le root pour afficher le résultat à l’écran et pour les autres processus, on découpera la grille de pixel 
en plusieurs morceaux, par tranches horizontales. On obtiendra alors des sous-grilles auxquelles on rajoutera sur chaque 
bord des cellules fantômes où on calcule si la planète est habitée, habitable ou inhabitable. Ceci permet aux cellules
du bord de connaitre l'état des cellules voisines.

Par exemple, une cellule “fantôme” de la première grille enverra son nouvel état à la seconde grille qui mettra sa cellule 
correspondante à jour. La mise à jour dans les zones critiques est assurée par les méthodes insert-vecteur-avant et
insert-vecteur-apres . La stratégie des cellules fantomes permet donc le raccordement des domaines.

L'algorithme à mémoire distribuée travaille comme suit :

- La grande matrice est découpée en nbp tranches (où nbp est le nombre de processus) et chaque tranche est envoyée à un processus.
La partie recue par le root ne sera pas traitée. 

- Chaque processus qui n'est pas root recoit la sous-matrice, la met à jour (on modifie les indices de height dans la méthode 
mise-a-jour), recoit sa première et dernière ligne des cellules fantomes des processus qui sont en dessus et en dessous (s'ils 
existent), et ses cellules fantomes revoient à leur tour les informations aux autres sous-matrices. On oublie pas de raccorder 
les zones critiques après reception des cellules fantomes. Finalement on demande un Gather des sous-matrices au niveau du root. 

- Le processus root recoit les différententes sous-matrices qui sont raccordées grace à Gather. Puis, le root se charge uniquement 
de l'affichage.


L'execution après parallélisation en mémoire distribuée donne :

- pour 2 processus : CPU(ms) : calcul 22.526  affichage 3.403
- pour 3 processus : CPU(ms) : calcul 15.695  affichage 3.7012
- pour 4 processus : CPU(ms) : calcul 10.145  affichage 3.5988

Au delà de 4 processus, le temps de calcul devient égal vire supérieure au temps de calcul séquentiel. Ce comportement est normal
vu que ma machine a 4 processeurs.
Nous remarquons que nous avons gagné en temps de calcul (speedup = 2 pour 4 processus) mais perdu en temps d'affichage (speedup < 1).
Ceci peut être expliqué par le fait que ma machine a uniquement 2 coeurs et que l'échange de messages au sein d'MPI est
couteux. De même l'existence de zones séquentielles non parallélisables peut rendre difficile l'optimisation en temps de
calcul et d'affichage.

NB: L'envoi et la réception cellule par cellule n'est pas convenable car il a une mauvaise influence sur la granularité.
En effet, il ne faut pas que les sous-taches soient trop petites sinon il y aura beaucoup d'echanges et on perd en efficacité.

## Parallélisation en mémoire partagée et distribuée

À présent, on utilise la parllélisation de boucle en mémoire partagée de la première partie avec la parallélisation en mémoire 
distribuée. Il faut de même remplacer MPI-Init par MPI-Init-thread puis modifier CXX = mpic++ -fopenmp dans Make-linux.inc.

L'execution avec 3 processus et 2 threads donne :

CPU(ms) : calcul 3.074  affichage 6.29365

Nous avons ainsi un speedup de 7 pour le calcul mais une dégradation du temps d'affichage. Ceci peut être expliqué par la perte de
temps pour échanger les messages ce qui attarde l'affichage.










