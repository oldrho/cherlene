# Name
MODNAME			:= cherlene

# Build directories
MODULEDIR		:= /lib/modules/$(shell uname -r)
BUILDDIR		:= $(MODULEDIR)/build
KERNELDIR		:= $(MODULEDIR)/kernel
SRCDIR			:= $(PWD)/src
OUTDIR			:= $(PWD)/bin

# Module
obj-m			:= $(MODNAME).o
$(MODNAME)-y	+= src/module.o

# CFLAGS
ccflags-y		:= -I$(SRCDIR)
ccflags-y		+= $(MORE_CFLAGS)


all:
	$(MAKE) -C $(BUILDDIR) M=$(PWD) modules

debug:
	$(MAKE) -C $(BUILDDIR) M=$(PWD) MORE_CFLAGS="-DDEBUG" modules

clean:
	$(MAKE) -C $(BUILDDIR) M=$(PWD) clean
