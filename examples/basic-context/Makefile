# Benoît Wallon <benoit.wallon@grenoble-inp.org> - 2019
# Mathieu Barbe <mathieu@kolabnow.com> - 2019
# Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr> - 2022
# See license for license details.
# a minimal example to test X-compile in host or in Docker container

DK_IMAGE_NAME = barbem/risc-v_cep
DK_DOCKERFILE_DIR=../../docker
DK_MNT_DIR=/mnt/host
DK_RUN_CMD=docker run --rm -v ${PWD}:${DK_MNT_DIR} -w ${DK_MNT_DIR} ${DK_IMAGE_NAME}

include ../../src/toolchain.config
include ../../src/riscv_flags.config

# Do not use -O0 as it uses s0 although I try to prevent it
CFLAGS=-gdwarf -O2 ${ARCH_CFLAGS} -I../../src/lib/include -ffreestanding -ffixed-s0
LDSCRIPT=default.ld
LDFLAGS=${ARCH_LDFLAGS} -ffreestanding
KERNEL=main

all: $(KERNEL)

dk-pull:${DK_DOCKERFILE_DIR}/Dockerfile
	docker pull ${DK_IMAGE_NAME}

dk-all:
	${DK_RUN_CMD} make all

crtm.o:crtm.S
	${CC} ${CFLAGS} -c $<
context_switch.o:context_switch.S
	${CC} ${CFLAGS} -c $<
main.o:main.c
	${CC} ${CFLAGS} -c $<
main:main.o context_switch.o crtm.o
	${CC} ${LDFLAGS} -T ${LDSCRIPT} $^ -o $@

clean:
	${RM} $(KERNEL) main.o context_switch.o crtm.o

MACHINE ?= cep

#
# Hardware plateforme
#

ZYBO_IP = 192.168.1.5
ZYBO_USER = root
ZYBO_MDP = root

#
# GDB
#

GDB_TARGET_HOST = localhost
GDB_TARGET_PORT = 1234
GDB_TARGET_REMOTE = $(GDB_TARGET_HOST):$(GDB_TARGET_PORT)
GDBINIT_FILE = ./gdbinit_mehdi
GDB_OPTS = -q \
    -ex "target remote $(GDB_TARGET_REMOTE)" \
    -ex "source $(GDBINIT_FILE)"

#
# Qemu
#

QEMU = $(RISCV_ROOT)qemu-system-riscv64
ifeq ("$(MACHINE)", "spike")
	QEMU_MACHINE=spike_v1.10
else ifeq ("$(MACHINE)", "sifive_u")
	QEMU_MACHINE=sifive_u
else ifeq ("$(MACHINE)", "virt")
	QEMU_MACHINE=virt
else ifeq ("$(MACHINE)", "cep")
	QEMU_MACHINE=cep
else
	$(error Machine unknown)
endif

#Starting with QEMU 4.1.0, we must add the '-bios none' flag
QEMU_OPTS = -machine $(QEMU_MACHINE) -kernel $(KERNEL) -rtc base=localtime -k fr -m 32M
QEMU_GDB_OPTS = -s -S
# Pour afficher l'affichage série en console, marche à travers ssh
QEMU_NO_GRAPH_OPTS = -nographic
# Pour avoir l'uart + le VGA en mode graphique
QEMU_GRAPH_OPTS = -sdl -serial mon:stdio

#
# Simulation
#

# go :Lance l'OS avec QEmu dans le terminal
go: all
	$(QEMU) $(QEMU_OPTS) $(QEMU_NO_GRAPH_OPTS)

# debug : Lance l'OS dans un terminal avec QEmu et attend gdb.
debug: all
	$(QEMU) $(QEMU_OPTS) $(QEMU_GDB_OPTS) $(QEMU_NO_GRAPH_OPTS)

# go-graphic : Lance l'OS avec QEmu en mode graphique
go_graphic: all
	$(QEMU) $(QEMU_GRAPH_OPTS) $(QEMUOPTS)

# kill QEmu si CTRL-A, X ne fonctionne pas.
kill_qemu:
	killall -9 $(QEMU) 

# gdb : Lancement de GDB avec connection automatique et lecture de GDBINIT_FILE
gdb:
	$(GDB) $(GDB_OPTS) $(KERNEL)
