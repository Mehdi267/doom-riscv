# Started as a projet at ensimag to build a simple riscv os, and then i added a very basic ext2 support and many syscalls similiar to the ones present in posix (fork, exec, open, read, write, ....) the os is not perfect and it has many flaws as it would take significant amunt of time to fix everything

To run doom 
create an image of the docker container barbem/risc-v_cep
then run this comamnd :

docker create --interactive --net host -e  DISPLAY=$DISPLAY --tty -v <xinul_folder_location>:/home/ -v /tmp/.X11-unix:/tmp/.X11-unix --name projet_os_riscv_graphic barbem/risc-v_cep 

and don't forget to give docker access to the x window system
xhost +local:docker

then run the container
sudo docker exec -it  projet_os_riscv_graphic /bin/bash 

then go to the folder home/src 

run the command make go
logically now you will have access to a terminal inside the os 

to run the commands
copy_files
and then xinuldoom

now to view the screen output 
run in an other shell of the docker container make vncrem or in the host machine and if remmina is not installed install it using apt install  remmina on a debian system

Hopefully doom works and the input goes in the shell window running doom, to shoot press a and use controls to move

to leave run the crtl-atl-x command 