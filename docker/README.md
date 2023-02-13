# Docker PCSEA RISC-V

L'image Docker attachée au projet PCSEA RISC-V permet de profiter
rapidement d'un environnement de cross-compilation  et de simulation fonctionnel pour
l’architecture RISC-V.

Avec cette image il est possible de :

* Cross-compiler le système d'exploitation, parties user et kernel;
* simuler le kernel avec Qemu;
* débuger le kernel avec GDB et Qemu;
* cross-compiler un programme RISC-V elf et l’exécuter sur une machine
host grâce à Spike et PK;
* lancer un bash pour utiliser l'environnement librement.


Cette image pourra également servir dans un second temps pour réaliser des
tests automatiques à travers le pipeline de Gitlab. Avec cette solution,
il serait possible de vérifier la non régression après chaque commit.

Cette image permet avant tout une compilation croisée et un lancement
rapide de l'OS sans altérer le système invité.

## Dépendances

* Programme Docker;

## Image RISC-V

Cette image est basée sur Debian et contient :

* riscv-gnu-toolchain
* riscv-spike
* riscv-fesvr
* riscv-qemu

## Makefile

* build : construit l'image Docker depuis le fichier Dockerfile. Va
télécharger Debian puis clonner et compiler les sources.
* dl ou pull : télécharge l'image depuis le site de Docker hub (<2Go)
mais pas besoins de compiler. Vous aurez les version des programmes au
moment où l'image a été générée et la compilation du projet a été vérifiée.
* CMD=ma_commande exec : Exécute la commande CMD dans un container Docker
* bash : lance un bash dans un container Docker;
* clean : supprime l'image RISC-V dans Docker.

Dans le dossier src du projet, le Makefile contient également des
targets particulières dans le but de compiler, télécharger et lancer la
compilation du projet.

## Manipulation sans Makefile
### Création de l'image

Ce placer dans le dossier docker puis:

    docker image build -t riscv .

Vous pouvez aller boire un café (un allongé) !


### Lancement du conteneur

Pour lancer le conteneur Docker:

    docker container run -ti --name env-riscv riscv /bin/bash

### Clean

Supprimer le conteneur:

    docker container rm env-riscv

Supprimer l'image:

    docker image rm riscv

### Custom docker container : 

### Creation du container et l'attacher au src dans votre machine

docker create --interactive --tty -v <projet_directory>:/home/ /tmp/.X11-unix:/tmp/.X11-unix --name projet_os_riscv barbem/risc-v_cep 

### Lancer le container 

docker start  projet_os_riscv

### Ouvrir bash dans le container

sudo docker exec -it projet_os_riscv /bin/bash 


## Exemple
docker create --interactive --net host -e  DISPLAY=$DISPLAY --tty -v /home/frikha/Desktop/semestre_8/projet_os/xinul:/home/ -v /tmp/.X11-unix:/tmp/.X11-unix --name projet_os_riscv_graphic barbem/risc-v_cep 

sudo docker exec -it  projet_os_riscv_graphic /bin/bash 


