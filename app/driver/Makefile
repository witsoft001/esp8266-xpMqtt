
ifndef PDIR
#指定名字为 libdriver.a
GEN_LIBS = libdriver.a
endif

INCLUDES := $(INCLUDES) -I $(PDIR)include
INCLUDES += -I ./
INCLUDES += -I ../../include/ets
PDIR := ../$(PDIR)
sinclude $(PDIR)Makefile

