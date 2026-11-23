.SUFFIXES:
ifeq ($(strip $(DEVKITARM)),)
$(error "DEVKITARM não está definido. Use a imagem oficial devkitPro.")
endif

TARGET := GUD_FNF_DS
BUILD := build
SOURCES := source
INCLUDES := include
GAME_TITLE := GUD FNF
GAME_SUBTITLE1 := Rhythm game homebrew
GAME_SUBTITLE2 := by ivtguioliveira8

include $(DEVKITARM)/ds_rules

ARCH := -march=armv5te -mtune=arm946e-s -mthumb
CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections -fdata-sections $(ARCH) $(INCLUDE) -DARM9
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := -lnds9
LIBDIRS := $(DEVKITPRO)/libnds

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export OFILES := $(CFILES:.c=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
.PHONY: all clean
all:
	@mkdir -p $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).nds $(TARGET).ds.gba
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).nds: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -MF $*.d -c $< -o $@
-include $(DEPENDS)
endif
