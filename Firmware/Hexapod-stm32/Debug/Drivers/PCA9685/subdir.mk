################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/PCA9685/driver_pca9685.c \
../Drivers/PCA9685/driver_pca9685_basic.c \
../Drivers/PCA9685/stm32f407_driver_pca9685_interface.c 

OBJS += \
./Drivers/PCA9685/driver_pca9685.o \
./Drivers/PCA9685/driver_pca9685_basic.o \
./Drivers/PCA9685/stm32f407_driver_pca9685_interface.o 

C_DEPS += \
./Drivers/PCA9685/driver_pca9685.d \
./Drivers/PCA9685/driver_pca9685_basic.d \
./Drivers/PCA9685/stm32f407_driver_pca9685_interface.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/PCA9685/%.o Drivers/PCA9685/%.su Drivers/PCA9685/%.cyclo: ../Drivers/PCA9685/%.c Drivers/PCA9685/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_DIRECT_SMPS_SUPPLY -DUSE_HAL_DRIVER -DSTM32H7A3xxQ -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"D:/Work/Projects/Hexapod-STM32/Firmware/Hexapod/Drivers/PCA9685" -I"D:/Work/Projects/Hexapod-STM32/Firmware/Hexapod/Drivers/PCA9685/interface/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-PCA9685

clean-Drivers-2f-PCA9685:
	-$(RM) ./Drivers/PCA9685/driver_pca9685.cyclo ./Drivers/PCA9685/driver_pca9685.d ./Drivers/PCA9685/driver_pca9685.o ./Drivers/PCA9685/driver_pca9685.su ./Drivers/PCA9685/driver_pca9685_basic.cyclo ./Drivers/PCA9685/driver_pca9685_basic.d ./Drivers/PCA9685/driver_pca9685_basic.o ./Drivers/PCA9685/driver_pca9685_basic.su ./Drivers/PCA9685/stm32f407_driver_pca9685_interface.cyclo ./Drivers/PCA9685/stm32f407_driver_pca9685_interface.d ./Drivers/PCA9685/stm32f407_driver_pca9685_interface.o ./Drivers/PCA9685/stm32f407_driver_pca9685_interface.su

.PHONY: clean-Drivers-2f-PCA9685

