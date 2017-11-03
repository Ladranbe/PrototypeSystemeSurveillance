################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../appli/demo.c \
../appli/demo_PWM.c \
../appli/demo_accelerometer.c \
../appli/demo_adc.c \
../appli/demo_audio.c \
../appli/demo_camera.c \
../appli/demo_dac.c \
../appli/demo_lcd2x16.c \
../appli/demo_matrix_keyboard.c \
../appli/demo_motor.c \
../appli/demo_rng.c \
../appli/demo_sd.c \
../appli/demo_touchscreen.c \
../appli/demo_usb_device.c \
../appli/demo_usb_host.c \
../appli/demo_webserver.c \
../appli/demo_with_lcd.c \
../appli/main.c 

OBJS += \
./appli/demo.o \
./appli/demo_PWM.o \
./appli/demo_accelerometer.o \
./appli/demo_adc.o \
./appli/demo_audio.o \
./appli/demo_camera.o \
./appli/demo_dac.o \
./appli/demo_lcd2x16.o \
./appli/demo_matrix_keyboard.o \
./appli/demo_motor.o \
./appli/demo_rng.o \
./appli/demo_sd.o \
./appli/demo_touchscreen.o \
./appli/demo_usb_device.o \
./appli/demo_usb_host.o \
./appli/demo_webserver.o \
./appli/demo_with_lcd.o \
./appli/main.o 

C_DEPS += \
./appli/demo.d \
./appli/demo_PWM.d \
./appli/demo_accelerometer.d \
./appli/demo_adc.d \
./appli/demo_audio.d \
./appli/demo_camera.d \
./appli/demo_dac.d \
./appli/demo_lcd2x16.d \
./appli/demo_matrix_keyboard.d \
./appli/demo_motor.d \
./appli/demo_rng.d \
./appli/demo_sd.d \
./appli/demo_touchscreen.d \
./appli/demo_usb_device.d \
./appli/demo_usb_host.d \
./appli/demo_webserver.d \
./appli/demo_with_lcd.d \
./appli/main.d 


# Each subdirectory must supply rules for building sources it contributes
appli/%.o: ../appli/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo %cd%
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -DSTM32F407VGTx -DSTM32F4 -DSTM32F4DISCOVERY -DSTM32 -DDEBUG -I../appli -I../lib/hal/inc -I../lib/bsp -I../lib/bsp/Common -I../lib/bsp/lcd2x16 -I../lib/bsp/MatrixKeyboard -I../lib/bsp/lis302dl -I../lib/bsp/lis3dsh -I../lib/bsp/audio -I../lib/bsp/ov9655 -I../lib/bsp/stmpe811 -I../lib/bsp/LCD -I../lib/middleware/ -I../lib/middleware/FatFs/src -I../lib/middleware/FatFs/src/drivers -I../lib/middleware/webserver -I../lib/middleware/webserver/lwip -I../lib/middleware/webserver/lwip/src/include -I../lib/middleware/webserver/lwip/system -I../lib/middleware/webserver/lwip/src/include/ipv4 -I../lib/middleware/webserver/lwip/src/include/lwip -I../lib/middleware/webserver/lwip/port -I../lib/middleware/webserver/STD_lib -I../lib/middleware/usb_device/Class/HID/Inc -I../lib/middleware/usb_device/Class/MSC/Inc -I../lib/middleware/usb_device/Core/Inc -I../lib/middleware/usb_host/Class/MSC/Inc -I../lib/middleware/usb_host/Core/Inc -I../lib/middleware/PDM -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


