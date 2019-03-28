/*++
File Name: testo350.h
Date: 2019/03/17
Author: Yundong Ma
Description:
--*/
#ifndef __TESTO350_H__
#define __TESTO350_H__
#if defined(__cplusplus)
  extern "C" {          /* Make sure we have C-declarations in C++ programs */
#endif
#include <stdio.h>  
#include <math.h> 
#include <string.h>
#include <sys/types.h>  
#include <libusb.h> 
////////////////////////////////////////////////
// DEFINE
// error return codes
#define RESULTEMUSB_OK                    0
#define RESULTEMUSB_FAIL                  -1
#define RESULTEMUSB_INVALID_PARAM      -101   // parameter out of range
#define RESULTEMSUB_CONNECT_FAILED     -102   // connection to USB device failed
#define RESULTEMUSB_WRITE_FAILED       -103   // write to device failed
#define RESULTEMUSB_READ_FAILED        -104   // write to device failed

#define DEFAULT_TIMEOUT                5000 // default timeout for read operation in msec

// ProductID of testo (is always 0x128D)
#define PRODUCT_ID_TESTO            0x128D

// VendorID for testo 350_2010 and testo 330_2010
#define VENDOR_ID_TESTO350_CU       0x000E
#define VENDOR_ID_TESTO350_MB       0x000F
#define VENDOR_ID_TESTO330_FL       0x0014
#define VENDOR_ID_TESTO350_CU_DEV   0x100E
#define VENDOR_ID_TESTO350_MB_DEV   0x100F
#define VENDOR_ID_TESTO330_FL_DEV   0x1014
///////////////////////////////////////////////
// ENUM
typedef enum _E_MEAS_TYPE
{
   MEAS_IDLE = 0, /* not used*/
   MEAS_FLUEGAS                 = 1, /* fluegas */
   MEAS_DELTAP                  = 2, /* dP-measurement*/
   MEAS_DRAUGHT                 = 3, /* draft-measurement*/
   MEAS_FLUEGAS_MS              = 5, /* fluegas + velocity*/
   MEAS_FLUEGAS_DP              = 6, /* fluegas + dP*/
   MEAS_UNKNOWN = -1, /* unknown */
} E_MEAS_TYPE;
typedef enum _E_MEAS_APP_STATES
{
   APP_STATE_STARTUP = 0, /* measurement application is not initialized*/
   APP_STATE_IDLE = 1, /* measurement application idle state*/
   APP_STATE_RUNNING = 2, /* measurement application running state*/
   APP_STATE_ZERO = 3, /* measurement application zero state*/
   APP_STATE_RINSE = 4, /* measurement application rinse state*/
   APP_STATE_WANTS_START = 5, /* measurement application zero state with automatic start afterwards */
} E_MEAS_APP_STATES;
typedef enum _E_MEAS_ACTION
{
   MEAS_START = 0, /* Messung starten*/
   MEAS_STOP = 1, /* Messung stoppen*/
   MEAS_CANCEL = 2, /* Messung abbrechen*/
   MEAS_ZERO = 3, /* Nullung*/
   MEAS_RINSE = 4, /* Spülen*/
   MEAS_MAX_ACTIONS = 5, /* */
   MEAS_ZERO_FORCE = 6, /* Nullung immer durchführen*/
   MEAS_RINSE_FORCE = 7, /* Spülung immer durchführen*/
   MEAS_VIEW_SAVE = 8, /* */
   MEAS_MAX_VIEW_ACTIONS = 9, /* */
} E_MEAS_ACTION;
typedef enum _E_FILTER
{
   FILTER_ALL = 0, /* Gibt alle Messwerte zurück*/
   FILTER_USER = 1, /* Gibt nur die Userspezifischen Messwerte zurück*/
   FILTER_MEAS = 2, /* Gibt nur die gemessenenen Messwerte zurück*/
   FILTER_NONEDUPLICATE = 3, /* Falls doppelte Messwerte oder Leerzeilen (VIEW_USER_EMPTY) vorhanden sind, werden diese herausgefiltert*/
} E_FILTER;
typedef enum _E_DEVICETYPES
{
   E_TYPE_T350_CU = 0, /* Control unit*/
   E_TYPE_T350_BOX = 1, /* measurement box*/
   E_TYPE_T350_ANALOG = 2, /* analog box*/
   E_TYPE_T330 = 3, /* testo 330 2010*/
   E_TYPE_UNKNOWN = -1, /* unknown device*/
} E_DEVICETYPES;
typedef enum _E_DEVICE_MODE
{
   NORMAL = 0, /* standard measure phase*/
   TEST = 1, /* testo mode*/
   SLAVE = 2, /* slave mode*/
} E_DEVICE_MODE;
typedef enum _E_APPLICATION
{
   APPLICATION_NONE = -1, /* keine Applikation ausgewählt*/
   APPLICATION_BURNER = 0, /* */
   APPLICATION_TURBINE = 1, /* */
   APPLICATION_ENGINE_1 = 2, /* */
   APPLICATION_ENGINE_2 = 3, /* */
   APPLICATION_CUSTOMIZED = 4, /* */
   APPLICATION_NUM_OF = 5, /* */
} E_APPLICATION;
typedef enum _MEAS_VIEW_STATES
{
   E_MEAS_VIEW_STATE_STARTUP = 0, /* measurement application is not initialized*/
   E_MEAS_VIEW_STATE_IDLE = 1, /* measurement application idle state*/
   E_MEAS_VIEW_STATE_RUNNING = 2, /* measurement application running state*/
   E_MEAS_VIEW_STATE_ZERO = 3, /* measurement application zero state*/
   E_MEAS_VIEW_STATE_RINSE = 4, /* measurement application rinse state*/
   E_APP_STATE_ZERO_WANTS_START = 5, /* measurement application zero state with automatic start afterwards*/
   E_MEAS_VIEW_STATE_DEADTIME = 6, /* dead time state*/
   E_MEAS_VIEW_STATE_STABILZATION = 7, /* stabilization state formerly Ramp up*/
   E_MEAS_VIEW_STATE_WAIT_ZERO = 8, /* measprogram waits before the zeroing state for the user to prepare the device*/
   E_MEAS_VIEW_STATE_WAIT_RAMP = 9, /* measprogram waits before the ramp up state for the user to prepare the device*/
   E_MEAS_VIEW_STATE_WAIT_TEST = 10, /* measprogram waits before the test data state for the user to prepare the device*/
   E_MEAS_VIEW_STATE_WAIT_RINSE = 11, /* measprogram waits before the rinse state for the user to prepare the device*/
   E_MEAS_VIEW_STATE_WAIT_EXIT = 12, /* measprogram waits before the exit state for the user to prepare the device*/
   E_MEAS_MAX_VIEW_STATES = 13, /* */
} MEAS_VIEW_STATES;
///////////////////////////////////////////////////
// STRUCT
typedef struct _MEAS_STATUS
{
   uint32_t dwCurrentValue; /*  */
   uint32_t dwMaxValue; /*  */
   uint8_t byMeasState; /* see MEAS_VIEW_STATES */
   uint8_t byMeasProgState; /* Aktueller Zustand des Messprogramms */
   uint16_t wCurrentCycle; /* Aktueller Zyklus des Messprogramms */
   uint16_t wMaxCycle; /* Anzahl der Zyklen des Messprogramms */
} __attribute__((packed)) MEAS_STATUS;

typedef struct _TCHANNELINFO
{
   uint8_t byCon; /* connector info */
   uint32_t dwIdent; /* channel type */
} __attribute__((packed)) TCHANNELINFO;

typedef struct _TDEVICETYPE
{
   uint8_t byID; /* eindeutige ID */
   uint8_t deviceType; /* devicetyp (see enum E_DEVICETYPE) */
} __attribute__((packed)) TDEVICETYPE;

typedef union _UFLOATINT
{
   float fValue; /* value*/
   uint32_t dwError; /* interpretation of value as error state*/
} __attribute__((packed)) UFLOATINT;

typedef struct _TMEASVALUE
{
   UFLOATINT Value; /* value */
   uint16_t wUnit; /* unit */
   signed char byExp; /* resolution */
} __attribute__((packed)) TMEASVALUE;

typedef struct _TVIEWVALUE
{
   TCHANNELINFO tChannelInfo; /* Kanal Infosds */
   TMEASVALUE tMeasValue; /* measure value */
   float fDilutionFactor; /* Verdünnungsfaktor */
}  __attribute__((packed)) TVIEWVALUE;

typedef struct _TVIEWVALUELIST
{
   uint16_t EApplication; /* Application EAPPLICATION */
   uint8_t EStatistic; /* Statistic   ESTATISTIC */
   uint8_t byNumberOfValues; /* Number of Values */
} __attribute__((packed)) TVIEWVALUELIST;

///////////////////////////////////////////////
// FUNCTION
// enumeration
libusb_device_handle* TESTOEMUSB_GetSerialNumber(unsigned short usVid, unsigned short usPid, uint32_t* pdwSerialNumber);
// open / close connection
int TESTOEMUSB_OpenBySerialNumber(libusb_device_handle* hDevice, uint32_t dwSerialNumber);
int TESTOEMUSB_Close(libusb_device_handle* hDevice);

// read / write
int TESTOEMUSB_Write(libusb_device_handle* hDevice, unsigned char* pWrBuffer, int iBytesToWrite );
int TESTOEMUSB_Read(libusb_device_handle* hDevice, unsigned char*  pRdBuffer, int iBytesToRead, int* piBytesRead );
 /**
    * @brief get/set serial number
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param dwSerialNumber
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetSerialNumberBox(libusb_device_handle * hDevice, uint32_t* pdwSerialNumber);
   /**
    * @brief current selected application
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bApplication
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetApplication(libusb_device_handle * hDevice, E_APPLICATION* pbApplication);
   /**
    * @brief current selected application
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bApplication
    * @return HRESULT (error return code)
    */ 
int TESTO350_SetApplication(libusb_device_handle * hDevice, E_APPLICATION bApplication);
	/**
	 * @brief current selected meas type
	 * @param hDevice
	 * @param bMeasType
	 * @return HRESULT (error return code)
	 */
int TESTO350_SetMeasType(libusb_device_handle* hDevice, E_MEAS_TYPE bMeasType);
	/**
	 * @brief current selected meas type
	 * @param hDevice
	 * @param pbMeasType
	 * @return HRESULT (error return code)
	 */
int TESTO350_GetMeasType(libusb_device_handle* hDevice, E_MEAS_TYPE* pbMeasType);
   /**
    * @brief current selected meas action
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bMeasAction
    * @return HRESULT (error return code)
    */ 
int TESTO350_SetMeasAction(libusb_device_handle * hDevice, E_MEAS_ACTION bMeasAction);
	/**
	 * @brief current selected meas action
	 * @param hDevice
	 * @param bMeasAction
	 * @return HRESULT (error return code)
	 */
int TESTO350_GetMeasAction(libusb_device_handle* hDevice, E_MEAS_ACTION* pbMeasAction);
	/**
	 * @brief NO2 Zuschlagskoeffizient
	 * @param hDevice
	 * @param measValue
	 * @return HRESULT (error return code)
	 */
int TESTO350_SetNO2Addition(libusb_device_handle * hDevice, TMEASVALUE measValue);
   /**
    * @brief NO2 Zuschlagskoeffizient
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param measValue
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetNO2Addition(libusb_device_handle * hDevice, TMEASVALUE* pmeasValue);
   /**
    * @brief O2 Referenzwert
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param measValue
    * @return HRESULT (error return code)
    */ 
int TESTO350_SetO2Ref(libusb_device_handle * hDevice, TMEASVALUE measValue);
   /**
    * @brief O2 Referenzwert
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param measValue
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetO2Ref(libusb_device_handle * hDevice, TMEASVALUE* pmeasValue);
   /**
    * @brief set/get current selected measure type (see E_MEAS_TYPE)
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bActualMesssType
    * @return HRESULT (error return code)
    */ 
int TESTO350_SetCurrentMeasurement(libusb_device_handle * hDevice, uint8_t bActualMesssType);
   /**
    * @brief set/get current selected measure type (see E_MEAS_TYPE)
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bActualMesssType
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetCurrentMeasurement(libusb_device_handle * hDevice, uint8_t* pbActualMesssType);
   /**
    * @brief set device mode (slave mode, electral calibration,....)
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param eDeviceMode
    * @return HRESULT (error return code)
    */ 
int TESTO350_SetDeviceMode(libusb_device_handle * hDevice, E_DEVICE_MODE eDeviceMode);
   /**
    * @brief set device mode (slave mode, electral calibration,....)
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param eDeviceMode
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetDeviceMode(libusb_device_handle * hDevice, E_DEVICE_MODE* peDeviceMode);
   /**
    * @brief returns the number of measure values in the current box and the current measurement
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bFilter desired filter (see EFILTER)
    * @param wCountMeasureValues number of measure values
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetNumberOfCurrentViewValues(libusb_device_handle * hDevice , E_FILTER bFilter, uint16_t* wCountMeasureValues);
   /**
    * @brief list with all IDs of the connected boxes
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bCountBoxes number of measure boxes
    * @param tDeviceType list of all connected analyzers
    * @param uMaxLengthOut  max size of previous structure (in number of structure elements)
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetBoxList(libusb_device_handle * hDevice, uint8_t* bCountBoxes, TDEVICETYPE* tDeviceType, unsigned int uMaxLengthOut);
   /**
    * @brief returns all measure values of the current measure box and the current measurement
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param bFilter desired filter (see EFILTER)
    * @param tViewValueList Informationstruct about view values
    * @param tViewValues array with all yiew values
    * @param uMaxLengthOut  max size of previous structure (in number of structure elements)
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetCurrentViewValues(libusb_device_handle * hDevice, E_FILTER bFilter, TVIEWVALUELIST* tViewValueList, TVIEWVALUE* tViewValues, unsigned int uMaxLengthOut);
   /**
    * @brief Set state of analyzer (e.g. zeroing: set MEAS_ZERO)
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param tMeasStatus 
    * @return HRESULT (error return code)
    */ 
int TESTO350_SetMeasStatus(libusb_device_handle * hDevice, MEAS_STATUS tMeasStatus);
   /**
    * @brief Get state of analyzer
    * @param uDeviceID Id of device returned by TESTOEMUSB_OpenBySerialNumber
    * @param tMeasStatus Measure Status structure
    * @return HRESULT (error return code)
    */ 
int TESTO350_GetMeasStatus(libusb_device_handle * hDevice, MEAS_STATUS* tMeasStatus);
#if defined(__cplusplus)
  }     /* Make sure we have C-declarations in C++ programs */
#endif
#endif // __TESTO350_H__
