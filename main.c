#include <signal.h>
#include "testo350.h"
#if 1
libusb_device_handle* dev_handle = NULL;
int exited = 1;
void sighandler(int sig)
{
    printf(" ----Quite Application!\r\n");
	exited = 0;
}
int main(void)
{
    uint32_t dwSerialNumber = 0;
    signal(SIGINT, sighandler);

    dev_handle = TESTOEMUSB_GetSerialNumber(PRODUCT_ID_TESTO, VENDOR_ID_TESTO350_MB, &dwSerialNumber);
    if (!dev_handle)
    {
        printf("Can't open VID=%04X, PID=%04X\r\n", PRODUCT_ID_TESTO, VENDOR_ID_TESTO350_MB);
        return -1;
    }
    printf("SerialNumber=%04X\r\n", dwSerialNumber);
    if (0 != TESTOEMUSB_OpenBySerialNumber(dev_handle, dwSerialNumber))
        printf("Can't open device sn:%08X\r\n", dwSerialNumber);
    if (0 != TESTO350_SetApplication(dev_handle, APPLICATION_TURBINE))
        printf("Can't set application\r\n");
    E_APPLICATION bApplication;
    if(TESTO350_GetApplication(dev_handle, &bApplication) !=0)
        printf("Can't get application\r\n");
    /*E_MEAS_TYPE bMeasType;
	if (0 != TESTO350_SetMeasType(dev_handle,MEAS_FLUEGAS))
		printf("Can't set meas type\r\n");
	if (0 != TESTO350_GetMeasType(dev_handle,&bMeasType))
		printf("Can't get meas type\r\n");*/
	E_MEAS_ACTION bMeasAction;
	if (0 != TESTO350_SetMeasAction(dev_handle,MEAS_START))
		printf("Can't set meas action\r\n");
	if (0 != TESTO350_GetMeasAction(dev_handle,&bMeasAction))
		printf("Can't get meas action\r\n");
	while(exited)
    {
        uint16_t wNumMeasVals;
        TVIEWVALUELIST tViewValueList;
        if (0 == TESTO350_GetNumberOfCurrentViewValues(dev_handle,FILTER_ALL,&wNumMeasVals))
        {
            TVIEWVALUE* ptViewValues = (TVIEWVALUE*) malloc(sizeof(TVIEWVALUE)*wNumMeasVals);
            if(ptViewValues != NULL)
            {
                if (0 == TESTO350_GetCurrentViewValues(dev_handle,FILTER_ALL, &tViewValueList, ptViewValues, wNumMeasVals))
                {
                    printf("=========================================================\r\n");
                    for (int i =0; i < wNumMeasVals; i++)
                    {
                        printf("\r\n   %08X   ", (*(TVIEWVALUE*)(ptViewValues+i)).tChannelInfo.dwIdent);
                        if(0 == isnan((*(TVIEWVALUE*)(ptViewValues+i)).tMeasValue.Value.fValue))
                        {
                            printf("   %0.3f   ",(*(TVIEWVALUE*)(ptViewValues+i)).tMeasValue.Value.fValue);
                        }
                        else
                        {
                            printf("   NAN"   );
                        }
                        printf("   %04X   \r\n",(*(TVIEWVALUE*)(ptViewValues+i)).tMeasValue.wUnit);
                    }
                    printf("=========================================================\r\n");
                }
                free(ptViewValues);
            }
        }
        sleep(1);
    }
	if (0 != TESTO350_SetMeasAction(dev_handle,MEAS_STOP))
		    printf("Can't set meas action\r\n");
        if (0 != TESTO350_GetMeasAction(dev_handle,&bMeasAction))
            printf("Can't get meas action\r\n");
	    TESTOEMUSB_Close(dev_handle);
    return 0;
}
#else
static libusb_device_handle *dev_handle = NULL; 
void printdev(libusb_device *dev)
{ 
    struct libusb_device_descriptor desc; 
    int r = libusb_get_device_descriptor(dev, &desc); 
    if(r <0){ printf("failed to get device descriptor\n"); 
    return; 
    } 
    printf("Number of possible configurations: %d\n,Device Class: %d\n",(int)desc.bNumConfigurations,(int)desc.bDeviceClass);
    printf("VendorID: %x\n",desc.idVendor); 
    printf("ProductID: %x\n",desc.idProduct); 
    struct libusb_config_descriptor *config; 
    libusb_get_config_descriptor(dev, 0, &config); 
    printf("Interfaces: %d\n",(int)config->bNumInterfaces); 
    const struct libusb_interface *inter; 
    const struct libusb_interface_descriptor *interdesc; 
    const struct libusb_endpoint_descriptor *epdesc; 
    for(int i=0; i<(int)config->bNumInterfaces; i++)
    { 
        inter =&config->interface[i]; 
        printf("Number of alternate settings: %d\n",inter->num_altsetting); 
        for(int j=0; j<inter->num_altsetting; j++)
        { 
            interdesc =&inter->altsetting[j]; 
            printf("Interface Number: %d\n",(int)interdesc->bInterfaceNumber); 
            printf("Number of endpoints: %d\n",(int)interdesc->bNumEndpoints); 
            for(int k=0; k<(int)interdesc->bNumEndpoints; k++)
            { 
                epdesc =&interdesc->endpoint[k]; 
                printf("Descriptor Type: %x\n",(int)epdesc->bDescriptorType); 
                printf("EP Address: %x\n",(int)epdesc->bEndpointAddress); 
            } 
        } 
    } 
    libusb_free_config_descriptor(config); 
} 
int main() 
{ 
    int i = 0; 
    int ret = 1; 
    int transferred = 0; 
    ssize_t cnt; 
    unsigned char data1[64] = {0x22, 0x01, 0x00, 0x00, 0x00, 0x40}; 
    unsigned char data2[64] = {0x40};
    struct libusb_device_descriptor desc; 
    libusb_device **devs; 
    libusb_context *ctx = NULL; 
    ret = libusb_init(NULL); 
    if(ret < 0) 
    { 
        fprintf(stderr, "failed to initialise libusb\n"); 
        return 1; 
    } 
    cnt = libusb_get_device_list(ctx, &devs); 
    if(cnt < 0) 
    { 
        perror("Get Device Error\n"); 
        return 1; 
    } 
    dev_handle = libusb_open_device_with_vid_pid(NULL, PRODUCT_ID_TESTO, VENDOR_ID_TESTO350_MB); 
    if(dev_handle == NULL)
    { 
        perror("Cannot open device\n"); 
    }
    else
    { 
        printf("Device Opened\n"); 
    } 
    /*printf("******************______************\n"); 
    for(i =0; i < cnt; i++)
    { 
        printdev(devs[i]); 
        printf("__________________******_____________\n"); 
    } */
    libusb_free_device_list(devs, 1); 
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) 
    { 
        printf("Kernel Driver Active\n"); 
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0)
        { 
            printf("Kernel Driver Detached!\n"); 
        } 
    } 
    ret = libusb_claim_interface(dev_handle, 0); 
    if(ret < 0) { 
        perror("Cannot Claim Interface\n"); 
        return 1; 
    } 
    /* 1. 发送读Image数据指令，使用0号通道??????????????????*/ 
    ret = libusb_bulk_transfer(dev_handle, 2, data1, 6, &transferred, DEFAULT_TIMEOUT); 
    if(ret==0 && transferred==6)
    { 
        //ret = libusb_bulk_transfer(dev_handle, 2, data2, 1, &transferred, USB_TIMEOUT); 
        //if(ret==0 && transferred==1)
        {
            printf("write Successful!\n"); 
            for(int n = 0; n < 2; n++)
            {
                unsigned char data_rec[64] = "\0";
                int length = 0;
                ret = libusb_bulk_transfer(dev_handle, 0x81, data_rec, 64, &length, DEFAULT_TIMEOUT);//device--------->host
                printf("ret=%d\r\n", ret);
                if (ret ==0)
                {
                    printf("--------------IN DATA---------------\r\n");
                    for (int i =0; i < length; i++)
                        printf("%02X", data_rec[i]);
                    printf("\r\n----------------------------\r\n");
                }
            }
        }
    }
    else
    { 
        printf("write error!\n"); 
    } 
    ret = libusb_release_interface(dev_handle, 0); 
    if(ret != 0)
    { 
        printf("Cannot Released Interface!\n"); 
    }
    else
    { 
        printf("Released Interface!\n"); 
    } 
    libusb_close(dev_handle); 
    libusb_exit(ctx); 
    return 0; 
}
#endif // 0
