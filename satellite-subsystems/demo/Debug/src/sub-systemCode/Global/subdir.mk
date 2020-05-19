################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sub-systemCode/Global/Global.c \
../src/sub-systemCode/Global/GlobalParam.c \
../src/sub-systemCode/Global/TLM_management.c 

OBJS += \
./src/sub-systemCode/Global/Global.o \
./src/sub-systemCode/Global/GlobalParam.o \
./src/sub-systemCode/Global/TLM_management.o 

C_DEPS += \
./src/sub-systemCode/Global/Global.d \
./src/sub-systemCode/Global/GlobalParam.d \
./src/sub-systemCode/Global/TLM_management.d 


# Each subdirectory must supply rules for building sources it contributes
src/sub-systemCode/Global/%.o: ../src/sub-systemCode/Global/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -O0 -fmessage-length=0 -ffunction-sections -Wall -Wextra  -g -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\workspace\ycube\satellite-subsystems\demo\src" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/at91/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/hal/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/freertos/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/../..//hal/hcc/include" -I"C:/ISIS/workspace/ycube/satellite-subsystems/demo/..//satellite-subsystems/include" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


