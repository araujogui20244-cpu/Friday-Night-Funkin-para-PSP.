TARGET = EBOOT
OBJS = src/main.o
INCDIR = src
CFLAGS = -O2 -G0 -Wall -Dmain=main -I$(PSPDEV)/psp/include/SDL2 -I$(PSPDEV)/psp/include
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
BUILD_PRX = 1
PSP_FW_VERSION = 660
LIBS = -lSDL2_image -lpng -ljpeg -lz -lSDL2_mixer -lmodplug -lvorbisfile -lvorbis -logg -lmpg123 -lFLAC -lSDL2 -lpspgu -lpspvram -lpspaudio -lpsphprm -lpspgl
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = GUD FNF PSP
PSPDEV ?= /usr/local/pspdev
PSPSDK ?= $(PSPDEV)/psp/sdk
include $(PSPSDK)/lib/build.mak
