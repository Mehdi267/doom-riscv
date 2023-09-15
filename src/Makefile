#
# Top Makefile
# 
# Mathieu Barbe <mathieu.barbe@grenoble-inp.org> - 2018 - 2020
# See LICENSE for license details.
#
# To view commands use: make V=1
#

KERNEL_NAME = kernel.bin
STDLIB_NAME = lib
BUILD_DIR = build
KERNEL_BIN = $(BUILD_DIR)/$(KERNEL_NAME)

#Preprocessing tags
TEST_OPT=NULL_TEST
VIRT_OPT=NULL_VIRT

#Disk variables
#in order to create the disk image using this command :
# qemu-img create -f raw xinul_disk.img 25M
#Set this value to virt if the disk is present in not
#then this value must set to any other value
DISK_VIRT ?= virt

#
# Import GNU toolchain configuration
#

include toolchain.config

#
# Machine
#

MACHINE ?= virt
#MACHINE ?= sifive_u
#MACHINE ?= spike
#MACHINE ?= zybo
# MACHINE ?= cep

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

#Starting with QEMU 4.1.0, we must add the '-bios none' flag
QEMU_OPTS = -machine $(QEMU_MACHINE) -kernel $(KERNEL_BIN)  -rtc base=localtime -k fr  
QEMU_CEP_MEM = -m 32M
QEMU_VIRT_MEM = -m 128M
QEMU_NO_BIOS = -bios none
QEMU_GDB_OPTS = -s -S
#in order to create the disk image using this command :
# qemu-img create -f raw xinul_disk.img 25M
QEMU_VIRTIO_CONF = -global virtio-mmio.force-legacy=false 
QEMU_OPT_DISK = -drive file=xinul_disk.img,if=none,format=raw,id=x0
QEMU_OPT_LINK_BLOCK = -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

# Pour afficher l'affichage série en console, marche à travers ssh
QEMU_NO_GRAPH_OPTS = -nographic
# Pour avoir l'uart + le VGA en mode graphique
# QEMU_GRAPH_OPTS = --enable-sdl --with-sdlabi=2.0 -display sdl -serial mon:stdio
#These work but there is they are not easy to manipulate and 
#and not well documented
# QEMU_GRAPH_OPTS = -vga std -serial mon:stdio
# QEMU_GRAPH_OPTS = -device bochs-display -serial mon:stdio
# QEMU_GRAPH_OPTS = -device isa-vga -serial mon:stdio
# QEMU_GRAPH_OPTS = -device virtio-vga -serial mon:stdio
QEMU_GRAPH_OPTS = -device virtio-gpu-device,bus=virtio-mmio-bus.1 -serial mon:stdio

QEMU = $(RISCV_ROOT)qemu-system-riscv64
ifeq ("$(MACHINE)", "spike")
	QEMU_MACHINE=spike_v1.10
else ifeq ("$(MACHINE)", "sifive_u")
	QEMU_MACHINE=sifive_u
else ifeq ("$(MACHINE)", "virt")
	QEMU_MACHINE=virt
	QEMU_OPTS+=$(QEMU_NO_BIOS)
	QEMU_OPTS+=$(QEMU_VIRT_MEM)
	ifeq ("$(DISK_VIRT)", "virt")
		QEMU_OPTS += $(QEMU_VIRTIO_CONF)
		QEMU_OPTS += $(QEMU_OPT_DISK)
		QEMU_OPTS += $(QEMU_OPT_LINK_BLOCK)
	endif
	VIRT_OPT=VIRTMACHINE
else ifeq ("$(MACHINE)", "cep")
	QEMU_MACHINE=cep
	QEMU_OPTS+=$(QEMU_CEP_MEM) 
else
	$(error Machine unknown)
endif



# Docker
#

DK_MAKEFILE_DIR = ../docker/
DK_GATEWAY = 172.17.0.1

#
# Targets
#

.PHONY: clean kernel $(KERNEL_BIN) user lib


test : clean all 
	$(QEMU) $(QEMU_OPTS) $(QEMU_NO_GRAPH_OPTS)

test : TEST_OPT=TESTING

all $(KERNEL_BIN) : kernel

kernel: user
	make MACHINE=$(MACHINE) TEST_OPT=$(TEST_OPT) VIRT_OPT=$(VIRT_OPT) -C kernel/ all

user:
	make TEST_OPT=$(TEST_OPT) -C user

lib:
	make -C $(STDLIB_NAME)

clean:
	rm -rf $(BUILD_DIR)

#
# Simulation
#

# go :Lance l'OS avec QEmu dans le terminal
go: all
	$(QEMU) $(QEMU_OPTS) $(QEMU_GRAPH_OPTS) 
#$(QEMU_NO_GRAPH_OPTS)

# debug : Lance l'OS dans un terminal avec QEmu et attend gdb.
debug: all
	$(QEMU) $(QEMU_OPTS) $(QEMU_GDB_OPTS) $(QEMU_GRAPH_OPTS) 
#$(QEMU_NO_GRAPH_OPTS)

# go-graphic : Lance l'OS avec QEmu en mode graphique
go_graphic: all
	$(QEMU) $(QEMU_GRAPH_OPTS) $(QEMU_OPTS)

# kill QEmu si CTRL-A, X ne fonctionne pas.
kill_qemu:
	killall -9 $(QEMU) 

vnc:
	remmina -c vnc://127.0.0.1:5900 

# gdb : Lancement de GDB avec connection automatique et lecture de GDBINIT_FILE
gdb:
	$(GDB) $(GDB_OPTS) $(KERNEL_BIN)

#
# Target Docker
#

dk-image-dl dk-image-pull: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) pull

dk-image-build: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) build

dk-bash: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) bash

dk-all: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) CMD="make all" exec

dk-go: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) CMD="make go" exec

dk-test: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) CMD="make test" exec

dk-clean: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) CMD="make clean" exec

dk-debug: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) DK_OPTS="-p $(GDB_TARGET_PORT):$(GDB_TARGET_PORT)" CMD="make debug" exec

dk-gdb: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) CMD="make GDB_TARGET_HOST=$(DK_GATEWAY) gdb" exec


dk-image-rm: $(DK_MAKEFILE_DIR)/Makefile
	make -C $(DK_MAKEFILE_DIR) CMD="make clean" exec

#
# Targets pour plateforme Zybo
#

zybo: user
	make MACHINE=zybo -C kernel/ all

zybo_ssh:
	ssh $(ZYBO_USER)@$(ZYBO_IP)

zybo_copy: zybo
	scp $(KERNEL_BIN) $(ZYBO_USER)@$(ZYBO_IP):

zybo_go:zybo_copy
	ssh $(ZYBO_USER)@$(ZYBO_IP) ./fesvr-zynq $(KERNEL_NAME)

#
# Machine Spike (Simulateur fonctionel)
#

spike: user
	make MACHINE=spike -C kernel/ all

spike_go: spike
	spike $(KERNEL_BIN)

spike_qemu:
	make MACHINE=spike -C . go
