.PHONY: all clean echos

TARGET = app_example
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O0

CPU = -mcpu=cortex-m4
# fpu
#FPU = -mfpu=fpv4-sp-d16
# float-abi
#FLOAT-ABI = -mfloat-abi=hard
# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DSTM32F401xC

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

#C_INCLUDES
C_INCLUDES =  \
-Isrc/include \
-Isrc/CMSIS/Include \
-Isrc/CMSIS/Device/ST/STM32F4xx/Include 

ASM_INCLUDES = \
-Isrc/include/asm_include

C_SOURCE_DIRS = \
src/CMSIS/Device/ST/STM32F4xx/Source/Templates \
src/app
ASM_SOURCE_DIRS = \
src/startup 

PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(ASM_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections 

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections 

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
ASFLAGS += -g -gdwarf-2
endif


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = $(wildcard src/ld/*.ld)

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -T $(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,--gc-sections -nostdlib

C_SOURCES := $(foreach dir,$(C_SOURCE_DIRS),$(wildcard $(dir)/*.c))
OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
ASM_SOURCES := $(foreach dir,$(ASM_SOURCE_DIRS),$(wildcard $(dir)/*.s))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(ASM_SOURCES:.s=.o))



OBJECT_DIRS = $(addprefix $(BUILD_DIR)/,$(C_SOURCE_DIRS))
OBJECT_DIRS += $(addprefix $(BUILD_DIR)/,$(ASM_SOURCE_DIRS))

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	$(SZ) $<

$(BUILD_DIR)/$(TARGET).elf: $(OBJECT_DIRS) $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
	
	
$(BUILD_DIR)/%.o: %.s 
	$(AS) -c $(ASFLAGS) $< -o $@
	
$(BUILD_DIR)/%.o: %.c 
	$(CC) -c $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf 
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf 
	$(BIN) $< $@

$(OBJECT_DIRS): 
	mkdir "$@"
	
clean:
	rd /s/q $(BUILD_DIR)
	
echos:
	echo $(OBJECTS)
	echo $(ASM_SOURCES)
	echo $(OBJECT_DIRS)