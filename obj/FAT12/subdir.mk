################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FAT12/FAT12.c 

OBJS += \
./FAT12/FAT12.o 

C_DEPS += \
./FAT12/FAT12.d 


# Each subdirectory must supply rules for building sources it contributes
FAT12/%.o: ../FAT12/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized  -g -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\Debug" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\Core" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\User" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\Peripheral\inc" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\USB_Device" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\SPI_FLASH" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\SW_UDISK" -I"C:\Users\Bogdan\mrs_community_workspace3\CH32V307VCT6USB2W25QQFLASH\FAT12" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


