# TP2

`pandoc -s --toc tp3.md --css=./github-pandoc.css -o tp3.html`

## Question du cours

1. Scénario où il n'y a pas d'interblocage : le premier processus fait une réception synchrone suivie d'un envoi synchrone et le deuxième processus fait un envoi synchrone suivi d'une reception synchrone.
2. Scénario où il y a pas d'interblocage : tous les processus envoient message bloquant à un autre processus avant d’effectuer une réception.

Au total, pour 2 processus il y a 4 cas dont 2 où il y a inteblocage et 2 où il n'y en a pas. On a donc une probabilité de 50%.

## Question du cours num. 2

* Quand n est très grand, on a S(n) qui vaut environ 1/f. Or dans ce cas on a (1-f) = 0.9 => f = 0.1 = > S(n) = 10.
* Il faut mettre au maximum 10 car le 11ème ne servira à rien.
* On a (s + np) = S(n) = 4 (quand nnest grand) ce qui nous donne np = 0.25. Après avoir doublé le nombre de données ont a donc : (s + 2np) = (0.25 + 2*3.75) = 7.75. La nouvelle accélération maximale est de 7.75. 

## Ensemble de mandelbrot 

* Pour répartir les lignes équitablement sur les processus, j'ai donné à chaque processus la partie entière de la division du nombre de lignes par le nombre de processus, puis j'ai donné ce qui reste au dernier processus. Ensuite, pour chaque processus, j'ai traité un bloc qui est gathered par le processus root (ici = 0).

* Pour 5 processus : 45s.
Pour 20 processus : 2min, 62s.
Le temps séquentiel est d'1min, 30s.

S(5) = 90/45 = 2.

Quand on augmente beaucoup le nombre de processus, le temps pour calculer chaque tache devient plus grand, et on prend plus longtemps que si on avait tout en séquentiel.

* Dans la stratégie maitre-esclave, le processus 0 donne une ligne à chaque processus esclave, l'esclave la traite puis l'envoie à 0. Ensuite, 0 envoie une nouvelle ligne à traiter à chaque processus qui a terminé sa tache. Dès qu'on a finit, le processus 0 envoie un signal de terminaison.  
 


## Produit matrice-vecteur



*Expliquer la façon dont vous avez calculé la dimension locale sur chaque processus, en particulier quand le nombre de processus ne divise pas la dimension de la matrice.*
