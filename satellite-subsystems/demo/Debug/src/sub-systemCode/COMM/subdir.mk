################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sub-systemCode/COMM/APRS.c \
../src/sub-systemCode/COMM/DelayedCommand_list.c \
../src/sub-systemCode/COMM/GSC.c \
../src/sub-systemCode/COMM/TRXVU.c 

OBJS += \
./src/sub-systemCode/COMM/APRS.o \
./src/sub-systemCode/COMM/DelayedCommand_list.o \
./src/sub-systemCode/COMM/GSC.o \
./src/sub-systemCode/COMM/TRXVU.o 

C_DEPS += \
./src/sub-systemCode/COMM/APRS.d \
./src/sub-systemCode/COMM/DelayedCommand_list.d \
./src/sub-systemCode/COMM/GSC.d \
./src/sub-systemCode/COMM/TRXVU.d 


# Each subdirectory must supply rules for building sources it contributes
src/sub-systemCode/COMM/%.o: ../src/sub-systemCode/COMM/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -O0 -fmessage-length=0 -ffunction-sections -Wall -Wextra  -g -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\workspace\ycube\satellite-subsystems\demo\src" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/at91/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/hal/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/freertos/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/hcc/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/..//satellite-subsystems/include" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


