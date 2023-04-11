#
# kernel Makefile
#
# See LICENSE for license details.
# 
# Maintainer : Mathieu Barbe <mathieu.barbe@grenoble-inp.org> - 2018
#
# To view commands use: make V=1
#

SHELL = /bin/sh
KERNEL_NAME ?= kernel.bin
BUILD_DIR ?= ../build
OBJ_DIR = $(BUILD_DIR)/kernel/objs
KERNEL_BIN ?= $(BUILD_DIR)/$(KERNEL_NAME)

TESTS_OPTIONS ?= WITH_MSG $(TEST_OPT) $(VIRT_OPT)

all: $(KERNEL_BIN)

#
# Architecture and hardware selection
#

DEFAULT_MACHINE = cep
MACHINE ?= $(DEFAULT_MACHINE)

#
# Standard librairies configuration
#

STDLIB_NAME ?= lib
include ../$(STDLIB_NAME)/Makefile
LIBS = $(LIB_TARGET) # Useful to compil stdlib from kernel Makefile


#
# User apps
#

BIN_DIR = $(BUILD_DIR)/user
include build/apps-integration.mk

#
# Source directories
#

KERNEL_SRC_DIRS = . $(sort $(dir $(wildcard */)))
KERNEL_SRC_DIRS := $(KERNEL_SRC_DIRS:/=)

# Specific machine driver
KERNEL_SRC_DIRS += drivers/$(MACHINE)

#
# Include headers directories
#

INCLUDE_DIRS = \
	$(LIB_INCLUDES) \
	. \
	boot/common \

#
# Kernel sources files
#

# Fetch BOOT_OBJS
include boot/boot.mk
OBJS = $(BOOT_OBJS)

KERNEL_SRC = $(foreach d, $(KERNEL_SRC_DIRS), $(wildcard $(d)/*.c))
KERNEL_ASM = $(foreach d, $(KERNEL_SRC_DIRS), $(wildcard $(d)/*.S))
OBJS += $(sort $(KERNEL_SRC:.c=.o) $(KERNEL_ASM:.S=.o))
OBJS := $(addprefix $(OBJ_DIR)/, $(OBJS))
DEPFILES = $(OBJS:.o=.d)

#
# Import specific architecture  parameters :
#

include ../riscv_flags.config
include ../toolchain.config

#
# CFlags
#

# Default optim and debug options
CFLAGS ?= -O0 -g

# common architecture option fetched from ../riscv_flags.config
CFLAGS += $(ARCH_CFLAGS)

# Add specific kernel options
CFLAGS += $(addprefix -I, $(INCLUDE_DIRS)) \

# Add tests options
CFLAGS += $(addprefix -D, $(TESTS_OPTIONS))

#
# LDFLAGS
#

# common architecture option fetched from ../riscv_flags.config
LDFLAGS += $(ARCH_LDFLAGS)

# Add specific kernel LD options
LD_SCRIPTS = boot/$(MACHINE)/default.lds
LDFLAGS += $(addprefix -T, $(LD_SCRIPTS))

#
# Targets
#

CONFIG_DEPENDENCIES = Makefile ../riscv_flags.config boot/boot.mk build/apps-integration.mk ../toolchain.config

# Remove every implicite rules
.suffixe:

$(KERNEL_BIN): $(LIBS) $(OBJS) $(LD_SCRIPTS) $(APPS_OBJS) $(APPS_OUT)/apps.lds
	$(call cmd,LD KERNEL -> $(@:../%=%), $(dir $@), \
	$(LD) $(CFLAGS) $(LDFLAGS) $(OBJS) $(APPS_OBJS) $(LIBS) -o $@)

$(OBJ_DIR)/%.o: %.S $(CONFIG_DEPENDENCIES)
	$(call cmd,AS KERNEL $<, $(dir $@), \
	$(AS) -c $< -o $@ $(CFLAGS) -MMD -MP)

$(OBJ_DIR)/%.o: %.c $(CONFIG_DEPENDENCIES)
	$(call cmd,CC KERNEL $<, $(dir $@), \
	$(CC) -c $< -o $@ $(CFLAGS) -MMD -MP)

# Include object dependencies generated previously by gcc
-include $(DEPFILES)

.PHONY: clean
clean:
	rm -f $(KERNEL_BIN)
	rm -rf $(OBJ_DIR)

#
# To view commands use: make V=1
#

ifdef V
cmd = @mkdir -p $2 ; echo "$3"; $3
else
cmd = @echo "$1"; mkdir -p $2 ; $3
endif



