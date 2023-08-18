
/*
 * Auto generated Run-Time-Environment Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'project' 
 * Target:  'Flash' 
 */

#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H


/*
 * Define the Device Header File: 
 */
#define CMSIS_device_header "stm32h7xx.h"

/* ARM::CMSIS:RTOS2:Keil RTX5:Source:5.5.4 */
#define RTE_CMSIS_RTOS2                 /* CMSIS-RTOS2 */
        #define RTE_CMSIS_RTOS2_RTX5            /* CMSIS-RTOS2 Keil RTX5 */
        #define RTE_CMSIS_RTOS2_RTX5_SOURCE     /* CMSIS-RTOS2 Keil RTX5 Source */
/* Keil.ARM Compiler::Compiler:Event Recorder:DAP:1.5.1 */
#define RTE_Compiler_EventRecorder
          #define RTE_Compiler_EventRecorder_DAP
/* Keil.ARM Compiler::Compiler:I/O:File:File System:1.2.0 */
#define RTE_Compiler_IO_File            /* Compiler I/O: File */
          #define RTE_Compiler_IO_File_FS         /* Compiler I/O: File (File System) */
/* Keil.ARM Compiler::Compiler:I/O:STDIN:User:1.2.0 */
#define RTE_Compiler_IO_STDIN           /* Compiler I/O: STDIN */
          #define RTE_Compiler_IO_STDIN_User      /* Compiler I/O: STDIN User */
/* Keil.ARM Compiler::Compiler:I/O:STDOUT:User:1.2.0 */
#define RTE_Compiler_IO_STDOUT          /* Compiler I/O: STDOUT */
          #define RTE_Compiler_IO_STDOUT_User     /* Compiler I/O: STDOUT User */
/* Keil.MDK-Pro::File System:CORE:LFN Debug:6.15.0 */
#define RTE_FileSystem_Core             /* File System Core */
          #define RTE_FileSystem_LFN              /* File System with Long Filename support */
          #define RTE_FileSystem_Debug            /* File System Debug Version */
/* Keil.MDK-Pro::File System:Drive:NAND:6.15.0 */
#define RTE_FileSystem_Drive_NAND_0     /* File System NAND Flash Drive 0 */

/* Keil.MDK-Pro::USB:CORE:Debug:6.16.0 */
#define RTE_USB_Core                    /* USB Core */
          #define RTE_USB_Core_Debug              /* USB Core Debug Version */
/* Keil.MDK-Pro::USB:Device:6.16.0 */
#define RTE_USB_Device_0                /* USB Device 0 */

/* Keil.MDK-Pro::USB:Device:MSC:6.16.0 */
#define RTE_USB_Device_MSC_0            /* USB Device MSC instance 0 */

/* Keil::CMSIS Driver:NAND:Memory Bus:1.1.0 */
#define RTE_Driver_NAND_MemoryBus       /* Driver NAND Flash on Memory Bus */
/* Keil::Device:STM32Cube Framework:STM32CubeMX:2.0.0 */
#define RTE_DEVICE_FRAMEWORK_CUBE_MX


#endif /* RTE_COMPONENTS_H */
