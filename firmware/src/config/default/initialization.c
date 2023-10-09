/*******************************************************************************
  System Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "configuration.h"
#include "definitions.h"
#include "device.h"
#include "../HDC_Heartrate_Eduard_Epurica.X/drv_max30100.h"
#include "../HDC_Heartrate_Eduard_Epurica.X/drv_max30100_definitions.h"


// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************
#pragma config BOD33_DIS = SET
#pragma config BOD33USERLEVEL = 0x1cU
#pragma config BOD33_ACTION = RESET
#pragma config BOD33_HYST = 0x2U
#pragma config NVMCTRL_BOOTPROT = 0
#pragma config NVMCTRL_SEESBLK = 0x0U
#pragma config NVMCTRL_SEEPSZ = 0x0U
#pragma config RAMECC_ECCDIS = SET
#pragma config WDT_ENABLE = CLEAR
#pragma config WDT_ALWAYSON = CLEAR
#pragma config WDT_PER = CYC8192
#pragma config WDT_WINDOW = CYC8192
#pragma config WDT_EWOFFSET = CYC8192
#pragma config WDT_WEN = CLEAR
#pragma config NVMCTRL_REGION_LOCKS = 0xffffffffU




// *****************************************************************************
// *****************************************************************************
// Section: Driver Initialization Data
// *****************************************************************************
// *****************************************************************************
const DRV_MAX30100_PLIB_INTERFACE gDrvMAX30100PLIBIntf[1] =
{
    {
        .read = (DRV_MAX30100_PLIB_READ) SERCOM7_I2C_Read,
        .write = (DRV_MAX30100_PLIB_WRITE) SERCOM7_I2C_Write, 
        .writeRead = (DRV_MAX30100_PLIB_WRITE_READ) SERCOM7_I2C_WriteRead,
        .errorGet = (DRV_MAX30100_PLIB_ERROR_GET) SERCOM7_I2C_ErrorGet,
        .callbackRegister = (DRV_MAX30100_PLIB_CALLBACK_REGISTER) SERCOM7_I2C_CallbackRegister,
        .transferSetup = (DRV_MAX30100_PLIB_TRANSFER_SETUP) SERCOM7_I2C_TransferSetup,
    }
};

DRV_MAX30100_CLIENT_OBJ gDrvMAX30100Sensor0ClientObjPool[1];

const DRV_MAX30100_INIT gDrvMAX30100InitObj[1] =
{
    {
        .plibInterface = &gDrvMAX30100PLIBIntf[0],
        .configParams.sensorAddr = MAX30100_ADDR,
        .configParams.transferParams.clockSpeed = 100000,
        .clientObjPool = (uintptr_t) gDrvMAX30100Sensor0ClientObjPool,
        .maxClients = 1,
    }
};

// *****************************************************************************
// *****************************************************************************
// Section: System Data
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Library/Stack Initialization Data
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: System Initialization
// *****************************************************************************
// *****************************************************************************
SYSTEM_OBJECTS sysObj;


// *****************************************************************************
// *****************************************************************************
// Section: Local initialization functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void STDIO_BufferModeSet ( void )

  Summary:
    Sets the buffering mode for stdin and stdout

  Remarks:
 ********************************************************************************/
static void STDIO_BufferModeSet(void)
{
    /* MISRAC 2012 deviation block start */
    /* MISRA C-2012 Rule 21.6 deviated 2 times in this file.  Deviation record ID -  H3_MISRAC_2012_R_21_6_DR_3 */

    /* Make stdin unbuffered */
    setbuf(stdin, NULL);

    /* Make stdout unbuffered */
    setbuf(stdout, NULL);
}




/*******************************************************************************
  Function:
    void SYS_Initialize ( void *data )

  Summary:
    Initializes the board, services, drivers, application and other modules.

  Remarks:
 */

void SYS_Initialize ( void* data )
{
    /* MISRAC 2012 deviation block start */
    /* MISRA C-2012 Rule 2.2 deviated in this file.  Deviation record ID -  H3_MISRAC_2012_R_2_2_DR_1 */

    NVMCTRL_Initialize( );

    STDIO_BufferModeSet();


  
    PORT_Initialize();

    CLOCK_Initialize();




    SERCOM2_USART_Initialize();

    EVSYS_Initialize();

    SERCOM7_I2C_Initialize();

    sysObj.drvMAX30100 = DRV_MAX30100_Initialize(DRV_MAX30100_INSTANCE_0, (SYS_MODULE_INIT*) &gDrvMAX30100InitObj[0]);



    APP_Initialize();


    NVIC_Initialize();

    /* MISRAC 2012 deviation block end */
}


/*******************************************************************************
 End of File
*/
