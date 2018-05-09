################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/MovDetector/BGFGTrack.cpp \
../src/MovDetector/Kalman.cpp \
../src/MovDetector/MovDetector.cpp \
../src/MovDetector/mvdectInterface.cpp \
../src/MovDetector/postDetector.cpp \
../src/MovDetector/psJudge.cpp 

OBJS += \
./src/MovDetector/BGFGTrack.o \
./src/MovDetector/Kalman.o \
./src/MovDetector/MovDetector.o \
./src/MovDetector/mvdectInterface.o \
./src/MovDetector/postDetector.o \
./src/MovDetector/psJudge.o 

CPP_DEPS += \
./src/MovDetector/BGFGTrack.d \
./src/MovDetector/Kalman.d \
./src/MovDetector/MovDetector.d \
./src/MovDetector/mvdectInterface.d \
./src/MovDetector/postDetector.d \
./src/MovDetector/psJudge.d 


# Each subdirectory must supply rules for building sources it contributes
src/MovDetector/%.o: ../src/MovDetector/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/local/cuda-8.0/bin/nvcc -I../include -I../src/OSA_CAP/inc -I/usr/include/opencv -I../src/MovDetector -O3 -Xcompiler -fPIC -Xcompiler -fopenmp -ccbin aarch64-linux-gnu-g++ -gencode arch=compute_53,code=sm_53 -m64 -odir "src/MovDetector" -M -o "$(@:%.o=%.d)" "$<"
	/usr/local/cuda-8.0/bin/nvcc -I../include -I../src/OSA_CAP/inc -I/usr/include/opencv -I../src/MovDetector -O3 -Xcompiler -fPIC -Xcompiler -fopenmp --compile -m64 -ccbin aarch64-linux-gnu-g++  -x c++ -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


