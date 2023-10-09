/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _DRIVER_MAX30100_H    /* Guard against multiple inclusion */
#define _DRIVER_MAX30100_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "driver.h"
#include "../src/config/default/system/system_module.h"
#include "../src/config/default/system/system.h"
#include "../src/config/default/system/system_common.h"
#include "drv_max30100_definitions.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

        
        
    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    /* ************************************************************************** */
    /** Descriptive Constant Name

      @Summary
        Brief one-line summary of the constant.
    
      @Description
        Full description, explaining the purpose and usage of the constant.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
      @Remarks
        Any additional remarks
     */
#define EXAMPLE_CONSTANT 0


    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    // *****************************************************************************

    /** Descriptive Data Type Name

      @Summary
        Brief one-line summary of the data type.
    
      @Description
        Full description, explaining the purpose and usage of the data type.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Remarks
        Any additional remarks
        <p>
        Describe enumeration elements and structure and union members above each 
        element or member.
     */
typedef enum
{
    /* Transfer is being processed */
    DRV_MAX30100_TRANSFER_STATUS_BUSY,

    /* Transfer is successfully completed */
    DRV_MAX30100_TRANSFER_STATUS_COMPLETED,

    /* Transfer had error */
    DRV_MAX30100_TRANSFER_STATUS_ERROR

} DRV_MAX30100_TRANSFER_STATUS;

/* function prototype of application callback function */
typedef void (*DRV_MAX30100_APP_CALLBACK)(DRV_MAX30100_TRANSFER_STATUS event, uintptr_t conext);

// *****************************************************************************
// *****************************************************************************
// Section: DRV_MAX30100 Driver Module Interface Routines
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* MAX30100 Driver Transfer Event Handler Function Pointer

   Summary
    Pointer to a MAX30100 Driver Event handler function

   Description
    This data type defines the required function signature for the MAX30100 driver
    event handling callback function. A client must register a pointer
    using the event handling function whose function signature (parameter
    and return value types) match the types specified by this function pointer
    in order to receive transfer related event calls back from the driver.

    The parameters and return values are described here and a partial example
    implementation is provided.

  Parameters:
    event - Identifies the type of event
    context - Value identifying the context of the application that
    registered the event handling function.

  Returns:
    None.

  Example:
    <code>
    void APP_MyTransferEventHandler( DRV_MAX30100_TRANSFER_STATUS event, uintptr_t context )
    {
        MY_APP_DATA_STRUCT* pAppData = (MY_APP_DATA_STRUCT *) context;

        switch(event)
        {
            case DRV_MAX30100_TRANSFER_STATUS_COMPLETED:

                // Handle the transfer complete event.
                break;

            case DRV_MAX30100_TRANSFER_STATUS_ERROR:
            default:

                // Handle error.
                break;
        }
    }
    </code>

  Remarks:
    If the event is DRV_MAX30100_TRANSFER_STATUS_COMPLETED, it means that the data was
    transferred successfully and new weather data is available.

    If the event is DRV_MAX30100_TRANSFER_STATUS_ERROR, it means that the data was not
    transferred successfully.

    The context parameter contains the handle to the client context,
    provided at the time the event handling function was registered using the
    DRV_MAX30100_EventHandlerSet function.  This context handle value is
    passed back to the client as the "context" parameter.  It can be any value
    necessary to identify the client context or instance (such as a pointer to
    the client's data) instance of the client that made the request.

    The event handler function executes in the driver's interrupt
    context. It is recommended of the application to not perform process
    intensive or blocking operations with in this function.
*/

typedef void (*DRV_MAX30100_EVENT_HANDLER )( DRV_MAX30100_TRANSFER_STATUS event, uintptr_t context );

// *****************************************************************************
/* Function:
    SYS_MODULE_OBJ DRV_MAX30100_Initialize(
        const SYS_MODULE_INDEX drvIndex,
        const SYS_MODULE_INIT * const init
    )

  Summary:
    Initializes the MAX30100 device

  Description:
    This routine initializes the MAX30100 device driver making it ready for
    clients to open and use. The initialization data is specified by the init
    parameter. Initialization is a multi-step process as the MAX30100 device driver
    will interrogate the device and read the calibration parameters. This process
    must be allowed to complete before the driver will respond with 
    SYS_STATUS_READY.

  Precondition:
    None.

  Parameters:
    drvIndex - Identifier for the instance to be initialized

    init  - Pointer to the init data structure containing any data necessary to
            initialize the driver.

  Returns:
    If successful, returns a valid handle to a driver instance object.
    Otherwise, returns SYS_MODULE_OBJ_INVALID.

  Example:
    <code>
    SYS_MODULE_OBJ   sysObjDrvMAX30100;

    DRV_MAX30100_PLIB_INTERFACE gDrvMAX30100PLIBIntf[1] = {
        .read = (DRV_MAX30100_PLIB_READ) SERCOM3_I2C_Read,
        .write = (DRV_MAX30100_PLIB_WRITE) SERCOM3_I2C_Write, 
        .writeRead = (DRV_MAX30100_PLIB_WRITE_READ) SERCOM3_I2C_WriteRead,
        .errorGet = (DRV_MAX30100_PLIB_ERROR_GET) SERCOM3_I2C_ErrorGet,
        .callbackRegister = (DRV_MAX30100_PLIB_CALLBACK_REGISTER) SERCOM3_I2C_CallbackRegister,
        .transferSetup = (DRV_MAX30100_PLIB_TRANSFER_SETUP) SERCOM3_I2C_TransferSetup,
    };

    DRV_MAX30100_CLIENT_OBJ gDrvMAX30100Sensor0ClientObjPool[1];

    DRV_MAX30100_INIT drvMAX30100InitData = {
        .plibInterface = &gDrvMAX30100PLIBIntf[0],
        .configParams.sensorAddr = DRV_MAX30100_I2C_ADDRESS,
        .configParams.transferParams.clockSpeed = 400000,
        .clientObjPool = (uintptr_t) gDrvMAX30100Sensor0ClientObjPool,
        .maxClients = 1,
    };

    sysObjDrvMAX30100 = DRV_MAX30100_Initialize(DRV_MAX30100_INDEX, (SYS_MODULE_INIT *)&drvMAX30100InitData);

    </code>

  Remarks:
    This routine must be called before any other DRV_MAX30100 routine is called.
    This routine should only be called once during system initialization.

*/

SYS_MODULE_OBJ DRV_MAX30100_Initialize( const SYS_MODULE_INDEX drvIndex, const SYS_MODULE_INIT * const init);


// *************************************************************************
/* Function:
    SYS_STATUS DRV_MAX30100_Status( const SYS_MODULE_INDEX drvIndex )

  Summary:
    Gets the current status of the MAX30100 driver module.

  Description:
    This routine provides the current status of the MAX30100 driver module.

  Preconditions:
    Function DRV_MAX30100_Initialize should have been called before calling
    this function.

  Parameters:
    drvIndex   -  Identifier for the instance used to initialize driver

  Returns:
    SYS_STATUS_READY - Indicates that the driver is ready and accept
                       requests for new operations.

    SYS_STATUS_UNINITIALIZED - Indicates the driver is not initialized.

  Example:
    <code>
    SYS_STATUS status;

    status = DRV_MAX30100_Status(DRV_MAX30100_INDEX);

    if (status == SYS_STATUS_READY)
    {
        // MAX30100 driver is initialized and ready to accept requests.
    }
    </code>

  Remarks:
    None.
*/

SYS_STATUS DRV_MAX30100_Status( const SYS_MODULE_INDEX drvIndex );



// *****************************************************************************
/* Function:
    DRV_HANDLE DRV_MAX30100_Open
    (
        const SYS_MODULE_INDEX drvIndex,
        const DRV_IO_INTENT ioIntent
    )

  Summary:
    Opens the specified MAX30100 driver instance and returns a handle to it.

  Description:
    This routine opens the specified MAX30100 driver instance and provides a
    handle that must be provided to all other client-level operations to
    identify the caller and the instance of the driver. The ioIntent
    parameter defines how the client interacts with this driver instance.

  Precondition:
    Function DRV_MAX30100_Initialize must have been called before calling this
    function.

  Parameters:
    drvIndex  - Identifier for the object instance to be opened

    ioIntent -  Zero or more of the values from the enumeration DRV_IO_INTENT
                "ORed" together to indicate the intended use of the driver.
                Note: This driver ignores the ioIntent argument.

  Returns:
    If successful, the routine returns a valid open-instance handle (a number
    identifying both the caller and the module instance).

    If an error occurs, the return value is DRV_HANDLE_INVALID. Error can occur
    - if the driver instance being opened is not initialized or is invalid.

  Example:
    <code>
    DRV_HANDLE handle;

    handle = DRV_MAX30100_Open(DRV_MAX30100_INDEX, DRV_IO_INTENT_READWRITE);
    if (handle == DRV_HANDLE_INVALID)
    {
        // Unable to open the driver
        // May be the driver is not initialized
    }
    </code>

  Remarks:
    The handle returned is valid until the DRV_MAX30100_Close routine is called.
*/


DRV_HANDLE DRV_MAX30100_Open(const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent);

// *****************************************************************************
/* Function:
    void DRV_MAX30100_Close( const DRV_HANDLE handle )

  Summary:
    Closes the opened-instance of the MAX30100 driver.

  Description:
    This routine closes opened-instance of the MAX30100 driver, invalidating the
    handle. A new handle must be obtained by calling DRV_MAX30100_Open
    before the caller may use the driver again.

  Precondition:
    DRV_MAX30100_Open must have been called to obtain a valid opened device handle.

  Parameters:
    handle -    A valid open-instance handle, returned from the driver's
                open routine

  Returns:
    None.

  Example:
    <code>
    // 'handle', returned from the DRV_MAX30100_Open

    DRV_MAX30100_Close(handle);

    </code>

  Remarks:
    None.
*/
void DRV_MAX30100_Close(const DRV_HANDLE handle);


/* Get Measurements */
bool DRV_MAX30100_readSensor(const DRV_HANDLE handle);
void MAX30100_readTemp(void);
uint16_t MAX30100_getIRdata(void);
uint16_t MAX30100_getREDdata(void);
float MAX30100_getTemp(void);
bool MAX30100_getREDandIRdata(const DRV_HANDLE handle, uint16_t* REDdata, uint16_t* IRdata);
//void MAX30100_getPartID(void);

/* Setup the Sensor */
void MAX30100_setMode(uint8_t mode);
void MAX30100_setHiResEnabled(uint8_t hiResEnable);
void MAX30100_setSampleRate(uint8_t sampRate);
void MAX30100_setPulseWidth(uint8_t pWidth);
void MAX30100_setIRLEDCurrent(uint8_t irCurrent);
void MAX30100_setREDLEDCurrent(uint8_t redCurrent);
void MAX30100_updateLEDCurrent(void);
void MAX30100_updateSensorConfig(void);


/* FIFO Operations */
void MAX30100_clearCounters(const DRV_HANDLE handle);
uint8_t MAX30100_getFifoOverflow(void);
bool MAX30100_getSampleCount(const DRV_HANDLE handle, uint8_t* sampleNr);
bool MAX30100_readFifoData(const DRV_HANDLE handle, uint8_t* REDdata, uint16_t* IRdata);

void DRV_MAX30100_ClientEventHandlerSet(
    const DRV_HANDLE handle,
    const DRV_MAX30100_APP_CALLBACK callback,
    const uintptr_t context
);

void DRV_MAX30100_Tasks(
    SYS_MODULE_OBJ object
);
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#include "drv_max30100_local.h"
    
#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
