# Disable default suffixes
.SUFFIXES:

ifndef SOURCE

ifeq ($(strip $(DEVKITARM)),)
	$(error "DEVKITARM variable not defined")
endif

ifeq ($(strip $(SM64DSe)),)
	$(error "SM64DSe variable not defined")
endif

ROM := tkwsc.nds

ifeq ($(CODEADDR),0x02156AA0)

all:
	@mkdir -p $(CURDIR)/build/patches
	@make --no-print-directory SOURCE=source/patches BUILD=build/patches OUTPUT_DIR=$(CURDIR)

else

all:
	@echo targets: DLs, MOM, release
	@echo See README.md for build instructions.

endif

DL_SOURCES := $(notdir $(wildcard source/DLs/*))

.PHONY: $(DL_SOURCES) DLs MOM release

DLs: $(DL_SOURCES)
	"$(SM64DSe)" insertDLs --rom=$(ROM) build/DLs dl_targets.txt

MOM:
	@echo building MOM ...

	@mkdir -p $(CURDIR)/build/MOM
	@make --no-print-directory SOURCE=source/MOM BUILD=build/MOM CODEADDR=0x023C4000

	"$(SM64DSe)" compile OVERLAY build/MOM 155 --rom=$(ROM) --no-run-make

release:
	python build_release.py

# Call make recursively for each DL
$(DL_SOURCES):
	@echo building DL \'$@\' ...

	@mkdir -p $(CURDIR)/build/DLs/$@
	@make --no-print-directory SOURCE=source/DLs/$@ BUILD=build/DLs/$@ IS_DL=1

.PHONY: clean

clean:
	@echo clean ...
	@rm -fr build

else

PORTLIBS := $(DEVKITPRO)/portlibs/arm
PATH     := $(DEVKITARM)/bin:$(PORTLIBS)/bin:$(PATH)
LIBNDS   := $(DEVKITPRO)/libnds

PREFIX := arm-none-eabi-

CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)as
AR      := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
LD      := $(PREFIX)ld

SOURCES  := source/libc $(SOURCE)
INCLUDES := include include/MOM source SM64DS-PI/include

LIBS := 
LIBDIRS := $(LIBNDS)  $(DEVKITARM) $(DEVKITARM)/arm-none-eabi

LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(DEVKITARM)/lib/gcc/arm-none-eabi/11.1.0
INCLUDE  := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
            $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)

ARCHFLAGS := -march=armv5te -mtune=arm946e-s

CFLAGS := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-narrowing \
	-Wno-parentheses -Wno-volatile -Wno-invalid-offsetof -Wno-char-subscripts -Wno-trigraphs \
	-Os $(ARCHFLAGS) -fomit-frame-pointer -fwrapv \
	$(INCLUDE) -DARM9 -c

CXXFLAGS := $(CFLAGS) -std=c++23 -fno-exceptions -fno-rtti -fno-threadsafe-statics -faligned-new=4

SYMBOLS = $(CURDIR)/SM64DS-PI/symbols9.x
LDFLAGS = --gc-sections -T $(SYMBOLS) -T $(CURDIR)/linker.x

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

OFILENAMES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
OFILES = $(foreach name,$(OFILENAMES),$(BUILD)/$(name))

OUTPUT_DIR := $(DEPSDIR)

ifdef IS_DL

OUTPUT_LO := $(OUTPUT_DIR)/newcode_lo
OUTPUT_HI := $(OUTPUT_DIR)/newcode_hi

BINFILES = $(OUTPUT_LO).bin $(OUTPUT_HI).bin
SYMFILES = $(OUTPUT_LO).sym $(OUTPUT_HI).sym

else

OUTPUT := $(OUTPUT_DIR)/newcode
BINFILES = $(OUTPUT).bin
SYMFILES = $(OUTPUT).sym

endif

all: $(BINFILES) $(SYMFILES)

$(OUTPUT_DIR)/%.bin : $(OUTPUT_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)

$(OUTPUT_DIR)/%.sym : $(OUTPUT_DIR)/%.elf
	$(OBJDUMP) -t $< > $@
	@echo written the symbol table ... $(notdir $@)

ifdef IS_DL

$(OUTPUT_LO).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext 0x02400000 $(OFILES) $(LIBPATHS) $(LIBS) -o $@

$(OUTPUT_HI).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext 0x02400004 $(OFILES) $(LIBPATHS) $(LIBS) -o $@

else

$(OUTPUT).elf: $(OFILES)
	@echo linking $(notdir $@)
	$(LD) $(LDFLAGS) -Ttext $(CODEADDR) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

endif

$(BUILD)/%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

$(BUILD)/%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

$(BUILD)/%.o: %.s
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ARCHFLAGS) -c $< -o $@ $(ERROR_FILTER)

-include $(DEPSDIR)/*.d

endif
