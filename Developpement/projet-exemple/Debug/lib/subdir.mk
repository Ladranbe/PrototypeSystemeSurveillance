################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../lib/startup_stm32.s 

OBJS += \
./lib/startup_stm32.o 


# Each subdirectory must supply rules for building sources it contributes
lib/%.o: ../lib/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo %cd%
	arm-none-eabi-as -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -I"C:/Users/benoi_000/Documents/Mini_Projet_Microprocesseurs_I2/Developpement/projet-exemple/inc" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


