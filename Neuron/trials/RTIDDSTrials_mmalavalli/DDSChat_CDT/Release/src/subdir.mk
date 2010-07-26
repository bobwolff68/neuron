################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DDSChatModule.cpp \
../src/main.cpp 

CXX_SRCS += \
../src/DDSChat.cxx \
../src/DDSChatPlugin.cxx \
../src/DDSChatSupport.cxx 

OBJS += \
./src/DDSChat.o \
./src/DDSChatModule.o \
./src/DDSChatPlugin.o \
./src/DDSChatSupport.o \
./src/main.o 

CPP_DEPS += \
./src/DDSChatModule.d \
./src/main.d 

CXX_DEPS += \
./src/DDSChat.d \
./src/DDSChatPlugin.d \
./src/DDSChatSupport.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cxx
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DRTI_UNIX -I/home/manjesh/RTIDDS/RTI/ndds.4.5c/include -I/home/manjesh/RTIDDS/RTI/ndds.4.5c/include/ndds -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DRTI_UNIX -I/home/manjesh/RTIDDS/RTI/ndds.4.5c/include -I/home/manjesh/RTIDDS/RTI/ndds.4.5c/include/ndds -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


