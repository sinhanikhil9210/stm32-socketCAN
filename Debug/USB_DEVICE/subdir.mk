################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_DEVICE/usb_can_bridge.c \
../USB_DEVICE/usbd_gsusb.c \
../USB_DEVICE/usbd_gsusb_desc.c 

OBJS += \
./USB_DEVICE/usb_can_bridge.o \
./USB_DEVICE/usbd_gsusb.o \
./USB_DEVICE/usbd_gsusb_desc.o 

C_DEPS += \
./USB_DEVICE/usb_can_bridge.d \
./USB_DEVICE/usbd_gsusb.d \
./USB_DEVICE/usbd_gsusb_desc.d 


# Each subdirectory must supply rules for building sources it contributes
USB_DEVICE/%.o USB_DEVICE/%.su USB_DEVICE/%.cyclo: ../USB_DEVICE/%.c USB_DEVICE/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -IC:/Users/nikhi/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc -IC:/Users/nikhi/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -IC:/Users/nikhi/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Device/ST/STM32F4xx/Include -IC:/Users/nikhi/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -IC:/Users/nikhi/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I"C:/Users/nikhi/STM32CubeIDE/workspace_1.18.0/socketCAN5/USB_DEVICE" -IC:/Users/nikhi/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.1/Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USB_DEVICE

clean-USB_DEVICE:
	-$(RM) ./USB_DEVICE/usb_can_bridge.cyclo ./USB_DEVICE/usb_can_bridge.d ./USB_DEVICE/usb_can_bridge.o ./USB_DEVICE/usb_can_bridge.su ./USB_DEVICE/usbd_gsusb.cyclo ./USB_DEVICE/usbd_gsusb.d ./USB_DEVICE/usbd_gsusb.o ./USB_DEVICE/usbd_gsusb.su ./USB_DEVICE/usbd_gsusb_desc.cyclo ./USB_DEVICE/usbd_gsusb_desc.d ./USB_DEVICE/usbd_gsusb_desc.o ./USB_DEVICE/usbd_gsusb_desc.su

.PHONY: clean-USB_DEVICE

