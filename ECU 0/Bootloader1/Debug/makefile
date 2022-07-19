################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

BUILD_ARTIFACT_NAME := Bootloader_1
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX := 
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME).$(BUILD_ARTIFACT_EXTENSION)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Bootloader_1.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Bootloader_1.list \

OBJCOPY_HEX += \
Bootloader_1.hex \

OBJCOPY_BIN += \
Bootloader_1.bin \

OBJCOPY_VERILOG += \
Bootloader_1.verilog \

OBJCOPY_SREC += \
Bootloader_1.srec \

OBJCOPY_SYMBOLSREC += \
Bootloader_1.symbolsrec \


# All Target
all: main-build

# Main-build Target
main-build: Bootloader_1.elf secondary-outputs

# Tool invocations
Bootloader_1.elf: $(OBJS) $(USER_OBJS) E:\Emmbedded\projects\STM32F103\stm32Cube\ IDE\graduation\ project\ECU\ 0\Bootloader1\STM32F103C8TX_FLASH.ld
	arm-none-eabi-gcc -o "Bootloader_1.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"E:\Emmbedded\projects\STM32F103\stm32Cube IDE\graduation project\ECU 0\Bootloader1\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Bootloader_1.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Bootloader_1.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Bootloader_1.list"
	@echo 'Finished building: $@'
	@echo ' '

Bootloader_1.hex: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "Bootloader_1.hex"
	@echo 'Finished building: $@'
	@echo ' '

Bootloader_1.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Bootloader_1.bin"
	@echo 'Finished building: $@'
	@echo ' '

Bootloader_1.verilog: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O verilog $(EXECUTABLES) "Bootloader_1.verilog"
	@echo 'Finished building: $@'
	@echo ' '

Bootloader_1.srec: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O srec $(EXECUTABLES) "Bootloader_1.srec"
	@echo 'Finished building: $@'
	@echo ' '

Bootloader_1.symbolsrec: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O symbolsrec $(EXECUTABLES) "Bootloader_1.symbolsrec"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN) $(OBJCOPY_VERILOG) $(OBJCOPY_SREC) $(OBJCOPY_SYMBOLSREC)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents fail-specified-linker-script-missing warn-no-linker-script-specified
.SECONDARY:

-include ../makefile.targets
