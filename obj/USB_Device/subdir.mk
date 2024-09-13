################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USB_Device/ch32v30x_usbfs_device.c \
../USB_Device/usb_desc.c 

OBJS += \
./USB_Device/ch32v30x_usbfs_device.o \
./USB_Device/usb_desc.o 

C_DEPS += \
./USB_Device/ch32v30x_usbfs_device.d \
./USB_Device/usb_desc.d 


# Each subdirectory must supply rules for building sources it contributes
USB_Device/%.o: ../USB_Device/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\Debug" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\Core" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\User" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\Peripheral\inc" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\USB_Device" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\SPI_FLASH" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\SW_UDISK" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\FAT12" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


