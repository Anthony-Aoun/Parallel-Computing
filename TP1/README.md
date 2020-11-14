# TP1

## Un hello World parallèle

* Quand le nombre de processus devient plus grand que le nombre de coeurs de notre machine, les executions ne se fontt plus dans l'ordre.
* Classer les résultats dans des fichiers permet de mieux visualiser l'output de l'execution pour chaque processus executé.

## Envoi bloquant et non bloquant

* La première version correspond à un envoi bloquant; donc ne rendra la main qu'après s’être assuré que modifier le buffer d’application n’altérera pas les données qui devront être reçues par la tâche destinataire. Pour cela on dit que c'est un envoi sûr. Cependant, la deuxième version est un envoi non bloquant; donc rendra la main immédiatement. Dans ce dernier cas, on attend pas que la communication soit finie c'est pour cela qu'on parle d'envoi dangereux. Dans le deuxième cas, il suffit de ramplacer MPI\_ISend par MPI\_Send et d'enlever le MPI_Wait.
* Dans ce cas, l'ordre est conservé grace à la la structure d'anneau mise en place qui fait que chaque processus a un sender et un receiver et le message circule dans un ordre bien défini. cette structure assure que chaque processus recoive un message avant d'en envoyer un autre.
* Les processus ne sont plus executés dans l'ordre car les porocessus n'attendent pas de recevoir un message avant d'envoyer le leur. 

## Calcul de pi par lancer de fléchettes




