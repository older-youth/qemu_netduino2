# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET_NAME ?= QEMU_netduino2_STM32F205IG
VERSION = 1.0.0
# Linux	获取系统时间
COMPILE_TIME = $(shell date +"%y%m%d%H%M%S")


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
ifeq ($(DEBUG), 1)
OPT = -Og
TARGET := $(TARGET_NAME)_V$(VERSION)
else
OPT = -O1
# 文件名变量追加赋值不能用+= 要用:=方式 也不能用=的方式，否则变量名中含有空格
TARGET := $(TARGET_NAME)_V$(VERSION)_$(COMPILE_TIME)
endif


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES ?=  \
app/main.c \
app/stm32f2xx_it.c \
system/system_stm32f2xx.c

# ASM sources
ASM_SOURCES =  \
system/gcc_startup/startup_stm32f2xx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
DB = $(GCC_PATH)/$(PREFIX)gdb
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
DB = $(PREFIX)gdb
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m3

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
-DUSE_STDPERIPH_DRIVER \
-DSOFTWARE_VER=\"$(VERSION)\"

# AS includes
AS_INCLUDES = \
-Iapp \
-Isystem \
-Isystem/include

# C includes
C_INCLUDES ?=  \
-I. \
-Iapp \
-Isystem \
-Isystem/include

# LIB_DIR ?= ${shell pwd}/libraries
DRI_DIR ?= drivers
LIB_DIR ?= libraries
FS_DIR ?= fs
RTOS_DIR ?= rtos
# include $(DRI_DIR)/drivers.mk
include $(LIB_DIR)/lib.mk
# include $(FS_DIR)/lfs.mk
# include $(RTOS_DIR)/rtos.mk

# compile gcc flags
# -Werror 把所有警告信息转为错误
# -Wno-unused-parameter 与-Werror连用,忽略-unused-parameter警告所导致的错误,本身并不能忽略警告
# -Wno-unused-but-set-variable
# -Wno-Wsign-compare
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -Wextra -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wextra -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = system/gcc_link/stm32f2xxig_flash.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	$(shell cp $(BUILD_DIR)/*.hex .)
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# jlink load elf
#######################################
# jlink_load:all

#######################################
# gdb load elf
#######################################
gdb_load:all
	cp $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/gdbload.elf
	$(DB) -x gdbloadcommand -batch $(BUILD_DIR)/gdbload.elf

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
