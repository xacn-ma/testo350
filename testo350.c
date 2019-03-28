#include "testo350.h"
/////////////////////////////////////////////
#define END_POINT_OUT   0x02
#define END_POINT_IN    0x81
#define STX 0x22
/////////////////////////////////////////////
typedef struct _HEAD {
    uint8_t stx;
    uint32_t len;
} __attribute__((packed)) HEAD;
typedef struct _RESPONSE {
    HEAD head;
    uint8_t body[64];
} __attribute__((packed)) RESPONSE;
typedef struct _REPLAY {
    HEAD head;
    uint8_t* body;
} __attribute__((packed)) REPLAY;
/////////////////////////////////////////////
static libusb_device **g_devs; 
static libusb_context *g_ctx = NULL; 
static uint8_t g_uSerialNumber[4] = {0};
static uint8_t g_ident[6] = {0};
static int TESTOUSB_Trans(libusb_device_handle * hDevice, RESPONSE* resp, REPLAY* replay)
{
    if (0 == TESTOEMUSB_Write(hDevice, (const void*)&resp->head, sizeof(resp->head)))
    {
        if (0 == TESTOEMUSB_Write(hDevice, (const void*)resp->body, resp->head.len))
        {
            int transferred = 0;
            if (0 == TESTOEMUSB_Read(hDevice, (void*)&replay->head, sizeof(replay->head), &transferred))
            {
                transferred = 0;
                if (0 == TESTOEMUSB_Read(hDevice, (void*)replay->body, replay->head.len, &transferred))
                {
                    return RESULTEMUSB_OK;
                }
            }
        }
    }
    return RESULTEMUSB_FAIL;
}
libusb_device_handle* TESTOEMUSB_GetSerialNumber(unsigned short usVid, unsigned short usPid, uint32_t* pdwSerialNumber)
{
    *pdwSerialNumber = 0;
    if(libusb_init(NULL) < 0) 
    { 
        fprintf(stderr, "failed to initialise libusb\n"); 
        return NULL; 
    } 
    if(libusb_get_device_list(g_ctx, &g_devs) < 0) 
    { 
        perror("Get Device Error\n"); 
        return NULL; 
    } 
    libusb_free_device_list(g_devs, 1); 
    libusb_device_handle* hDevice = libusb_open_device_with_vid_pid(NULL, usVid, usPid); 
    if(hDevice)
    {
        if(libusb_kernel_driver_active(hDevice, 0) == 1) 
        { 
            printf("Kernel Driver Active\n"); 
            if(libusb_detach_kernel_driver(hDevice, 0) == 0)
            { 
                printf("Kernel Driver Detached!\n"); 
            } 
        }  
        if(libusb_claim_interface(hDevice, 0) < 0) { 
            perror("Cannot Claim Interface\n"); 
            libusb_close(hDevice); 
            hDevice = NULL; 
        } 
    }
    if (hDevice)
        TESTO350_GetSerialNumberBox(hDevice, pdwSerialNumber);
    return hDevice;
}
// open / close connection
int TESTOEMUSB_OpenBySerialNumber(libusb_device_handle* hDevice, uint32_t dwSerialNumber)
{
    RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 1,
        .body = {0x42}
        };
    REPLAY replay = {
        .body = g_ident
        };
    memcpy(&g_uSerialNumber, &dwSerialNumber, sizeof(dwSerialNumber));
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
    return 0;
}
int TESTOEMUSB_Close(libusb_device_handle* hDevice)
{
    if(libusb_release_interface(hDevice, 0) != 0)
    { 
        printf("Cannot Released Interface!\n"); 
    }
    libusb_close(hDevice); 
    libusb_exit(g_ctx); 
    return 0;
}

// read / write
int TESTOEMUSB_Write(libusb_device_handle* hDevice, unsigned char* pWrBuffer, int iBytesToWrite )
{
    int transferred = 0;
    /*printf("Write: %d\r\n", iBytesToWrite);
    for(int i = 0; i < iBytesToWrite; i++)
        printf("%02X ", *(pWrBuffer+i));
    printf("\r\n");*/
    int ret = libusb_bulk_transfer(hDevice, END_POINT_OUT, pWrBuffer, iBytesToWrite, &transferred, DEFAULT_TIMEOUT); 
    if(ret==0 && transferred==iBytesToWrite)
        return RESULTEMUSB_OK;
    else
        return RESULTEMUSB_WRITE_FAILED;
}
int TESTOEMUSB_Read(libusb_device_handle* hDevice, unsigned char*  pRdBuffer, int iBytesToRead, int* piBytesRead )
{
    int transferred = 0;
    int ret = libusb_bulk_transfer(hDevice, END_POINT_IN, pRdBuffer, iBytesToRead, &transferred, DEFAULT_TIMEOUT); 
    if (ret == 0)
    {
        /*printf("Read: %d\r\n", transferred);
        for (int i = 0; i < transferred; i++)
            printf("%02X ", *(uint8_t*)(pRdBuffer+i));
        printf("\r\n");*/
        if (piBytesRead == NULL)
        {
            if(transferred == iBytesToRead)
                return RESULTEMUSB_OK;
        }
        else
        {
            *piBytesRead = transferred;
            return RESULTEMUSB_OK;
        }
        
    }
    else
    {
        return RESULTEMUSB_READ_FAILED;
    }
}
int TESTO350_GetSerialNumberBox(libusb_device_handle * hDevice, uint32_t* pdwSerialNumber)
{
    uint8_t uSerialNumber[8] = {0};
    RESPONSE resp = {
        .head.stx = STX,
        .head.len = 1,
        .body = {0x40}
        };
    REPLAY replay = {
        .body = uSerialNumber
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        *pdwSerialNumber = *(uint32_t*)&uSerialNumber[4];
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetApplication(libusb_device_handle* hDevice, E_APPLICATION* pbApplication)
{
    RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 9,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1c, 0x25}
        };
    REPLAY replay = {
        .body = pbApplication
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetApplication(libusb_device_handle* hDevice, E_APPLICATION bApplication)
{
   RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1c, 0xa5, bApplication}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)&dwResult
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        if (dwResult == 0)
            return RESULTEMUSB_OK;
        else
            return RESULTEMUSB_FAIL;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetMeasType(libusb_device_handle* hDevice, E_MEAS_TYPE bMeasType)
{
	
   RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x21, 0xa5, bMeasType}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)&dwResult
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        if (dwResult == 0)
            return RESULTEMUSB_OK;
        else
            return RESULTEMUSB_FAIL;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetMeasType(libusb_device_handle* hDevice, E_MEAS_TYPE* bMeasType)
{
   RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x09,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x21, 0x25}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)bMeasType
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetMeasAction(libusb_device_handle* hDevice, E_MEAS_ACTION bMeasAction)
{
    MEAS_STATUS tMeasStatus;
    if (0 == TESTO350_GetMeasStatus(hDevice, &tMeasStatus))
    {
        tMeasStatus.byMeasState = bMeasAction;
        if (0 == TESTO350_SetMeasStatus(hDevice, tMeasStatus))
        {
            return RESULTEMUSB_OK;
        }
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetMeasAction(libusb_device_handle* hDevice, E_MEAS_ACTION* pbMeasAction)
{
    MEAS_STATUS tMeasStatus;
    if (0 == TESTO350_GetMeasStatus(hDevice, &tMeasStatus))
    {
        *pbMeasAction =  tMeasStatus.byMeasState;
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetNO2Addition(libusb_device_handle * hDevice, TMEASVALUE measValue)
{
    /*RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1d, 0xa5}
    };
    memcpy((void*)&resp.body[9], (void*)&measValue, sizeof(measValue));
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)&dwResult
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        if (dwResult == 0)
            return RESULTEMUSB_OK;
        else
            return RESULTEMUSB_FAIL;
    }*/
    return RESULTEMUSB_FAIL;
} 
int TESTO350_GetNO2Addition(libusb_device_handle * hDevice, TMEASVALUE* pmeasValue)
{
    /*RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x09,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1d, 0x25}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)pmeasValue
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }*/
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetO2Ref(libusb_device_handle * hDevice, TMEASVALUE measValue)
{
    /*RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1d, 0xa5}
    };
    memcpy((void*)&resp.body[9], (void*)&measValue, sizeof(measValue));
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)&dwResult
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        if (dwResult == 0)
            return RESULTEMUSB_OK;
        else
            return RESULTEMUSB_FAIL;
    }*/
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetO2Ref(libusb_device_handle * hDevice, TMEASVALUE* pmeasValue)
{
    /*RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x09,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1d, 0x25}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)pmeasValue
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }*/
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetCurrentMeasurement(libusb_device_handle* hDevice, uint8_t bActualMesssType)
{
    RESPONSE resp = {
        .head.stx = STX,
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x21, 0xa5, bActualMesssType}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = *(uint8_t*)&dwResult
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        if (dwResult == 0)
            return RESULTEMUSB_OK;
        else
            return RESULTEMUSB_FAIL;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetCurrentMeasurement(libusb_device_handle* hDevice, uint8_t* pbActualMesssType)
{
    RESPONSE resp = {
        .head.stx = STX,
        .head.len = 0x09,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x21, 0x25}
    };
    REPLAY replay = {
        .body = pbActualMesssType
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetDeviceMode(libusb_device_handle* hDevice, E_DEVICE_MODE eDeviceMode)
{

    return RESULTEMUSB_FAIL;
}
int TESTO350_GetDeviceMode(libusb_device_handle* hDevice, E_DEVICE_MODE* peDeviceMode)
{
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetNumberOfCurrentViewValues(libusb_device_handle* hDevice, E_FILTER bFilter, uint16_t* wCountMeasureValues)
{
    RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1a, 0x25, bFilter}
    };
    REPLAY replay = {
        .body = (uint8_t*)wCountMeasureValues
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetCurrentViewValues(libusb_device_handle* hDevice, E_FILTER bFilter, TVIEWVALUELIST* tViewValueList, TVIEWVALUE* tViewValues, unsigned int uMaxLengthOut)
{
    RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x0a,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1b, 0x25, bFilter}
    };
    uint8_t* buffer = (uint8_t*) malloc(sizeof(TVIEWVALUE)*uMaxLengthOut+sizeof(TVIEWVALUELIST));
    REPLAY replay = {
        .body = buffer
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        memcpy((void*)tViewValueList, (void*)buffer, sizeof(TVIEWVALUELIST));
        memcpy((void*)tViewValues, (void*)&buffer[sizeof(TVIEWVALUELIST)], sizeof(TVIEWVALUE)*uMaxLengthOut);
        free(buffer);
        return RESULTEMUSB_OK;
    }
    free(buffer);
    return RESULTEMUSB_FAIL;
}
int TESTO350_SetMeasStatus(libusb_device_handle* hDevice, MEAS_STATUS tMeasStatus)
{
    RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x09+sizeof(tMeasStatus),
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1d, 0xa5}
    };
    
    memcpy(&(resp.body[9]), &tMeasStatus, sizeof(tMeasStatus));
    REPLAY replay = {
        .body = (uint8_t*)&tMeasStatus
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
int TESTO350_GetMeasStatus(libusb_device_handle* hDevice, MEAS_STATUS* tMeasStatus)
{
    RESPONSE resp = {
        .head.stx = STX, 
        .head.len = 0x09,
        .body = {0x4f, g_ident[4], g_ident[5], g_uSerialNumber[0], g_uSerialNumber[1], 
            g_uSerialNumber[2], g_uSerialNumber[3], 0x1d, 0x25}
    };
    uint32_t dwResult = 0;
    REPLAY replay = {
        .body = (uint8_t*)tMeasStatus
        };
    if (RESULTEMUSB_OK == TESTOUSB_Trans(hDevice, &resp, &replay))
    {
        return RESULTEMUSB_OK;
    }
    return RESULTEMUSB_FAIL;
}
