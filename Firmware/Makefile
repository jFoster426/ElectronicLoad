# Output file name
OUTPUT_NAME = firmware

# Toolchain prefix
COMPILER_PREFIX = arm-none-eabi

# Define processor options
# Cortex-M4, thumb instruction set, hardware fpu
COMPILER_OPTIONS += -mcpu=cortex-m4
COMPILER_OPTIONS += -mthumb
COMPILER_OPTIONS += -mfpu=fpv4-sp-d16
COMPILER_OPTIONS += -mfloat-abi=hard
# Optimize for code size
COMPILER_OPTIONS += -Os
# Allow gdb to be used
COMPILER_OPTIONS += -g
# Allow eliminating of dead code and data by breaking
# each up into a separate section
# Combine this with -Wl,--gc-sections in the linker
# options to garbage collect the unused sections
# and shrink the binary size
COMPILER_OPTIONS += -ffunction-sections
COMPILER_OPTIONS += -fdata-sections
# Show all warnings
COMPILER_OPTIONS += -Wall
# Show even more warnings
COMPILER_OPTIONS += -Wextra

# Point to linker script
LINKER_OPTIONS += -T linker.ld
# Linker will garbage collect unused sections
LINKER_OPTIONS += -Wl,--gc-sections

# 
LINKER_LIBRARIES += -lc
LINKER_LIBRARIES += -lgcc
LINKER_LIBRARIES += -lnosys

# include directories
INCLUDES += -I inc

# Source files
SOURCES += src/system_stm32g4xx.c
SOURCES += src/startup_stm32g474xx.s
SOURCES += src/syscalls.c

SOURCES += src/clocks.c
SOURCES += src/delay.c
SOURCES += src/gpio.c
SOURCES += src/lcd.c
SOURCES += src/systick.c
SOURCES += src/uart.c
SOURCES += src/main.c

OBJECTS := $(patsubst src/%,build/%,$(SOURCES))
OBJECTS := $(OBJECTS:.c=.o)
OBJECTS := $(OBJECTS:.s=.o)

.PHONY : all clean upload

all : clean $(OUTPUT_NAME).bin

upload : $(OUTPUT_NAME).bin
	@openocd -d0 -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program build/$(OUTPUT_NAME).elf verify reset exit"

clean :
	@clear
	@rm -f build/*.o
	@rm -f build/*.elf
	@rm -f build/*.bin
	@rm -f build/*.asm

build/%.o : src/%.c
	@$(COMPILER_PREFIX)-gcc $(COMPILER_OPTIONS) $(INCLUDES) -c $< -o $@
    
build/%.o : src/%.s
	@$(COMPILER_PREFIX)-gcc $(COMPILER_OPTIONS) $(INCLUDES) -c $< -o $@
    
$(OUTPUT_NAME).elf : $(OBJECTS)
	@$(COMPILER_PREFIX)-gcc $(COMPILER_OPTIONS) $(LINKER_OPTIONS) $(OBJECTS) $(LINKER_LIBRARIES) -o build/$@
	@$(COMPILER_PREFIX)-objdump -D build/$@ > build/$(OUTPUT_NAME).asm
	$(COMPILER_PREFIX)-size build/$@

$(OUTPUT_NAME).bin : $(OUTPUT_NAME).elf
	@$(COMPILER_PREFIX)-objcopy -O binary build/$^ build/$@
