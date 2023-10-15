# About the Project
This project started as a student project at Ensimag, aimed at creating a simple RISC-V OS. Over time, it evolved to include basic ext2 file system support and several system calls, similar to those found in POSIX (e.g., fork, exec, open, read, write, etc.). Please note that this OS is not perfect and contains known flaws, which would require a significant amount of time to address.

# Running DOOM on the OS
To run DOOM on this OS, follow these steps:

1. Create an image of the Docker container `barbem/risc-v_cep`.

2. Run the following command to create a container with graphics support:
   ```bash
   docker create --interactive --net host -e DISPLAY=$DISPLAY --tty -v <xinul_folder_location>:/home/ -v /tmp/.X11-unix:/tmp/.X11-unix --name projet_os_riscv_graphic barbem/risc-v_cep
   ```

3. Ensure Docker has access to the X Window System with the following command:
   ```bash
   xhost +local:docker
   ```

4. Start the container with the following command:
   ```bash
   sudo docker exec -it projet_os_riscv_graphic /bin/bash
   ```

5. Navigate to the `home/src` folder and run the following command to access a terminal inside the OS:
   ```bash
   make go
   ```

6. Now you can run the following commands: `copy_files` and then `xinuldoom`.

7. To view the screen output, run either `make vncrem` in another shell of the Docker container or on the host machine. If Remmina is not installed on a Debian system, you can install it with:
   ```bash
   apt install remmina
   ```

8. If everything works correctly, DOOM should run, and input will be sent to the shell window running DOOM. To shoot, press 'A' and use the controls to move.

9. To exit, use the 'Ctrl-Alt-X' command.

# Credits
Credits for this project go to the ensimag professors that made this projet, the mit XV6 project and the Operating Systemsbook by Tanenbaum.