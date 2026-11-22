TARGET = EBOOT
OBJS = src/main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 660
LIBS = -lpspdebug -lpspdisplay -lpspctrl -lpspkernel

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = GUD FNF PSP

include $(PSPSDK)/lib/build.mak
