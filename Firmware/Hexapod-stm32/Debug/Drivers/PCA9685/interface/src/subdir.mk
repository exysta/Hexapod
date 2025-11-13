################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/PCA9685/interface/src/clock.c \
../Drivers/PCA9685/interface/src/delay.c \
../Drivers/PCA9685/interface/src/iic.c \
../Drivers/PCA9685/interface/src/uart.c \
../Drivers/PCA9685/interface/src/wire.c 

OBJS += \
./Drivers/PCA9685/interface/src/clock.o \
./Drivers/PCA9685/interface/src/delay.o \
./Drivers/PCA9685/interface/src/iic.o \
./Drivers/PCA9685/interface/src/uart.o \
./Drivers/PCA9685/interface/src/wire.o 

C_DEPS += \
./Drivers/PCA9685/interface/src/clock.d \
./Drivers/PCA9685/interface/src/delay.d \
./Drivers/PCA9685/interface/src/iic.d \
./Drivers/PCA9685/interface/src/uart.d \
./Drivers/PCA9685/interface/src/wire.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/PCA9685/interface/src/%.o Drivers/PCA9685/interface/src/%.su Drivers/PCA9685/interface/src/%.cyclo: ../Drivers/PCA9685/interface/src/%.c Drivers/PCA9685/interface/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H7A3xxQ -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"D:/Work/Projects/Hexapod-STM32/Firmware/Hexapod/Drivers/PCA9685" -I"D:/Work/Projects/Hexapod-STM32/Firmware/Hexapod/Drivers/PCA9685/interface/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-PCA9685-2f-interface-2f-src

clean-Drivers-2f-PCA9685-2f-interface-2f-src:
	-$(RM) ./Drivers/PCA9685/interface/src/clock.cyclo ./Drivers/PCA9685/interface/src/clock.d ./Drivers/PCA9685/interface/src/clock.o ./Drivers/PCA9685/interface/src/clock.su ./Drivers/PCA9685/interface/src/delay.cyclo ./Drivers/PCA9685/interface/src/delay.d ./Drivers/PCA9685/interface/src/delay.o ./Drivers/PCA9685/interface/src/delay.su ./Drivers/PCA9685/interface/src/iic.cyclo ./Drivers/PCA9685/interface/src/iic.d ./Drivers/PCA9685/interface/src/iic.o ./Drivers/PCA9685/interface/src/iic.su ./Drivers/PCA9685/interface/src/uart.cyclo ./Drivers/PCA9685/interface/src/uart.d ./Drivers/PCA9685/interface/src/uart.o ./Drivers/PCA9685/interface/src/uart.su ./Drivers/PCA9685/interface/src/wire.cyclo ./Drivers/PCA9685/interface/src/wire.d ./Drivers/PCA9685/interface/src/wire.o ./Drivers/PCA9685/interface/src/wire.su

.PHONY: clean-Drivers-2f-PCA9685-2f-interface-2f-src

