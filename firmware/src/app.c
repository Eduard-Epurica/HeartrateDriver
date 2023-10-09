/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "definitions.h"
#include "peripheral/port/plib_port.h"
#include "../HDC_Heartrate_Eduard_Epurica.X/drv_max30100.h"
#include "config/default/peripheral/sercom/usart/plib_sercom2_usart.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
void appDRVMAX30100EventHandler(const DRV_MAX30100_TRANSFER_STATUS event, uintptr_t context)
{
     APP_DATA* pApp = NULL;
    
    /* check for read completion and advance state */
    if (event == DRV_MAX30100_TRANSFER_STATUS_COMPLETED)
    {
        pApp = (APP_DATA*) context;
        if (pApp != NULL)
        {
            pApp->state = APP_STATE_WAIT_FOR_REQUEST;
        }
    }
}
/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
#define MAX30100_ADDRESS        0x57  // 8bit address converted to 7bit
#define MAX30100_MODE_CONFIG    0x06  // Control register
#define MAX30100_REV_ID         0xFF  // Part revision


const uint8_t main_menu[] = 
{
    "*** MAX30100 Heartrate Sensor Demonstration ***\r\n"
    "Connect Heartrate Mikroe Click board to either EXT2 or EXT3\r\n"
    "Press 1 to start the Calibration process\r\n\r\n"  
 
};

const uint8_t calibrating_message[] = 
{
    "\r\n*** Starting the Calibration ***"
    "\r\n*** Hold the finger over the IR sensor ***"
    "\r\nThe initial calibration process will take about 30 seconds\r\n"
};


const uint8_t input_message[] = 
{
    "\r\r\n*** Press 0 to start another measurement ***\r\n"
}; 

const uint8_t measuring_message[] = 
{
    "\r\n*** Starting another Measurement ***"
    "\r\n*** Hold the finger over the IR sensor ***\r\n"

};

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
 uint8_t dataReg = 0x68;
 uint8_t input;
void APP_Tasks ( void )
{
    
    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
            

            if (appInitialized)
            {
                    
                appData.state = APP_STATE_WAIT_FOR_MAX30100;
            }
            printf("\33[H\33[2J");
            printf("%s", main_menu);
            break;
          
        }
        case APP_STATE_WAIT_FOR_MAX30100:
          
            if (DRV_MAX30100_Status(0) == SYS_STATUS_READY)
            {
                appData.state = APP_STATE_IDLE;
            }
           appData.drvMAX30100 = DRV_MAX30100_Open(0, DRV_IO_INTENT_EXCLUSIVE);
           DRV_MAX30100_ClientEventHandlerSet(appData.drvMAX30100, appDRVMAX30100EventHandler, (uintptr_t) &appData);   
          break;
            
        case APP_STATE_IDLE:
            /* check for a key press and act accordingly */
           
            if (SERCOM2_USART_ReadIsBusy() == false)
            {
                input = getc(stdin); 
                if (input == '1')
                {
                    printf("%s", calibrating_message);
                    appData.state = APP_STATE_SERVICE_TASKS;
                    /* request a read of the weather */
                   ;
                }
            }
            break;     
            
        case APP_STATE_SERVICE_TASKS:
        {
       
            if(DRV_MAX30100_GetRead_state())
            {
                printf("%s", input_message);
                appData.state = APP_STATE_WAIT_FOR_REQUEST;
            }
         
            break;
        }
        /* TODO: implement your application state machine.*/
        case APP_STATE_WAIT_FOR_REQUEST:
        {
            if (SERCOM2_USART_ReadIsBusy() == false)
            {
                input = getc(stdin); 
                if (input == '0')
                {
                   printf("%s", measuring_message);
                   appData.state = APP_STATE_SERVICE_TASKS;
                    /* request a read of the weather */
                   //DRV_MAX30100_Close(appData.drvMAX30100);
                }
            }
            
        }
        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
