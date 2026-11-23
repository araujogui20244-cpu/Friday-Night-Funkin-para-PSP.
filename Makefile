TARGET = EBOOT
OBJS = src/main.o
INCDIR = src
CFLAGS = -O2 -G0 -Wall -I$(PSPDEV)/psp/include
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
BUILD_PRX = 1
PSP_FW_VERSION = 660
LIBS = -lSDL_image -lpng -ljpeg -lz -lSDL_mixer -lmodplug -lvorbisfile -lvorbis -logg -lmpg123 -lFLAC -lSDL -lpspgu -lpspvram -lpspaudio -lpsphprm -lpspirkeyb
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = GUD FNF PSP
PSPDEV ?= /usr/local/pspdev
PSPSDK ?= $(PSPDEV)/psp/sdk
include $(PSPSDK)/lib/build.mak
