# Makefile to rebuild SM64 split image

### Default target ###

default: all

### Build Options ###

# These options can either be changed by modifying the makefile, or
# by building with 'make SETTING=value'. 'make clean' may be required.

# Version of the game to build
VERSION ?= us
# Graphics microcode used
GRUCODE ?= f3d_old
# If NON_MATCHING is 1, define the NON_MATCHING and AVOID_UB macros when building (recommended)
NON_MATCHING ?= 1
# Build for Emscripten/WebGL
TARGET_WEB ?= 0
# Build for DOS
TARGET_DOS ?= 0

# NSP
TARGET_NSP ?= 1
# HOST=1: headless native build of the NSP code paths (see tools/host_shim); implies BENCH=1
HOST ?= 0
# ARMSIM=1: the benchmark built with the calculator's compiler and flags, run on QEMU's ARM926
# board instead of a calculator (see tools/armsim/run.sh); implies BENCH=1
ARMSIM ?= 0
NSP_PREFIX := nspire-

# Compiler to use (ido or gcc)
COMPILER ?= gcc

# DirectX11
ENABLE_DX11 ?= 0
# DirectX12
ENABLE_DX12 ?= 0
# GL2.1 / GLES2
ENABLE_OPENGL ?= 0
# Legacy OGL
ENABLE_OPENGL_LEGACY ?= 0
# Software rasterizer
ENABLE_SOFTRAST ?= 1
# Pick GL backend for DOS: osmesa, dmesa
DOS_GL := osmesa

# Automatic settings only for ports

  NON_MATCHING := 1
  GRUCODE := f3dex2
  TARGET_WINDOWS := 0
  ifeq ($(TARGET_WEB),0)
	ifeq ($(TARGET_DOS),0)
	  ifeq ($(OS),Windows_NT)
		TARGET_WINDOWS := 1
	  else
		# TODO: Detect Mac OS X, BSD, etc. For now, assume Linux
		TARGET_LINUX := 1
	  endif
	endif
  endif

  ifeq ($(TARGET_WINDOWS),1)
	# On Windows, default to DirectX 11
	ifeq ($(ENABLE_OPENGL)$(ENABLE_OPENGL_LEGACY)$(ENABLE_DX12)$(ENABLE_SOFTRAST),0000)
	  ENABLE_DX11 ?= 1
	endif
  else ifeq ($(ENABLE_OPENGL_LEGACY)$(ENABLE_SOFTRAST),00)
	# On others, default to OpenGL
	ENABLE_OPENGL ?= 1
  endif

  # Sanity checks
  ifeq ($(ENABLE_DX11),1)
	ifneq ($(TARGET_WINDOWS),1)
	  $(error The DirectX 11 backend is only supported on Windows)
	endif
	ifeq ($(ENABLE_OPENGL),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_OPENGL_LEGACY),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX12),1)
	  $(error Cannot specify multiple graphics backends)
	endif
  endif
  ifeq ($(ENABLE_DX12),1)
	ifneq ($(TARGET_WINDOWS),1)
	  $(error The DirectX 12 backend is only supported on Windows)
	endif
	ifeq ($(ENABLE_OPENGL),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_OPENGL_LEGACY),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX11),1)
	  $(error Cannot specify multiple graphics backends)
	endif
  endif
  ifeq ($(ENABLE_SOFTRAST),1)
	ifeq ($(ENABLE_OPENGL),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_OPENGL_LEGACY),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX11),1)
	  $(error Cannot specify multiple graphics backends)
	endif
	ifeq ($(ENABLE_DX12),1)
	  $(error Cannot specify multiple graphics backends)
	endif
  endif

ifeq ($(COMPILER),gcc)
  NON_MATCHING := 1
endif

# Release

ifeq ($(VERSION),jp)
  VERSION_DEF := VERSION_JP
  GRUCODE_DEF := F3D_OLD
else
ifeq ($(VERSION),us)
  VERSION_DEF := VERSION_US
  GRUCODE_DEF := F3D_OLD
else
ifeq ($(VERSION),eu)
  VERSION_DEF := VERSION_EU
  GRUCODE_DEF := F3D_NEW
else
ifeq ($(VERSION),sh)
  $(warning Building SH is experimental and is prone to breaking. Try at your own risk.)
  VERSION_DEF := VERSION_SH
  GRUCODE_DEF := F3D_NEW
# TODO: GET RID OF THIS!!! We should mandate assets for Shindou like EU but we dont have the addresses extracted yet so we'll just pretend you have everything extracted for now.
  NOEXTRACT := 1
else
  $(error unknown version "$(VERSION)")
endif
endif
endif
endif

TARGET := sm64.$(VERSION)
VERSION_CFLAGS := -D$(VERSION_DEF)
VERSION_ASFLAGS := #-D$(VERSION_DEF)=1

# Microcode

ifeq ($(GRUCODE),f3dex) # Fast3DEX
  GRUCODE_DEF := F3DEX_GBI
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1
  TARGET := $(TARGET).f3dex
else
ifeq ($(GRUCODE), f3dex2) # Fast3DEX2
  GRUCODE_DEF := F3DEX_GBI_2
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1
  TARGET := $(TARGET).f3dex2
else
ifeq ($(GRUCODE), f3dex2e) # Fast3DEX2 Extended (for PC)
  GRUCODE_DEF := F3DEX_GBI_2E
  TARGET := $(TARGET).f3dex2e
else
ifeq ($(GRUCODE),f3d_new) # Fast3D 2.0H (Shindou)
  GRUCODE_DEF := F3D_NEW
  TARGET := $(TARGET).f3d_new
else
ifeq ($(GRUCODE),f3dzex) # Fast3DZEX (2.0J / Animal Forest - Dōbutsu no Mori)
  $(warning Fast3DZEX is experimental. Try at your own risk.)
  GRUCODE_DEF := F3DEX_GBI_2
  GRUCODE_ASFLAGS := --defsym F3DEX_GBI_SHARED=1
  TARGET := $(TARGET).f3dzex
endif
endif
endif
endif
endif

GRUCODE_CFLAGS := -D$(GRUCODE_DEF)
GRUCODE_ASFLAGS := $(GRUCODE_ASFLAGS) --defsym $(GRUCODE_DEF)=1

ifeq ($(NON_MATCHING),1)
  MATCH_CFLAGS := -DNON_MATCHING -DAVOID_UB
  MATCH_ASFLAGS := --defsym AVOID_UB=1
endif

################### Universal Dependencies ###################

# (This is a bit hacky, but a lot of rules implicitly depend
# on tools and assets, and we use directory globs further down
# in the makefile that we want should cover assets.)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)

# Make sure assets exist
NOEXTRACT ?= 0
ifeq ($(NOEXTRACT),0)
DUMMY != ./tools/rom/extract_assets.py $(VERSION) >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to extract assets)
endif
endif

# Make tools if out of date
DUMMY != $(MAKE) -s -C tools >&2 || echo FAIL
ifeq ($(DUMMY),FAIL)
  $(error Failed to build tools)
endif

endif
endif

################ Target Executable and Sources ###############

# BUILD_DIR is location where all build artifacts are placed
BUILD_DIR_BASE := build
ifeq ($(TARGET_WEB),1)
  BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_web
else
  ifeq ($(TARGET_NSP),1)
	BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_nsp$(if $(filter 1,$(BENCH)),_bench)$(if $(filter 1,$(HOST)),_host)$(if $(filter 1,$(ARMSIM)),_armsim)
  else
	BUILD_DIR := $(BUILD_DIR_BASE)/$(VERSION)_pc
  endif
endif

LIBULTRA := $(BUILD_DIR)/libultra.a
ifeq ($(TARGET_WEB),1)
EXE := $(BUILD_DIR)/$(TARGET).html
else
  ifeq ($(TARGET_WINDOWS),1)
	EXE := $(BUILD_DIR)/$(TARGET).exe
  else
	ifeq ($(TARGET_DOS),1)
	  EXE := $(BUILD_DIR)/$(TARGET).exe
	else
	  EXE := $(BUILD_DIR)/$(TARGET)
	endif
  endif
endif
ELF := $(BUILD_DIR)/$(TARGET).elf
MIO0_DIR := $(BUILD_DIR)/bin
SOUND_BIN_DIR := $(BUILD_DIR)/sound
TEXTURE_DIR := textures
ACTOR_DIR := actors
LEVEL_DIRS := $(patsubst levels/%,%,$(dir $(wildcard levels/*/header.h)))

# Directories containing source files
SRC_DIRS := src src/engine src/game src/audio src/menu src/buffers actors levels bin bin/$(VERSION) data assets
ASM_DIRS := lib
  SRC_DIRS := $(SRC_DIRS) src/pc src/pc/gfx src/pc/audio src/pc/controller
  ASM_DIRS :=
ifeq ($(TARGET_DOS),1)
  SRC_DIRS += lib/glide3
endif
ifeq ($(ARMSIM),1)
  SRC_DIRS += tools/armsim
endif
BIN_DIRS := bin bin/$(VERSION)

ULTRA_SRC_DIRS := lib/src lib/src/math
ULTRA_ASM_DIRS := lib/asm lib/data
ULTRA_BIN_DIRS := lib/bin

GODDARD_SRC_DIRS := src/goddard src/goddard/dynlists

MIPSISET := -mips2
MIPSBIT := -32

ifeq ($(COMPILER),gcc)
  MIPSISET := -mips3
endif

ifeq ($(TARGET_WEB),1)
  OPT_FLAGS := -O2 -g4 --source-map-base http://localhost:8080/
else ifeq ($(DEBUG),1)
  OPT_FLAGS := -g
else
  OPT_FLAGS := -ffast-math -O2
  #-fno-dse -fno-if-conversion -fno-if-conversion2 -fno-inline-functions-called-once -fno-ipa-modref -fno-move-loop-invariants -fno-ssa-phiopt -fno-tree-bit-ccp -fno-tree-dse -fno-tree-pta -fno-tree-sra 
endif

# File dependencies and variables for specific files
include Makefile.split

# Source code files
LEVEL_C_FILES := $(wildcard levels/*/leveldata.c) $(wildcard levels/*/script.c) $(wildcard levels/*/geo.c)
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
CXX_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
S_FILES := $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))
ULTRA_C_FILES := $(foreach dir,$(ULTRA_SRC_DIRS),$(wildcard $(dir)/*.c))
GODDARD_C_FILES := $(foreach dir,$(GODDARD_SRC_DIRS),$(wildcard $(dir)/*.c))
GENERATED_C_FILES := $(BUILD_DIR)/assets/mario_anim_data.c $(BUILD_DIR)/assets/demo_data.c \
  $(addprefix $(BUILD_DIR)/bin/,$(addsuffix _skybox.c,$(notdir $(basename $(wildcard textures/skyboxes/*.png)))))

ifeq ($(TARGET_WINDOWS),0)
  CXX_FILES :=
endif

  ULTRA_C_FILES := \
	alBnkfNew.c \
	guLookAtRef.c \
	guMtxF2L.c \
	guNormalize.c \
	guOrthoF.c \
	guPerspectiveF.c \
	guRotateF.c \
	guScaleF.c \
	guTranslateF.c

  C_FILES := $(filter-out src/game/main.c,$(C_FILES))
  ULTRA_C_FILES := $(addprefix lib/src/,$(ULTRA_C_FILES))

ifeq ($(VERSION),sh)
SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/jp/*.m64) \
	$(wildcard sound/sequences/*.m64) \
	$(foreach file,$(wildcard sound/sequences/jp/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
	$(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))
else
SOUND_BANK_FILES := $(wildcard sound/sound_banks/*.json)
SOUND_SEQUENCE_FILES := $(wildcard sound/sequences/$(VERSION)/*.m64) \
	$(wildcard sound/sequences/*.m64) \
	$(foreach file,$(wildcard sound/sequences/$(VERSION)/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
	$(foreach file,$(wildcard sound/sequences/*.s),$(BUILD_DIR)/$(file:.s=.m64))
endif

SOUND_SAMPLE_DIRS := $(wildcard sound/samples/*)
SOUND_SAMPLE_AIFFS := $(foreach dir,$(SOUND_SAMPLE_DIRS),$(wildcard $(dir)/*.aiff))
SOUND_SAMPLE_TABLES := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.table))
SOUND_SAMPLE_AIFCS := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.aifc))
SOUND_OBJ_FILES := $(SOUND_BIN_DIR)/sound_data.o


# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
		   $(foreach file,$(CXX_FILES),$(BUILD_DIR)/$(file:.cpp=.o)) \
		   $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
		   $(foreach file,$(GENERATED_C_FILES),$(file:.c=.o))

ULTRA_O_FILES := $(foreach file,$(ULTRA_S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
				 $(foreach file,$(ULTRA_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

GODDARD_O_FILES := $(foreach file,$(GODDARD_C_FILES),$(BUILD_DIR)/$(file:.c=.o))

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(ULTRA_O_FILES:.o=.d) $(GODDARD_O_FILES:.o=.d)

# Files with GLOBAL_ASM blocks
ifeq ($(NON_MATCHING),0)
GLOBAL_ASM_C_FILES != grep -rl 'GLOBAL_ASM(' $(wildcard src/**/*.c)
GLOBAL_ASM_O_FILES = $(foreach file,$(GLOBAL_ASM_C_FILES),$(BUILD_DIR)/$(file:.c=.o))
GLOBAL_ASM_DEP = $(BUILD_DIR)/src/audio/non_matching_dep
endif

# Segment elf files
SEG_FILES := $(SEGMENT_ELF_FILES) $(ACTOR_ELF_FILES) $(LEVEL_ELF_FILES)

##################### Compiler Options #######################
INCLUDE_CFLAGS := -I include -I $(BUILD_DIR) -I $(BUILD_DIR)/include -I src -I src/pc -I .
ENDIAN_BITWIDTH := $(BUILD_DIR)/endian-and-bitwidth


AS := as
ifneq ($(TARGET_WEB),1)
  CC := gcc
  CXX := g++
else
  CC := emcc
endif
ifeq ($(TARGET_WINDOWS),1)
  LD := $(CXX)
else
  LD := $(CC)
endif
CPP := cpp -P
OBJDUMP := objdump
OBJCOPY := objcopy
# OBJCOPY RUNNING INTO ERRORS
ifeq ($(TARGET_DOS),1)
  CPP := i586-pc-msdosdjgpp-cpp -P
  OBJDUMP := i586-pc-msdosdjgpp-objdump
  OBJCOPY := i586-pc-msdosdjgpp-objcopy
  AS := i586-pc-msdosdjgpp-as
  CC := i586-pc-msdosdjgpp-gcc
  CXX := i586-pc-msdosdjgpp-g++
  LD := $(CXX)
endif

ifeq ($(TARGET_NSP), 1)
  AS := arm-none-eabi-as
  CC := $(NSP_PREFIX)gcc
  LD := $(NSP_PREFIX)gcc
  CXX := $(NSP_PREFIX)g++
  OBJDUMP := arm-none-eabi-objdump
  OBJCOPY := arm-none-eabi-objcopy
  # Use GNU cpp from the toolchain; macOS's `cpp` is clang, which mangles the text/level preprocessing
  CPP := arm-none-eabi-cpp -P
  ifeq ($(HOST),1)
    CC := clang
    LD := clang
    CXX := clang++
  endif
  ifeq ($(ARMSIM),1)
    CC := arm-none-eabi-gcc
    LD := arm-none-eabi-gcc
    CXX := arm-none-eabi-g++
  endif
endif

PYTHON := python3

# Platform-specific compiler and linker flags
ifeq ($(TARGET_WINDOWS),1)
  PLATFORM_CFLAGS  := -DTARGET_WINDOWS
  PLATFORM_LDFLAGS := -lm -lxinput9_1_0 -lole32 -no-pie -mwindows
endif
ifeq ($(TARGET_LINUX),1)
  PLATFORM_CFLAGS  := -DTARGET_LINUX `pkg-config --cflags libusb-1.0`
  PLATFORM_LDFLAGS := -lm -lpthread `pkg-config --libs libusb-1.0` -lasound -lpulse -no-pie
endif
ifeq ($(TARGET_WEB),1)
  PLATFORM_CFLAGS  := -DTARGET_WEB
  PLATFORM_LDFLAGS := -lm -no-pie -s TOTAL_MEMORY=20MB -g4 --source-map-base http://localhost:8080/ -s "EXTRA_EXPORTED_RUNTIME_METHODS=['callMain']"
endif
ifeq ($(TARGET_DOS),1)
  PLATFORM_CFLAGS  := -DTARGET_DOS -std=gnu99 -nostdlib -m32 -march=i586 -mtune=pentium-mmx -mmmx -ffreestanding -fgnu89-inline
  PLATFORM_LDFLAGS := -lm -no-pie -Llib/allegro -lalleg
endif

ifeq ($(TARGET_NSP),1)
	PLATFORM_CFLAGS := -DTARGET_NSP -I include/ndless
	PLATFORM_LDFLAGS := -Llib -lnspireio 
	# `make BENCH=1` builds a self-running benchmark into build/<ver>_nsp_bench (see src/pc/benchmark.c)
	ifeq ($(BENCH),1)
		GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null)$(shell git diff --quiet HEAD 2>/dev/null || echo -dirty)
		PLATFORM_CFLAGS += -DBENCHMARK -DGIT_HASH=\"$(GIT_HASH)\"
	endif
	ifeq ($(HOST),1)
		GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null)$(shell git diff --quiet HEAD 2>/dev/null || echo -dirty)
		PLATFORM_CFLAGS := -DTARGET_NSP -DHOST_BUILD -DBENCHMARK -DGIT_HASH=\"$(GIT_HASH)\" -I tools/host_shim -g
		PLATFORM_LDFLAGS := -lm
	endif
	ifeq ($(ARMSIM),1)
		GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null)$(shell git diff --quiet HEAD 2>/dev/null || echo -dirty)
		# -mcpu and -D_TINSPIRE as nspire-gcc passes them; newlib's semihosting runtime stands in for Ndless
		PLATFORM_CFLAGS := -mcpu=arm926ej-s -D_TINSPIRE -DTARGET_NSP -DARMSIM_BUILD -DBENCHMARK -DGIT_HASH=\"$(GIT_HASH)\" -I tools/host_shim -g
		PLATFORM_LDFLAGS := -mcpu=arm926ej-s --specs=rdimon.specs -lm
	endif
endif

PLATFORM_CFLAGS += -Wfatal-errors -DNO_SEGMENTED_MEMORY 

# Compiler and linker flags for graphics backend
ifeq ($(ENABLE_OPENGL),1)
  GFX_CFLAGS  := -DENABLE_OPENGL
  GFX_LDFLAGS :=
endif
ifeq ($(ENABLE_DX11),1)
  GFX_CFLAGS := -DENABLE_DX11
  PLATFORM_LDFLAGS += -lgdi32 -static
endif
ifeq ($(ENABLE_DX12),1)
  GFX_CFLAGS := -DENABLE_DX12
  PLATFORM_LDFLAGS += -lgdi32 -static
endif
ifeq ($(ENABLE_SOFTRAST),1)
  GFX_CFLAGS := -DENABLE_SOFTRAST
  GFX_LDFLAGS :=
endif
ifeq ($(ENABLE_OPENGL_LEGACY),1)
  GFX_CFLAGS  := -DENABLE_OPENGL_LEGACY
  GFX_LDFLAGS :=
endif

ifneq ($(ENABLE_OPENGL)$(ENABLE_OPENGL_LEGACY),00)
  ifeq ($(TARGET_WINDOWS),1)
	GFX_CFLAGS  += $(shell sdl2-config --cflags) -DGLEW_STATIC
	GFX_LDFLAGS += $(shell sdl2-config --libs) -lglew32 -lopengl32 -lwinmm -limm32 -lversion -loleaut32 -lsetupapi
  endif
  ifeq ($(TARGET_LINUX),1)
	GFX_CFLAGS  += $(shell sdl2-config --cflags)
	GFX_LDFLAGS += -lGL $(shell sdl2-config --libs) -lX11 -lXrandr
  endif
  ifeq ($(TARGET_WEB),1)
	GFX_CFLAGS  += -s USE_SDL=2
	GFX_LDFLAGS += -lGL -lSDL2
  endif
  ifeq ($(TARGET_DOS),1)
	ifeq ($(DOS_GL),dmesa)
	  GFX_CFLAGS += -Iinclude/glide3 -Iinclude/dmesa -DENABLE_DMESA
	  GFX_LDFLAGS += -Llib/dmesa -lgl -Llib/glide3 -lglide3i
	else ifeq ($(DOS_GL),osmesa)
	  GFX_CFLAGS += -Iinclude/osmesa -DENABLE_OSMESA
	  GFX_LDFLAGS += -Llib/osmesa -lgl
	else
	  $(error OpenGL enabled, but no correct DOS_GL value specified (osmesa, dmesa))
	endif
  endif
else ifeq ($(ENABLE_SOFTRAST),1)
  ifeq ($(TARGET_WINDOWS),1)
	GFX_CFLAGS  += $(shell sdl2-config --cflags)
	GFX_LDFLAGS += $(shell sdl2-config --libs)
  endif
  ifeq ($(TARGET_LINUX),1)
  endif
  ifeq ($(TARGET_WEB),1)
	GFX_CFLAGS  += -s USE_SDL=2
	GFX_LDFLAGS += -lSDL2
  endif
endif

ifeq ($(TARGET_DOS),0)
  MARCH := 
endif

CC_CHECK := $(CC) -fsyntax-only -fsigned-char $(INCLUDE_CFLAGS) -Wall -Wextra -Wno-format-security -D_LANGUAGE_C $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(PLATFORM_CFLAGS) $(GFX_CFLAGS) $(GRUCODE_CFLAGS)
CFLAGS := $(OPT_FLAGS) $(INCLUDE_CFLAGS) -D_LANGUAGE_C $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(PLATFORM_CFLAGS) $(GFX_CFLAGS) $(GRUCODE_CFLAGS) -fno-strict-aliasing -fwrapv $(MARCH)

ASFLAGS := -I include -I $(BUILD_DIR) $(VERSION_ASFLAGS)

LDFLAGS := $(PLATFORM_LDFLAGS) #$(GFX_LDFLAGS)


####################### Other Tools #########################

# N64 tools
TOOLS_DIR = tools
MIO0TOOL = $(TOOLS_DIR)/mio0
N64CKSUM = $(TOOLS_DIR)/n64cksum
N64GRAPHICS = $(TOOLS_DIR)/n64graphics
N64GRAPHICS_CI = $(TOOLS_DIR)/n64graphics_ci
TEXTCONV = $(TOOLS_DIR)/textconv
AIFF_EXTRACT_CODEBOOK = $(TOOLS_DIR)/aiff_extract_codebook
VADPCM_ENC = $(TOOLS_DIR)/vadpcm_enc
EXTRACT_DATA_FOR_MIO = $(TOOLS_DIR)/extract_data_for_mio
SKYCONV = $(TOOLS_DIR)/skyconv

# Use Objcopy instead of extract_data_for_mio
ifeq ($(COMPILER),gcc)
EXTRACT_DATA_FOR_MIO := $(OBJCOPY) -O binary --only-section=.data
endif

######################## Targets #############################

ifneq ($(filter 1,$(HOST) $(ARMSIM)),)
all: $(EXE)
else
all: $(EXE).tns
endif

clean:
	$(RM) -r $(BUILD_DIR_BASE)

distclean:
	$(RM) -r $(BUILD_DIR_BASE)
	./tools/rom/extract_assets.py --clean

libultra: $(BUILD_DIR)/libultra.a

$(BUILD_DIR)/src/game/crash_screen.o: $(CRASH_TEXTURE_C_FILES)

$(BUILD_DIR)/include/text_strings.h: include/text_strings.h.in
	$(TEXTCONV) text/charmap.txt $< $@

$(BUILD_DIR)/include/text_menu_strings.h: include/text_menu_strings.h.in
	$(TEXTCONV) text/charmap_menu.txt $< $@

ifeq ($(COMPILER),gcc)
$(BUILD_DIR)/lib/src/math/%.o: CFLAGS += -fno-builtin
endif

ifeq ($(VERSION),eu)
TEXT_DIRS := text/de text/us text/fr

# EU encoded text inserted into individual segment 0x19 files,
# and course data also duplicated in leveldata.c
$(BUILD_DIR)/bin/eu/translation_en.o: $(BUILD_DIR)/text/us/define_text.inc.c
$(BUILD_DIR)/bin/eu/translation_de.o: $(BUILD_DIR)/text/de/define_text.inc.c
$(BUILD_DIR)/bin/eu/translation_fr.o: $(BUILD_DIR)/text/fr/define_text.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/us/define_courses.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/de/define_courses.inc.c
$(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/fr/define_courses.inc.c

else
ifeq ($(VERSION),sh)
TEXT_DIRS := text/jp
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/jp/define_text.inc.c

else
TEXT_DIRS := text/$(VERSION)

# non-EU encoded text inserted into segment 0x02
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/$(VERSION)/define_text.inc.c
endif
endif

$(BUILD_DIR)/text/%/define_courses.inc.c: text/define_courses.inc.c text/%/courses.h
	$(CPP) $(VERSION_CFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) text/charmap.txt - $@

$(BUILD_DIR)/text/%/define_text.inc.c: text/define_text.inc.c text/%/courses.h text/%/dialogs.h
	$(CPP) $(VERSION_CFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) text/charmap.txt - $@

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIRS) $(GODDARD_SRC_DIRS) $(ULTRA_SRC_DIRS) $(ULTRA_ASM_DIRS) $(ULTRA_BIN_DIRS) $(BIN_DIRS) $(TEXTURE_DIRS) $(TEXT_DIRS) $(SOUND_SAMPLE_DIRS) $(addprefix levels/,$(LEVEL_DIRS)) include) $(MIO0_DIR) $(addprefix $(MIO0_DIR)/,$(VERSION)) $(SOUND_BIN_DIR) $(SOUND_BIN_DIR)/sequences/$(VERSION)

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

$(BUILD_DIR)/include/text_strings.h: $(BUILD_DIR)/include/text_menu_strings.h
$(BUILD_DIR)/src/menu/file_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/menu/star_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/ingame_menu.o: $(BUILD_DIR)/include/text_strings.h

################################################################
# TEXTURE GENERATION                                           #
################################################################

# RGBA32, RGBA16, IA16, IA8, IA4, IA1, I8, I4
$(BUILD_DIR)/%: %.png
	$(N64GRAPHICS) -i $@ -g $< -f $(lastword $(subst ., ,$@))

$(BUILD_DIR)/%.inc.c: $(BUILD_DIR)/% %.png
	hexdump -v -e '1/1 "0x%X,"' $< > $@
	echo >> $@

# Color Index CI8
$(BUILD_DIR)/%.ci8: %.ci8.png
	$(N64GRAPHICS_CI) -i $@ -g $< -f ci8

# Color Index CI4
$(BUILD_DIR)/%.ci4: %.ci4.png
	$(N64GRAPHICS_CI) -i $@ -g $< -f ci4

################################################################

# compressed segment generation


$(BUILD_DIR)/%.table: %.aiff
	$(AIFF_EXTRACT_CODEBOOK) $< >$@

$(BUILD_DIR)/%.aifc: $(BUILD_DIR)/%.table %.aiff
	$(VADPCM_ENC) -c $^ $@

$(ENDIAN_BITWIDTH): tools/determine-endian-bitwidth.c
	$(CC) -c $(CFLAGS) -o $@.dummy2 $< 2>$@.dummy1; true
	grep -o 'msgbegin --endian .* --bitwidth .* msgend' $@.dummy1 > $@.dummy2
	head -n1 <$@.dummy2 | cut -d' ' -f2-5 > $@
	@rm $@.dummy1
	@rm $@.dummy2

#SOUND TARGETS REMOVED HERE

$(BUILD_DIR)/levels/scripts.o: $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/include/level_headers.h: levels/level_headers.h.in
	$(CPP) -I . levels/level_headers.h.in | $(PYTHON) tools/output_level_headers.py > $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/assets/mario_anim_data.c: $(wildcard assets/anims/*.inc.c)
	$(PYTHON) tools/mario_anims_converter.py > $@

$(BUILD_DIR)/assets/demo_data.c: assets/demo_data.json $(wildcard assets/demos/*.bin)
	$(PYTHON) tools/demo_data_converter.py assets/demo_data.json $(VERSION_CFLAGS) > $@

ifeq ($(COMPILER),ido)
# Source code
$(BUILD_DIR)/levels/%/leveldata.o: OPT_FLAGS := -g
$(BUILD_DIR)/actors/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/bin/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/src/goddard/%.o: OPT_FLAGS := -g
$(BUILD_DIR)/src/goddard/%.o: MIPSISET := -mips1
$(BUILD_DIR)/src/audio/%.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/src/audio/load.o: OPT_FLAGS := -O2 -framepointer -Wo,-loopunroll,0
$(BUILD_DIR)/lib/src/%.o: OPT_FLAGS :=
$(BUILD_DIR)/lib/src/math/ll%.o: MIPSISET := -mips3 -32
$(BUILD_DIR)/lib/src/math/%.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/math/ll%.o: OPT_FLAGS :=
$(BUILD_DIR)/lib/src/ldiv.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/string.o: OPT_FLAGS := -O2
$(BUILD_DIR)/lib/src/gu%.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/al%.o: OPT_FLAGS := -O3

ifeq ($(VERSION),eu)
$(BUILD_DIR)/lib/src/_Litob.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/_Ldtob.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/_Printf.o: OPT_FLAGS := -O3
$(BUILD_DIR)/lib/src/sprintf.o: OPT_FLAGS := -O3

# enable loop unrolling except for external.c (external.c might also have used
# unrolling, but it makes one loop harder to match)
$(BUILD_DIR)/src/audio/%.o: OPT_FLAGS := -O2
$(BUILD_DIR)/src/audio/load.o: OPT_FLAGS := -O2
$(BUILD_DIR)/src/audio/external.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
else

# The source-to-source optimizer copt is enabled for audio. This makes it use
# acpp, which needs -Wp,-+ to handle C++-style comments.
$(BUILD_DIR)/src/audio/effects.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0 -sopt,-inline=sequence_channel_process_sound,-scalaroptimize=1 -Wp,-+
$(BUILD_DIR)/src/audio/synthesis.o: OPT_FLAGS := -O2 -sopt,-scalaroptimize=1 -Wp,-+
#$(BUILD_DIR)/src/audio/seqplayer.o: OPT_FLAGS := -O2 -sopt,-inline_manual,-scalaroptimize=1 -Wp,-+ #-Wo,-v,-bb,-l,seqplayer_list.txt

# Add a target for build/eu/src/audio/*.copt to make it easier to see debug
$(BUILD_DIR)/src/audio/%.acpp: src/audio/%.c
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/acpp $(TARGET_CFLAGS) $(INCLUDE_CFLAGS) $(VERSION_CFLAGS) $(MATCH_CFLAGS) $(GRUCODE_CFLAGS) -D__sgi -+ $< > $@

$(BUILD_DIR)/src/audio/seqplayer.copt: $(BUILD_DIR)/src/audio/seqplayer.acpp
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/copt -signed -I=$< -CMP=$@ -cp=i -scalaroptimize=1 -inline_manual

$(BUILD_DIR)/src/audio/%.copt: $(BUILD_DIR)/src/audio/%.acpp
	$(QEMU_IRIX) -silent -L $(IRIX_ROOT) $(IRIX_ROOT)/usr/lib/copt -signed -I=$< -CMP=$@ -cp=i -scalaroptimize=1

endif
endif

ifeq ($(NON_MATCHING),0)
$(GLOBAL_ASM_O_FILES): CC := $(PYTHON) tools/asm_processor/build.py $(CC) -- $(AS) $(ASFLAGS) --
endif

# Rebuild files with 'GLOBAL_ASM' if the NON_MATCHING flag changes.
$(GLOBAL_ASM_O_FILES): $(GLOBAL_ASM_DEP).$(NON_MATCHING)
$(GLOBAL_ASM_DEP).$(NON_MATCHING):
	@rm -f $(GLOBAL_ASM_DEP).*
	touch $@

$(BUILD_DIR)/%.o: %.cpp
	@$(CXX) -fsyntax-only $(CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CXX) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.c
	@$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<


$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	@$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/$*.d $<
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d -o $@ $<

$(EXE).elf: $(O_FILES) $(MIO0_FILES:.mio0=.o) $(ULTRA_O_FILES) $(GODDARD_O_FILES)
	$(LD) -L $(BUILD_DIR) -o $@ $(O_FILES) $(ULTRA_O_FILES) $(GODDARD_O_FILES) $(LDFLAGS) 

ZEHNFLAGS := --name "sm64" --uses-lcd-blit 1 --240x320-support 1 --color-support 1 --32MB-support 0 --compress

ifneq ($(filter 1,$(HOST) $(ARMSIM)),)
# HOST: native executable. ARMSIM: an ELF that QEMU loads directly.
$(EXE): $(EXE).elf
	cp $< $@
endif

$(EXE).tns: $(EXE).elf
	arm-none-eabi-strip --strip-unneeded $^
	genzehn --input $^ --output $@.zehn $(ZEHNFLAGS)
	make-prg $@.zehn $@
	rm $@.zehn



.PHONY: all clean distclean default libultra
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true

ifeq ($(BENCH),1)
# Recompile benchmark.c on every bench build so its embedded GIT_HASH stays current
$(BUILD_DIR)/src/pc/benchmark.o: .FORCE
.PHONY: .FORCE
.FORCE:
endif
