/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */
#include "configuration.h"
#include "drv_max30100.h"
#include "definitions.h"
#include <math.h>
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/* ************************************************************************** */
/** Descriptive Data Item Name

  @Summary
    Brief one-line summary of the data item.
    
  @Description
    Full description, explaining the purpose and usage of data item.
    <p>
    Additional description in consecutive paragraphs separated by HTML 
    paragraph breaks, as necessary.
    <p>
    Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
  @Remarks
    Any additional remarks
 */
int global_data;

hr_interruptStatbits_t interrupt_stat_bits;
hr_interruptEnbits_t interrupt_en_bits;
hr_modeConfigbits_t mode_config_bits;
hr_spo2Configbits_t spo2_config_bits;
hr_ledConfigbits_t led_config_bits;


static uint8_t fifo_wr_ptr;
static uint8_t fifo_rd_ptr;
static uint8_t ovf_ctr;


float bpmRate;
float bpmRate_old;
uint16_t irData;
uint8_t counter = 20;
uint8_t itemCounter = 0;
uint8_t counter_same_value = 250;
uint32_t timer_ms;
uint16_t sampRate = 200;

uint32_t tsLastBeat;
uint32_t tsLastSample;
uint32_t tsLastReport;

float threshold;
float beatPeriod;
float lastMaxValue;
uint32_t timer_ms;
float heart_rate;
float sum = 0;
float average = 0;
float last_value_considered = 40;
uint8_t stop = 0;
bool beatDetected;

float items[20];
int front = -1, rear = -1;

BeatMonitorState_t state;

static DRV_MAX30100_OBJ gDrvMAX30100Obj[DRV_MAX30100_INSTANCES_NUMBER];

typedef struct {
    float v_ctr[2];
    float dcW;
} filter_t;

filter_t irFilters;
// *****************************************************************************
// *****************************************************************************
// Section: DRV_MAX30100 Driver Local Functions
// *****************************************************************************
// *****************************************************************************
void DRV_MAX30100_enQueue(float value)
{
    items[19] = value;
}

void DRV_MAX30100_deQueue()
{
    for(int i = 1;i< 20;i++)
    {
        items[i-1] = items[i]; 
    }
    
}

void DRV_MAX30100_Add_Samples()
{
    sum = 0;
    for(int i = 0;i < 20;i++)
        sum = sum + items[i];
}

void DRV_MAX30100_TMR_CallBack_Counter(void)
{
    if (timer_ms < 0x7FFFFFFF) 
	{
        timer_ms++;
    } 
	else 
	{
        timer_ms = 0;
    }
}

bool DRV_MAX30100_GetRead_state()
{
    bool dummyReadCeck;
    
    dummyReadCeck = gDrvMAX30100Obj->check_read;
    gDrvMAX30100Obj->check_read = false; 
    
    return dummyReadCeck;
    
}

/* Filtering functions for processing raw data*/
float DRV_MAX30100_dcRemoval(float x) 
{
    float olddcW = irFilters.dcW;
    irFilters.dcW = (float) x + (DC_REMOVER_ALPHA * irFilters.dcW);

    return (irFilters.dcW - olddcW);
}

float DRV_MAX30100_filterBuLp1(uint16_t x) 
{
   
    irFilters.v_ctr[0] = irFilters.v_ctr[1];

    
    // FS = 100Hz, Fc = 10Hz, 1st order
    irFilters.v_ctr[1] = (2.452372752527856026e-1 * x) + (0.50952544949442879485 * irFilters.v_ctr[0]);
    return (irFilters.v_ctr[0] + irFilters.v_ctr[1]);
}

void DRV_MAX30100_decreaseThreshold(void) 
{
    // When a valid beat rate readout is present
    if (lastMaxValue > 0 && beatPeriod > 0) 
	{
        threshold -= lastMaxValue * (1 - BEATDETECTOR_THRESHOLD_FALLOFF_TARGET) /
                (beatPeriod / BEATDETECTOR_SAMPLES_PERIOD);
    } 
	else 
	{
        // Asymptotic decay
        threshold *= BEATDETECTOR_THRESHOLD_DECAY_FACTOR;
    }

    if (threshold < BEATDETECTOR_MIN_THRESHOLD) 
	{
        threshold = BEATDETECTOR_MIN_THRESHOLD;
    }
}

uint32_t DRV_MAX30100_checkTimeDiff(uint32_t time) 
{
    if (time < timer_ms) 
	{
        return (timer_ms - time);
    } 
	else 
	{
        return ((0x7FFFFFFF - time) + timer_ms);
    }
}

float DRV_MAX30100_getBPMRate(void) 
{
    if (beatPeriod != 0) 
	{
        return ((1 / beatPeriod) * 1000 * 60);
    } 
	else 
	{
        return 0;
    }
}

bool DRV_MAX30100_checkForBeat(float sample) 
{
    static bool beatDetected = false;

    switch (state) 
	{
        case BEATDETECTOR_STATE_INIT:
            if (timer_ms > BEATDETECTOR_INIT_HOLDOFF) 
			{
                state = BEATDETECTOR_STATE_WAITING;
            }
            break;

        case BEATDETECTOR_STATE_WAITING:
                
            if (sample > threshold) 
			{
                threshold = sample < BEATDETECTOR_MAX_THRESHOLD ? sample : BEATDETECTOR_MAX_THRESHOLD;
                state = BEATDETECTOR_STATE_FOLLOWING_SLOPE;
            }

            // Tracking lost, resetting
            if (DRV_MAX30100_checkTimeDiff(tsLastBeat) > BEATDETECTOR_INVALID_READOUT_DELAY) 
			{
                beatPeriod = 0;
                lastMaxValue = 0;
            }

            DRV_MAX30100_decreaseThreshold();
            break;

        case BEATDETECTOR_STATE_FOLLOWING_SLOPE:
            if (sample < threshold) 
			{
                state = BEATDETECTOR_STATE_MAYBE_DETECTED;
            } 
			else 
			{
                threshold = sample < BEATDETECTOR_MAX_THRESHOLD ? sample : BEATDETECTOR_MAX_THRESHOLD;
            }
            break;

        case BEATDETECTOR_STATE_MAYBE_DETECTED:
            if ((sample + BEATDETECTOR_STEP_RESILIENCY) < threshold) 
			{
                // Found a beat
                beatDetected = true;
                lastMaxValue = sample;
                state = BEATDETECTOR_STATE_MASKING;
                float delta = DRV_MAX30100_checkTimeDiff(tsLastBeat);
                if (delta) 
				{
                    beatPeriod = BEATDETECTOR_BPFILTER_ALPHA * delta +
                            (1 - BEATDETECTOR_BPFILTER_ALPHA) * beatPeriod;
                }
                tsLastBeat = timer_ms;
            } 
			else 
			{
                state = BEATDETECTOR_STATE_FOLLOWING_SLOPE;
            }
            break;

        case BEATDETECTOR_STATE_MASKING:
            if (DRV_MAX30100_checkTimeDiff(tsLastBeat) > BEATDETECTOR_MASKING_HOLDOFF) 
			{
                state = BEATDETECTOR_STATE_WAITING;
            }
            DRV_MAX30100_decreaseThreshold();
            break;
    }

    return beatDetected;
}

void DRV_MAX30100_processSample(DRV_MAX30100_OBJ* dObj, uint16_t irData1) 
{
    if (DRV_MAX30100_checkTimeDiff(tsLastSample) > 1.0 / sampRate * 1000.0) 
	{
        tsLastSample = timer_ms;

        
        float irACValue;
        irACValue = DRV_MAX30100_dcRemoval(irData1);
        
        // The signal fed to the beat detector is mirrored since the cleanest monotonic spike is below zero
        float filteredPulseValue = DRV_MAX30100_filterBuLp1((uint16_t) -irACValue);
        
        beatDetected = DRV_MAX30100_checkForBeat(filteredPulseValue);

        if (beatDetected) {
            bpmRate_old = bpmRate;
            bpmRate = DRV_MAX30100_getBPMRate();
            
            if (bpmRate_old == bpmRate){
                counter_same_value = counter_same_value - 1;
            }
            
            if(((bpmRate > 50) && (bpmRate < 180)) 
                     && ((bpmRate_old != bpmRate) 
                     || (counter_same_value == 0)) 
                        && (abs (bpmRate_old - bpmRate) < 10)
                        && ((bpmRate - last_value_considered) < 50) 
                        && (counter != 0)) {
                
                printf("\n\rMeasuring heart beat rate... Sample nr. %d",21-counter);
                items[itemCounter] =bpmRate;
                itemCounter++;
                counter = counter - 1;
                last_value_considered = bpmRate;
              
            }
            
            //Calibration finished
            if(counter == 0){
                dObj->taskState =  DRV_MAX30100_TASK_STATE_READ_IR_RED_COMPLETE;
                return;
            }
            
            if (counter_same_value == 0){
                counter_same_value = COUNTER_SAME_VALUE;
            }
           
            
        }
    }
            
}        

void DRV_MAX30100_processHeartbeat(DRV_MAX30100_OBJ* dObj, uint16_t irData1)
{
        dObj->taskState =  DRV_MAX30100_TASK_STATE_IDLE;
        if(((bpmRate > 40) && (bpmRate < 180)) 
             && ((bpmRate_old != bpmRate) 
             || (counter_same_value == 1)) 
             && (abs (bpmRate_old - bpmRate) < 10)
             && ((bpmRate - last_value_considered) < 50) ) {
                printf("\n\rMeasuring heart beat rate...");
                DRV_MAX30100_deQueue();
                DRV_MAX30100_enQueue(bpmRate);
                sum = 0;
                DRV_MAX30100_Add_Samples();
                average = sum/COUNTER_RESET_VALUE;
                heart_rate = round(average);
                printf("\n\rHeart Rate:%.0f bpm\n", heart_rate);
                last_value_considered = bpmRate;
                dObj->check_read = true;
                }
        
        if (counter_same_value == 0){
                counter_same_value = COUNTER_SAME_VALUE;
            }
}

/* Generate the driver handle based upon the driver index and client ID */
static inline uint32_t _DRV_MAX30100_MAKE_HANDLE(uint8_t drvIndex, uint8_t clientIndex)
{
    return ((drvIndex << 8) | clientIndex);
}

/* This function will be called by I2C PLIB when transfer is completed */
static void _DRV_MAX30100_PLIBEventHandler(uintptr_t context)
{
    DRV_MAX30100_OBJ* dObj = (DRV_MAX30100_OBJ*) context;
    DRV_MAX30100_CLIENT_OBJ* clientObj = NULL;
    DRV_MAX30100_ERROR error;
    
    if (dObj == NULL)
    {
        return;
    }
    
    clientObj = dObj->activeClient;
    error = dObj->plibInterface->errorGet(); 
    
    if (error != DRV_MAX30100_ERROR_NONE)
    {
        dObj->status = SYS_STATUS_READY;
        dObj->activeClient = NULL;
        dObj->taskState = DRV_MAX30100_TASK_STATE_ERROR;
        return;
    }
    
    /* test for initial calibration stage */
    if (dObj->event == DRV_MAX30100_EVENT_WRITE_DONE)
    {
        /* the write operation completed. The next device state will already
         * have been written to the taskState so allow the transition by
         * setting SYS_STATUS_READY */
        dObj->status = SYS_STATUS_READY;
        dObj->activeClient = NULL;
        return;
    } 
    else if (dObj->event == DRV_MAX30100_EVENT_READ_DONE)
    {
        /* tell the task state machine to advance to the next state */
        dObj->taskState = dObj->nextTaskState;
        /* put the next state into error in case of an erroneous callback*/
        dObj->nextTaskState = DRV_MAX30100_TASK_STATE_ERROR;
        
        dObj->status = SYS_STATUS_READY;        
        dObj->activeClient = NULL;
    }
    
    if (clientObj != NULL) 
    {
        if (clientObj->callback != NULL)
        {
            clientObj->callback(DRV_MAX30100_TRANSFER_STATUS_COMPLETED, clientObj->context);
        }
    }
}

static DRV_MAX30100_CLIENT_OBJ* _DRV_MAX30100_ClientObjGet(const DRV_HANDLE handle)
{
    uint32_t drvIndex = handle >> 8;
    uint32_t clientIndex = handle & 0xFF;
    
    if (drvIndex < DRV_MAX30100_INSTANCES_NUMBER)
    {
        if (clientIndex < gDrvMAX30100Obj[drvIndex].nClientsMax)
        {
            if (gDrvMAX30100Obj[drvIndex].clientObjPool[clientIndex].inUse == true)
            {
                return &gDrvMAX30100Obj[drvIndex].clientObjPool[clientIndex];
            }
        }
    }
    
    return NULL;
}

static int32_t _DRV_MAX30100_FreeClientGet(DRV_MAX30100_OBJ* dObj)
{
    for (int32_t i = 0; i < dObj->nClientsMax; i++) 
    {
        if (dObj->clientObjPool[i].inUse == false)
        {
            dObj->clientObjPool[i].inUse = true;
            return i;
        }
    }
    
    return -1;
}


SYS_STATUS DRV_MAX30100_Status( const SYS_MODULE_INDEX drvIndex )
{
    DRV_MAX30100_OBJ* dObj = NULL;
    
    if (drvIndex >= DRV_MAX30100_INSTANCES_NUMBER)
    {
        return SYS_STATUS_ERROR;
    }
    
    /* if the driver is still initializing or in the middle of a read 
     *  return a BUSY status to the application code rather than the true status */
    dObj = &gDrvMAX30100Obj[drvIndex];
    if (dObj->taskState != DRV_MAX30100_TASK_STATE_IDLE)
    {
        return SYS_STATUS_BUSY;
    }
    
    /* Return the actual driver status */
    return dObj->status;

}


void DRV_MAX30100_ClientEventHandlerSet(
    const DRV_HANDLE handle,
    const DRV_MAX30100_APP_CALLBACK callback,
    const uintptr_t context
)
{
    DRV_MAX30100_CLIENT_OBJ* clientObj = _DRV_MAX30100_ClientObjGet(handle);
    
    if (clientObj != NULL)
    {
        clientObj->callback = callback;
        clientObj->context = context;
    }
}

SYS_MODULE_OBJ DRV_MAX30100_Initialize( const SYS_MODULE_INDEX drvIndex, const SYS_MODULE_INIT * const init)
{
    DRV_MAX30100_OBJ* dObj = NULL;
    DRV_MAX30100_INIT *MAX30100Init = (DRV_MAX30100_INIT *)init;

    /* Validate the request */
    if(drvIndex >= DRV_MAX30100_INSTANCES_NUMBER)
    {
        return SYS_MODULE_OBJ_INVALID;
    }
    else
    {
        dObj = &gDrvMAX30100Obj[drvIndex];
    }
    
    dObj->status = SYS_STATUS_UNINITIALIZED;
    
    if(dObj->inUse == true)
    {
        return SYS_MODULE_OBJ_INVALID;
    }

    /* initialize the parameters */
    dObj->inUse = true;
    dObj->nClients = 0;
    dObj->activeClient = NULL;
    dObj->plibInterface = MAX30100Init->plibInterface;
    dObj->configParams = MAX30100Init->configParams;
    dObj->clientObjPool = (DRV_MAX30100_CLIENT_OBJ*) MAX30100Init->clientObjPool;
    dObj->nClientsMax = MAX30100Init->maxClients;
    dObj->plibInterface->callbackRegister(_DRV_MAX30100_PLIBEventHandler, (uintptr_t) dObj);
    dObj->taskState = DRV_MAX30100_TASK_STATE_INIT;
    dObj->check_read = false;
    /* set status */
    dObj->status = SYS_STATUS_READY;
    
    /* the SYS_MODULE_OBJ for this driver will be the index */
    return (SYS_MODULE_OBJ) drvIndex;
}

bool DRV_MAX30100_readSensor(const DRV_HANDLE handle){
    
    DRV_MAX30100_OBJ* dObj;
    DRV_MAX30100_CLIENT_OBJ* clientObj = NULL;
   
    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
    }
    
    clientObj = _DRV_MAX30100_ClientObjGet(handle);
    if ((clientObj == NULL) || (clientObj->drvIndex >= DRV_MAX30100_INSTANCES_NUMBER))
    {
        return false;
    }
    
    dObj = &gDrvMAX30100Obj[clientObj->drvIndex];
    
    dObj->status = SYS_STATUS_BUSY;
    dObj->activeClient = clientObj;
    dObj->taskState = DRV_MAX30100_TASK_STATE_READ_IR_RED;
    dObj->nextTaskState = DRV_MAX30100_TASK_STATE_READ_IR_RED_COMPLETE;
    dObj->event = DRV_MAX30100_EVENT_READ_DONE;
    
    /* send the request */
    dObj->writeBuffer[0] = MAX30100_FIFO_DATA_REG;
    dObj->plibInterface->writeRead(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 1, (void*) dObj->readBuffer, 4);  
    
    return true;
    
}

DRV_HANDLE DRV_MAX30100_Open(const SYS_MODULE_INDEX drvIndex, const DRV_IO_INTENT ioIntent)
{
    DRV_MAX30100_OBJ* dObj = NULL;
    DRV_MAX30100_CLIENT_OBJ* clientObj = NULL;
    int32_t iClient;
    
    /* Validate the request */
    if (drvIndex >= DRV_MAX30100_INSTANCES_NUMBER)
    {
        return DRV_HANDLE_INVALID;
    }

    dObj = &gDrvMAX30100Obj[drvIndex];
    
    /* Flag error if: 
     * Driver is already opened by maximum configured clients
     */
    if((dObj->status != SYS_STATUS_READY) || 
            (dObj->nClients >= dObj->nClientsMax))
    {
        return DRV_HANDLE_INVALID;
    }

    /* get a free client handle */
    iClient = _DRV_MAX30100_FreeClientGet(dObj);
    if (iClient >= 0)
    {
        clientObj = &dObj->clientObjPool[iClient];
        clientObj->drvIndex = drvIndex;            
        dObj->nClients++;
        
        return (_DRV_MAX30100_MAKE_HANDLE(drvIndex, iClient));
    }
    
    return DRV_HANDLE_INVALID;
}


void DRV_MAX30100_Close(const DRV_HANDLE handle)
{
    DRV_MAX30100_CLIENT_OBJ* clientObj = _DRV_MAX30100_ClientObjGet(handle);
    DRV_MAX30100_OBJ* dObj = NULL;
    
    if (clientObj != NULL)
    {
        dObj = &gDrvMAX30100Obj[clientObj->drvIndex];
        dObj->nClients--;
        clientObj->inUse = false;
    }
    
}


bool DRV_MAX30100_getSampleCount(const DRV_HANDLE handle, uint8_t* sampleNr)
{
    DRV_MAX30100_OBJ* dObj;
    DRV_MAX30100_CLIENT_OBJ* clientObj = NULL;
    
    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
        
    }
    
    clientObj = _DRV_MAX30100_ClientObjGet(handle);
   
    if((clientObj == NULL) || (clientObj->drvIndex >= DRV_MAX30100_INSTANCES_NUMBER))
    {
        return false;
    }
    
    dObj = &gDrvMAX30100Obj[clientObj->drvIndex];
    
    *sampleNr = dObj->data.sampleNr;
    
    return false;
}


bool DRV_MAX30100_getREDandIRdata(const DRV_HANDLE handle, uint16_t* REDdata, uint16_t* IRdata)
{
    DRV_MAX30100_OBJ* dObj;
    DRV_MAX30100_CLIENT_OBJ* clientObj = NULL;
    
    if (handle == DRV_HANDLE_INVALID)
    {
        return false;
        
    }
    
    
    clientObj = _DRV_MAX30100_ClientObjGet(handle);
    
    if ((clientObj == NULL) || (clientObj->drvIndex >= DRV_MAX30100_INSTANCES_NUMBER))
    {
        return false;
    }
    
    dObj = &gDrvMAX30100Obj[clientObj->drvIndex];
    
    *REDdata = dObj->data.REDdata;
    *IRdata = dObj->data.IRdata;
    
    return false;
}


void DRV_MAX30100_reset(DRV_MAX30100_OBJ* dObj)
{
    
    dObj->taskState = DRV_MAX30100_TASK_STATE_READ_ID;
    dObj->activeClient = NULL;
    dObj->status = SYS_STATUS_BUSY;
    dObj->event = DRV_MAX30100_EVENT_WRITE_DONE;  
    /* configure the i2c interface */
    dObj->plibInterface->transferSetup(&dObj->configParams.transferParams, 0);
            
    /* send the reset request */
    dObj->writeBuffer[0] = MAX30100_MODE_CONFIG_REG;
    dObj->writeBuffer[1] = MAX30100_RESET_VALUE;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
    
}

void DRV_MAX30100_FIFOclear(DRV_MAX30100_OBJ* dObj)
{
    if (dObj == NULL)
    {
        return;
    }
      
    dObj->taskState = DRV_MAX30100_TASK_STATE_READ_ID;
    dObj->activeClient = NULL;
    dObj->status = SYS_STATUS_BUSY;
    dObj->event = DRV_MAX30100_EVENT_WRITE_DONE; 
    
    /* send the reset request */
    dObj->writeBuffer[0] = MAX30100_FIFO_WR_PTR_REG;
    dObj->writeBuffer[1] = 0x0;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
    
    /* send the reset request */
    dObj->writeBuffer[0] = MAX30100_FIFO_RD_PTR_REG;
    dObj->writeBuffer[1] = 0x0;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
    
    /* send the reset request */
    dObj->writeBuffer[0] = MAX30100_OVF_CTR_REG;
    dObj->writeBuffer[1] = 0x0;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
    
    
}

/*A modification of the usual _DRV_SENSOR_ReadReg()*/
static void DRV_MAX30100_readBlock(DRV_MAX30100_OBJ* dObj, uint8_t reg, uint8_t length) 
{
    if (dObj == NULL)
    {
        return;
    }
    
    /* this function is used by the driver itself so there is no active client */
    dObj->activeClient = NULL;
    /* pass this event to the peripheral callback when the read is completed */
    dObj->event = DRV_MAX30100_EVENT_READ_DONE;
    /* mark the driver as BUSY */
    dObj->status = SYS_STATUS_BUSY;
            
    /* send the request */
    dObj->writeBuffer[0] = reg;
    dObj->plibInterface->writeRead(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 1, (void*) dObj->readBuffer, length);      
    while(SERCOM7_I2C_IsBusy());
      
 

    
}

void DRV_MAX30100_getPartID(DRV_MAX30100_OBJ* dObj)
{
    uint8_t reg = 0x00;
    
    if (dObj == NULL)
    {
        return;
    }
    
    dObj->activeClient = NULL;
    dObj->status = SYS_STATUS_BUSY;
    dObj->event = DRV_MAX30100_EVENT_READ_DONE;
    
    dObj->writeBuffer[0] = MAX30100_PART_ID_REG;
    dObj->plibInterface->writeRead(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 1, (void*) dObj->readBuffer, 1); 
    while(SERCOM7_I2C_IsBusy());
    
    reg = dObj->readBuffer[0];
    dObj->deviceID = dObj->readBuffer[0];
    printf("Verifying device ID...\r\n");
    printf("Device ID: %d\r\r\n\n", reg);
   
}

void DRV_MAX30100_shutdown(DRV_MAX30100_OBJ* dObj)
{
    uint8_t reg = 0x00;
    
    if (dObj == NULL)
    {
        return;
    }
    
    dObj->writeBuffer[0] = MAX30100_MODE_CONFIG_REG;
    dObj->plibInterface->writeRead(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 1, (void*) dObj->readBuffer, 1); 
    while(SERCOM7_I2C_IsBusy());
    
    reg= reg | 0x7F;
    dObj->writeBuffer[0] = MAX30100_MODE_CONFIG_REG;
    dObj->writeBuffer[0] = reg;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
}


void DRV_MAX30100_setLEDs(DRV_MAX30100_OBJ* dObj)
{
    
    if (dObj == NULL)
    {
        return;
    }
    dObj->status = SYS_STATUS_BUSY;
    dObj->event = DRV_MAX30100_EVENT_WRITE_DONE;
    dObj->taskState = DRV_MAX30100_TASK_STATE_IDLE;
    
    dObj->writeBuffer[0] = MAX30100_SPO2_CONFIG_REG;
    dObj->writeBuffer[1] = 0x07;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
  
    
    dObj->writeBuffer[0] = MAX30100_LED_CONFIG_REG ;
    dObj->writeBuffer[1] = 0xFF;
   // dObj->writeBuffer[2] = MAX30100_I50;
    dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
    while(SERCOM7_I2C_IsBusy());
    
}


void DRV_MAX30100_Tasks(SYS_MODULE_OBJ object)
{
    DRV_MAX30100_OBJ* dObj = NULL;
    
    if ((object == SYS_MODULE_OBJ_INVALID) ||
        (object >= DRV_MAX30100_INSTANCES_NUMBER))
    {
        /* invalid system object */
        return;
    }
    
    dObj = &gDrvMAX30100Obj[object];
   
    
    if (dObj->status == SYS_STATUS_BUSY)
    {
        return;
    }
    
    
    switch (dObj->taskState)
    {
        case DRV_MAX30100_TASK_STATE_INIT:
           DRV_MAX30100_reset(dObj);
           stop =0;
           irFilters.dcW = 0;
           irFilters.v_ctr[0] = 0;
           irFilters.v_ctr[1] = 0;
           tsLastSample = 0;
           state = BEATDETECTOR_STATE_INIT;
           threshold = BEATDETECTOR_MIN_THRESHOLD;
           beatPeriod = 0;
           lastMaxValue = 0;
           tsLastBeat = 0;
           tsLastReport = 0;
           bpmRate = 0;
           
           dObj->taskState = DRV_MAX30100_TASK_STATE_READ_ID;
            
            break;
        
        case DRV_MAX30100_TASK_STATE_READ_ID:
            /* read the device ID */        
            DRV_MAX30100_getPartID(dObj);
            
            dObj->taskState = DRV_MAX30100_TASK_STATE_SENSOR_CONFIG;
            break;
           
            
        case DRV_MAX30100_TASK_STATE_SENSOR_CONFIG:
            /* Configure the sensor*/
            
            // Initialize Settings
            interrupt_stat_bits.interruptStat = 0x00;
            mode_config_bits.shdn = DISABLED;
            mode_config_bits.reset = DISABLED;

            // Interrupts
            dObj->writeBuffer[0] = MAX30100_INTERRUPT_STAT_REG;
            dObj->writeBuffer[1] = interrupt_stat_bits.interruptStat;
            dObj->writeBuffer[2]= interrupt_en_bits.interruptEn;
            dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 3);  
            while(SERCOM7_I2C_IsBusy());
            
            // Configurations
            dObj->writeBuffer[0] = MAX30100_MODE_CONFIG_REG;
            dObj->writeBuffer[1] = MAX30100_HR_ONLY;
            dObj->plibInterface->write(dObj->configParams.sensorAddr, (void*) dObj->writeBuffer, 2);
            while(SERCOM7_I2C_IsBusy());
            dObj->taskState = DRX_MAX30100_TASK_STATE_SET_LED;
            
            break;
  
            
        case DRV_MAX30100_TASK_STATE_CLEAR_COUNTERS:
           
            DRV_MAX30100_FIFOclear(dObj);
            dObj->nextTaskState = DRX_MAX30100_TASK_STATE_SET_LED;
            break;

            
        case DRX_MAX30100_TASK_STATE_SET_LED:
            /* read is currently being performed and we are waiting for the result
             * the peripheral callback will advance the state to PROCESS_READ */
            DRV_MAX30100_setLEDs(dObj);
            dObj->taskState = DRV_MAX30100_TASK_STATE_READ_IR_RED;
            break;       
            
        
        case DRV_MAX30100_TASK_STATE_READ_IR_RED:
            DRV_MAX30100_readBlock(dObj,MAX30100_FIFO_DATA_REG,4);
            
            dObj->taskState = DRV_MAX30100_TASK_STATE_READ_SAMPLES;
            /* read is currently being performed and we are waiting for the result
             * the peripheral callback will advance the state to PROCESS_READ */
          
            break;  
            
      
        case DRV_MAX30100_TASK_STATE_READ_SAMPLES:            
                /* reading IR and RED led data complete */
              //  DRV_MAX30100_shutdown(dObj);
                dObj->data.IRdata = (int16_t) DRV_MAX30100_CONCAT_BYTES(dObj->readBuffer[0] , dObj->readBuffer[1]);
                //printf("%d\r\n",dObj->data.IRdata );
                dObj->data.REDdata = (int16_t) DRV_MAX30100_CONCAT_BYTES(dObj->readBuffer[2] , dObj->readBuffer[3]);
                //Obj->data.IRdata = meanDiff(dObj->data.IRdata);
                dObj->taskState = DRV_MAX30100_TASK_STATE_IDLE;
                DRV_MAX30100_processSample(dObj,dObj->data.IRdata);
                DRV_MAX30100_TMR_CallBack_Counter();
               // printf("%d\r\n",dObj->data.IRdata);
                
                
            break;
            
        
        case DRV_MAX30100_TASK_STATE_READ_IR_RED_COMPLETE:
            DRV_MAX30100_processHeartbeat(dObj,dObj->data.IRdata);
            break;
            
        case DRV_MAX30100_TASK_STATE_IDLE:
            dObj->taskState = DRV_MAX30100_TASK_STATE_READ_IR_RED;
           
            /* do nothing until the next read interval or async read */
            break;
            
        
        case DRV_MAX30100_TASK_STATE_ERROR:
            DRV_MAX30100_shutdown(dObj);
            break;
    
    }
}
/* *****************************************************************************
 End of File
 */
