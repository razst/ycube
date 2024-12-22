################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Demos/IsisAOCSdemo.c \
../src/Demos/IsisHSTXS_V2demo.c \
../src/Demos/IsisMTQv2demo.c \
../src/Demos/IsisSPv2demo.c \
../src/Demos/IsisTRXVUrevDdemo.c \
../src/Demos/IsisTRXVUrevEdemo.c \
../src/Demos/common.c \
../src/Demos/input.c \
../src/Demos/isisRXSrevCdemo.c \
../src/Demos/isis_ants2_demo.c \
../src/Demos/isis_ants_demo.c \
../src/Demos/isismeps_ivid5_pdu_demo.c \
../src/Demos/isismeps_ivid5_piu_demo.c \
../src/Demos/trxvu_frame_ready.c 

OBJS += \
./src/Demos/IsisAOCSdemo.o \
./src/Demos/IsisHSTXS_V2demo.o \
./src/Demos/IsisMTQv2demo.o \
./src/Demos/IsisSPv2demo.o \
./src/Demos/IsisTRXVUrevDdemo.o \
./src/Demos/IsisTRXVUrevEdemo.o \
./src/Demos/common.o \
./src/Demos/input.o \
./src/Demos/isisRXSrevCdemo.o \
./src/Demos/isis_ants2_demo.o \
./src/Demos/isis_ants_demo.o \
./src/Demos/isismeps_ivid5_pdu_demo.o \
./src/Demos/isismeps_ivid5_piu_demo.o \
./src/Demos/trxvu_frame_ready.o 

C_DEPS += \
./src/Demos/IsisAOCSdemo.d \
./src/Demos/IsisHSTXS_V2demo.d \
./src/Demos/IsisMTQv2demo.d \
./src/Demos/IsisSPv2demo.d \
./src/Demos/IsisTRXVUrevDdemo.d \
./src/Demos/IsisTRXVUrevEdemo.d \
./src/Demos/common.d \
./src/Demos/input.d \
./src/Demos/isisRXSrevCdemo.d \
./src/Demos/isis_ants2_demo.d \
./src/Demos/isis_ants_demo.d \
./src/Demos/isismeps_ivid5_pdu_demo.d \
./src/Demos/isismeps_ivid5_piu_demo.d \
./src/Demos/trxvu_frame_ready.d 


# Each subdirectory must supply rules for building sources it contributes
src/Demos/%.o: ../src/Demos/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -O0 -fmessage-length=0 -ffunction-sections -Wall -Wextra  -g -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\workspace_ycube\satellite-subsystems\demo\src" -I"C:/ISIS/workspace_ycube/satellite-subsystems/demo/../..//hal/at91/include" -I"C:/ISIS/workspace_ycube/satellite-subsystems/demo/../..//hal/hal/include" -I"C:/ISIS/workspace_ycube/satellite-subsystems/demo/../..//hal/freertos/include" -I"C:/ISIS/workspace_ycube/satellite-subsystems/demo/../..//hal/hcc/include" -I"C:/ISIS/workspace_ycube/satellite-subsystems/demo/..//satellite-subsystems/include" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


