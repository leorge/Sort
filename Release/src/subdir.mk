################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/@main.c \
../src/QMI_sort.c \
../src/QM_sort.c \
../src/asymm_qsort.c \
../src/bubble_sort.c \
../src/buffer.c \
../src/dual_pivot.c \
../src/dump.c \
../src/get_pivot.c \
../src/heap_sort.c \
../src/hybrid.c \
../src/imsort.c \
../src/index.c \
../src/insert_sort.c \
../src/iqsort.c \
../src/merge_array.c \
../src/pivot_hole.c \
../src/qsort_3way.c \
../src/qsort_first.c \
../src/qsort_kr.c \
../src/qsort_med3.c \
../src/qsort_middle.c \
../src/quick_asymm.c \
../src/quick_hole.c \
../src/quick_pivot.c \
../src/quick_random.c \
../src/quick_secure.c \
../src/ticket_sort.c \
../src/timer.c 

OBJS += \
./src/@main.o \
./src/QMI_sort.o \
./src/QM_sort.o \
./src/asymm_qsort.o \
./src/bubble_sort.o \
./src/buffer.o \
./src/dual_pivot.o \
./src/dump.o \
./src/get_pivot.o \
./src/heap_sort.o \
./src/hybrid.o \
./src/imsort.o \
./src/index.o \
./src/insert_sort.o \
./src/iqsort.o \
./src/merge_array.o \
./src/pivot_hole.o \
./src/qsort_3way.o \
./src/qsort_first.o \
./src/qsort_kr.o \
./src/qsort_med3.o \
./src/qsort_middle.o \
./src/quick_asymm.o \
./src/quick_hole.o \
./src/quick_pivot.o \
./src/quick_random.o \
./src/quick_secure.o \
./src/ticket_sort.o \
./src/timer.o 

C_DEPS += \
./src/@main.d \
./src/QMI_sort.d \
./src/QM_sort.d \
./src/asymm_qsort.d \
./src/bubble_sort.d \
./src/buffer.d \
./src/dual_pivot.d \
./src/dump.d \
./src/get_pivot.d \
./src/heap_sort.d \
./src/hybrid.d \
./src/imsort.d \
./src/index.d \
./src/insert_sort.d \
./src/iqsort.d \
./src/merge_array.d \
./src/pivot_hole.d \
./src/qsort_3way.d \
./src/qsort_first.d \
./src/qsort_kr.d \
./src/qsort_med3.d \
./src/qsort_middle.d \
./src/quick_asymm.d \
./src/quick_hole.d \
./src/quick_pivot.d \
./src/quick_random.d \
./src/quick_secure.d \
./src/ticket_sort.d \
./src/timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/@main.o: ../src/@main.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c99 -D_POSIX_C_SOURCE=199309 -DRUSAGE -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/@main.d" -MT"src/@main.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c99 -D_POSIX_C_SOURCE=199309 -DRUSAGE -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


