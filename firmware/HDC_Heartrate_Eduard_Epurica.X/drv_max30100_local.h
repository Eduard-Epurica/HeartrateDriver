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

#ifndef _DRV_MAX30100_LOCAL_H   /* Guard against multiple inclusion */
#define _DRV_MAX30100_LOCAL_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */

#include "configuration.h"


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

// Interrupt Flags
#define MAX30100_POWER_RDY       0x01
#define MAX30100_SPO2_RDY        0x10
#define MAX30100_HR_RDY          0x20
#define MAX30100_TEMP_RDY        0x40
#define MAX30100_FIFO_FULL       0x80

// Status
#define READY           0x01
#define N_READY         0x00
#define ENABLED         0x01
#define DISABLED        0x00
#define RESERVED        0x00

// Mode Select
#define MAX30100_HR_ONLY         0x02
#define MAX30100_SPO2_EN         0x03

// SPO2 Sample Rate (SPS)
#define MAX30100_SR50            0x00
#define MAX30100_SR100           0x01
#define MAX30100_SR167           0x02
#define MAX30100_SR200           0x03
#define MAX30100_SR400           0x04
#define MAX30100_SR600           0x05
#define MAX30100_SR800           0x06
#define MAX30100_SR1000          0x07

// LED Pulse Width
#define MAX30100_PW200           0x00
#define MAX30100_PW400           0x01
#define MAX30100_PW800           0x02
#define MAX30100_PW1600          0x03

// LED Current (mA)
#define MAX30100_I0              0x00 
#define MAX30100_I4              0x01
#define MAX30100_I7              0x02
#define MAX30100_I11             0x03
#define MAX30100_I14             0x04 
#define MAX30100_I17             0x05
#define MAX30100_I21             0x06 
#define MAX30100_I24             0x07
#define MAX30100_I27             0x08 
#define MAX30100_I31             0x09 
#define MAX30100_I34             0x0A 
#define MAX30100_I37             0x0B
#define MAX30100_I40             0x0C 
#define MAX30100_I44             0x0D 
#define MAX30100_I47             0x0E 
#define MAX30100_I50             0x0F

// MAX30100 Slave Address
#define MAX30100_ADDR   0x57

// Registers
#define MAX30100_INTERRUPT_STAT_REG     0x00
#define MAX30100_INTERRUPT_EN_REG       0x01
#define MAX30100_FIFO_WR_PTR_REG        0x02
#define MAX30100_OVF_CTR_REG            0x03
#define MAX30100_FIFO_RD_PTR_REG        0x04
#define MAX30100_FIFO_DATA_REG          0x05
#define MAX30100_MODE_CONFIG_REG        0x06
#define MAX30100_SPO2_CONFIG_REG        0x07
#define MAX30100_LED_CONFIG_REG         0x09
#define MAX30100_TEMP_INT_REG           0x16
#define MAX30100_TEMP_FRAC_REG          0x17
#define MAX30100_REV_ID_REG             0xFE
#define MAX30100_PART_ID_REG            0xFF
#define MAX30100_RESET_VALUE            0x40

#define DC_REMOVER_ALPHA                0.9
#define BEATDETECTOR_INIT_HOLDOFF                   2000    // in ms, how long to wait before counting
#define BEATDETECTOR_MASKING_HOLDOFF                200     // in ms, non-retriggerable window after beat detection
#define BEATDETECTOR_BPFILTER_ALPHA                 0.8     // EMA factor for the beat period value
#define BEATDETECTOR_MIN_THRESHOLD                  20      // minimum threshold (filtered) value
#define BEATDETECTOR_MAX_THRESHOLD                  800    // maximum threshold (filtered) value
#define BEATDETECTOR_STEP_RESILIENCY                30      // maximum negative jump that triggers the beat edge
#define BEATDETECTOR_THRESHOLD_FALLOFF_TARGET       0.3     // thr chasing factor of the max value when beat
#define BEATDETECTOR_THRESHOLD_DECAY_FACTOR         0.99    // thr chasing factor when no beat
#define BEATDETECTOR_INVALID_READOUT_DELAY          2000    // in ms, no-beat time to cause a reset
#define BEATDETECTOR_SAMPLES_PERIOD                 10      // in ms, 1/Fs
/* Macro to combine two 8 bit data's to form a 16 bit data */
#define DRV_MAX30100_CONCAT_BYTES(msb, lsb) (((uint16_t)msb << 8) | (uint16_t)lsb)


/* Write buffer size, for the MAX30100 only address+2 bytes may be written
 * however we create a buffer large enough for the calibration data to
 * be read at initialization
 */

#define COUNTER_RESET_VALUE                         20 
#define COUNTER_SAME_VALUE                          250
#define DRV_MAX30100_READ_BUFFER_SIZE     32
#define DRV_MAX30100_WRITE_BUFFER_SIZE    5
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

/* Device states */
typedef enum
{  
    DRV_MAX30100_TASK_STATE_INIT = 0,
    DRV_MAX30100_TASK_STATE_READ_ID,
    DRV_MAX30100_TASK_STATE_SENSOR_CONFIG,
    DRV_MAX30100_TASK_STATE_CLEAR_COUNTERS, 
    DRX_MAX30100_TASK_STATE_SET_LED,        
    DRV_MAX30100_TASK_STATE_READ_SAMPLES,
    DRV_MAX30100_TASK_STATE_READ_IR_RED,
    DRV_MAX30100_TASK_STATE_IDLE,
    DRV_MAX30100_TASK_STATE_READ_IR_RED_COMPLETE, 
    DRV_MAX30100_TASK_STATE_ERROR
             
} DRV_MAX30100_TASK_STATES;


typedef enum {
    BEATDETECTOR_STATE_INIT,
    BEATDETECTOR_STATE_WAITING,
    BEATDETECTOR_STATE_FOLLOWING_SLOPE,
    BEATDETECTOR_STATE_MAYBE_DETECTED,
    BEATDETECTOR_STATE_MASKING
} BeatMonitorState_t;

// *****************************************************************************
/* MAX30100 Client Instance Object

  Summary:
    Object used to keep any data required by a client of the MAX30100 driver.

  Description:
    None.

  Remarks:
    None.
*/
typedef struct
{
    bool                        inUse;
    DRV_MAX30100_APP_CALLBACK     callback;
    uintptr_t                   context;
    uint8_t                     drvIndex;
    DRV_MAX30100_CONFIG_PARAMS    configParams;
   
} DRV_MAX30100_CLIENT_OBJ;




typedef struct
{
    uint8_t             sampleNr;
    uint16_t            IRdata;
    uint16_t            REDdata;
    
} DRV_MAX30100_DATA;

// *****************************************************************************
/* MAX30100 Driver Instance Object

  Summary:
    Object used to keep any data required for an instance of the MAX30100 driver.

  Description:
    None.

  Remarks:
    None.
*/

typedef struct
{
    /* The status of the driver */
    SYS_STATUS                          status;
    
    /* Flag to indicate this object is in use  */
    bool                                inUse;

    /* Indicates the number of clients that have opened this driver */
    size_t                              nClients;

    /* Maximum number of clients */
    size_t                              nClientsMax;

    /* PLIB API list that will be used by the driver to access the hardware */
    const DRV_MAX30100_PLIB_INTERFACE*    plibInterface;
    
    /* the pool of clients and the current active client */
    DRV_MAX30100_CLIENT_OBJ*              clientObjPool;
    DRV_MAX30100_CLIENT_OBJ*              activeClient;
    
    /* read buffer, primarily used for sample number or
     * IR and RED led readings 
    */
    volatile uint8_t                    readBuffer[DRV_MAX30100_READ_BUFFER_SIZE];
    /* write buffer to start readout requests */
    uint8_t                             writeBuffer[DRV_MAX30100_WRITE_BUFFER_SIZE];

    /* config parameters for address and clock speed */
    DRV_MAX30100_CONFIG_PARAMS   configParams;
    
    /* Event for the event handler */
    DRV_MAX30100_EVENT                    event;  
    
    /* Current state of the driver */
    volatile DRV_MAX30100_TASK_STATES     taskState;
    
    /* next state of the driver for asynchronous transactions */
    DRV_MAX30100_TASK_STATES              nextTaskState;

    /* Device ID (should be 0x11 or decimal l7) */
    uint8_t                             deviceID;
    
    /* Data where the IR and RED led latest values are stored, as well as the sample number*/
    DRV_MAX30100_DATA                   data;
    
    bool check_read;
    
} DRV_MAX30100_OBJ;


#endif //#ifndef _DRV_MAX30100C_LOCAL_H



/* *****************************************************************************
 End of File
 */
