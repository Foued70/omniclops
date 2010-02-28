################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../anyoption.cpp \
../bitmap.cpp \
../detectfloor.cpp \
../drawing.cpp \
../fast.cpp \
../graphslam.cpp \
../harris.cpp \
../libcam.cpp \
../main.cpp \
../map2D.cpp \
../motionmodel.cpp \
../omni.cpp \
../pointcloud.cpp \
../polynomial.cpp \
../posegraph.cpp \
../trackfeatures.cpp 

OBJS += \
./anyoption.o \
./bitmap.o \
./detectfloor.o \
./drawing.o \
./fast.o \
./graphslam.o \
./harris.o \
./libcam.o \
./main.o \
./map2D.o \
./motionmodel.o \
./omni.o \
./pointcloud.o \
./polynomial.o \
./posegraph.o \
./trackfeatures.o 

CPP_DEPS += \
./anyoption.d \
./bitmap.d \
./detectfloor.d \
./drawing.d \
./fast.d \
./graphslam.d \
./harris.d \
./libcam.d \
./main.d \
./map2D.d \
./motionmodel.d \
./omni.d \
./pointcloud.d \
./polynomial.d \
./posegraph.d \
./trackfeatures.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv -I/usr/include/gstreamer-0.10 -O3 -g3 -Wall -c -fmessage-length=0 -lcam -lcv -lcxcore -lcvaux -lhighgui `pkg-config --cflags --libs gstreamer-0.10` -L/usr/local/lib -lcv -lcxcore -lcvaux -lhighgui `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs gstreamer-plugins-base-0.10` -lgstapp-0.10 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


