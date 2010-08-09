################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/NeuronDP.cpp \
../src/NeuronVP.cpp \
../src/NeuronVS.cpp 

CXX_SRCS += \
../src/NeuronDDS.cxx \
../src/NeuronDDSPlugin.cxx \
../src/NeuronDDSSupport.cxx 

OBJS += \
./src/NeuronDDS.o \
./src/NeuronDDSPlugin.o \
./src/NeuronDDSSupport.o \
./src/NeuronDP.o \
./src/NeuronVP.o \
./src/NeuronVS.o 

CPP_DEPS += \
./src/NeuronDP.d \
./src/NeuronVP.d \
./src/NeuronVS.d 

CXX_DEPS += \
./src/NeuronDDS.d \
./src/NeuronDDSPlugin.d \
./src/NeuronDDSSupport.d 


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


