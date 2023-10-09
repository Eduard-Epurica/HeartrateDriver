/*******************************************************************************
  MAX30100 Driver Definitions Header File

  Company:
    Microchip Technology Inc.

  File Name:
    drv_MAX30100_definitions.h

  Summary:
    MAX30100 Driver Definitions Header File

  Description:
    This file provides implementation-specific definitions for the MAX30100
    driver's system interface.
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

#ifndef DRV_MAX30100_DEFINITIONS_H
#define DRV_MAX30100_DEFINITIONS_H

// *****************************************************************************
// *****************************************************************************
// Section: File includes
// *****************************************************************************
// *****************************************************************************

#include <device.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END
        
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
       
typedef enum
{
    DRV_MAX30100_ERROR_NONE = 0,
    DRV_MAX30100_ERROR_NACK,
    DRV_MAX30100_ERROR_BUS,
} DRV_MAX30100_ERROR;

typedef enum
{
    DRV_MAX30100_EVENT_READ_SENSOR_DONE = 0,
    DRV_MAX30100_EVENT_WRITE_DONE,
    DRV_MAX30100_EVENT_READ_DONE,
    DRV_MAX30100_EVENT_ERROR,
} DRV_MAX30100_EVENT;

typedef struct
{
    uint32_t clockSpeed;
} DRV_MAX30100_TRANSFER_SETUP;

typedef struct
{
    uint8_t                         sensorAddr;
    DRV_MAX30100_TRANSFER_SETUP       transferParams;
} DRV_MAX30100_CONFIG_PARAMS;


typedef union {

    struct {
        uint8_t pwr_rdy : 1;
        uint8_t : 3;
        uint8_t spo2_rdy : 1;
        uint8_t hr_rdy : 1;
        uint8_t temp_rdy : 1;
        uint8_t fifo_afull : 1;
    };
    uint8_t interruptStat;
} hr_interruptStatbits_t;


typedef union {

    struct {
        uint8_t : 4;
        uint8_t en_spo2_rdy : 1;
        uint8_t en_hr_rdy : 1;
        uint8_t en_temp_rdy : 1;
        uint8_t en_fifo_afull : 1;
    };
    uint8_t interruptEn;
} hr_interruptEnbits_t;


typedef union {

    struct {
        uint8_t mode : 3;
        uint8_t temp_en : 1;
        uint8_t : 2;
        uint8_t reset : 1;
        uint8_t shdn : 1;
    };
    uint8_t modeConfig;
} hr_modeConfigbits_t;


typedef union {

    struct {
        uint8_t led_pw : 2;
        uint8_t spo2_sr : 3;
        uint8_t : 1;
        uint8_t spo2_hires_en : 1;
        uint8_t : 1;
    };
    uint8_t spo2Config;
} hr_spo2Configbits_t;


typedef union {

    struct {
        uint8_t ir_pa : 4;
        uint8_t red_pa : 4;
    };
    uint8_t ledConfig;
} hr_ledConfigbits_t;



typedef void (* DRV_MAX30100_PLIB_CALLBACK)( uintptr_t );

typedef bool (* DRV_MAX30100_PLIB_WRITE_READ)(uint16_t , uint8_t* , uint32_t , uint8_t* , uint32_t);

typedef bool (* DRV_MAX30100_PLIB_WRITE)(uint16_t , uint8_t* , uint32_t );

typedef bool (* DRV_MAX30100_PLIB_READ)(uint16_t , uint8_t* , uint32_t);

typedef bool (* DRV_MAX30100_PLIB_IS_BUSY)(void);

typedef DRV_MAX30100_ERROR (* DRV_MAX30100_PLIB_ERROR_GET)(void);

typedef void (* DRV_MAX30100_PLIB_CALLBACK_REGISTER)(DRV_MAX30100_PLIB_CALLBACK, uintptr_t);

typedef bool (* DRV_MAX30100_PLIB_TRANSFER_SETUP)(DRV_MAX30100_TRANSFER_SETUP*, uint32_t);


// *****************************************************************************
/* MAX30100 Driver PLIB Interface Data

  Summary:
    Defines the data required to initialize the MAX30100 driver PLIB Interface.

  Description:
    This data type defines the data required to initialize the MAX30100 driver
    PLIB Interface.

  Remarks:
    None.
*/

typedef struct
{
    /* MAX30100 PLIB writeRead API */
    DRV_MAX30100_PLIB_WRITE_READ              writeRead;

    /* MAX30100 PLIB write API */
    DRV_MAX30100_PLIB_WRITE                   write;

    /* MAX30100 PLIB read API */
    DRV_MAX30100_PLIB_READ                    read;

    /* MAX30100 PLIB Error get API */
    DRV_MAX30100_PLIB_ERROR_GET               errorGet;

    /* MAX30100 PLIB callback register API */
    DRV_MAX30100_PLIB_CALLBACK_REGISTER       callbackRegister;

    /* MAX30100 PLIB Transfer setup API*/
    DRV_MAX30100_PLIB_TRANSFER_SETUP          transferSetup;    
} DRV_MAX30100_PLIB_INTERFACE;

// *****************************************************************************
/* MAX30100 Driver Initialization Data

  Summary:
    Defines the data required to initialize the MAX30100 driver

  Description:
    This data type defines the data required to initialize or the MAX30100 driver.

  Remarks:
    None.
*/

typedef struct
{
    /* Identifies the PLIB API set to be used by the driver to access the
     * peripheral. */
    const DRV_MAX30100_PLIB_INTERFACE*    plibInterface;

    /* Config parameters */
    DRV_MAX30100_CONFIG_PARAMS            configParams;
    
    /* client object pool */
    const uintptr_t                     clientObjPool;

    /* Number of clients */
    size_t                              maxClients;
} DRV_MAX30100_INIT;


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // #ifndef DRV_MAX30100_DEFINITIONS_H

/*******************************************************************************
 End of File
*/
