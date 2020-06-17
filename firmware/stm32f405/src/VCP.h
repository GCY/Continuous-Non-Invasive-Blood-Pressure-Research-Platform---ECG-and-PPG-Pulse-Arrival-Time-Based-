#ifndef __VCP__
#define __VCP__

#include "./usb_cdc_device/usbd_usr.h"
#include "./usb_cdc_device/usbd_cdc_core.h"
#include "./usb_cdc_device/usb_conf.h"
#include "./usb_cdc_device/usbd_desc.h"
#include "./usb_cdc_device/usbd_cdc_vcp.h"

/* Private */
#define USB_VCP_RECEIVE_BUFFER_LENGTH		128
extern uint8_t INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];
extern uint32_t int_usb_vcp_buf_in, int_usb_vcp_buf_out, int_usb_vcp_buf_num;
extern USB_VCP_Result USB_VCP_INT_Status;

extern uint8_t USB_VCP_INT_Init;
extern USB_OTG_CORE_HANDLE	USB_OTG_dev;

extern uint8_t INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];


extern USB_VCP_Result USBVCPInit();
extern USB_VCP_Result USB_VCP_GetStatus();
extern USB_VCP_Result USB_VCP_Getc(uint8_t*);
extern USB_VCP_Result USB_VCP_Putc(volatile char);
extern USB_VCP_Result USB_VCP_Puts(char*);
extern USB_VCP_Result INT_USB_VCP_AddReceived(uint8_t);

#endif
