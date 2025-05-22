/* usbd_gsusb.c */
#include "usbd_gsusb.h"
#include "usbd_core.h"
#include "usbd_ctlreq.h"
//#include "can.h"               /* Your HAL CAN header */
#include <string.h>

//extern CAN_HandleTypeDef hcan;  /* Make sure your CAN handle is visible here */




/* Size of our single-interface configuration descriptor */
#define USBD_GSUSB_CONFIG_DESC_SIZ  32U

static const gs_device_config device_config = {
  .icount     = 0,    /* 0 = one CAN channel */
  .sw_version = 1,    /* arbitrary firmware version */
  .hw_version = 1,    /* arbitrary hardware version */
};


//#define GS_USB_BREQ_HOST_FORMAT     0x00  /* host → device:  u32 byte_order */
//#define GS_USB_BREQ_DEVICE_CONFIG   0x05  /* device → host:  struct gs_device_config */
/* Device Qualifier Descriptor (for FS-only devices you can keep this) */
__ALIGN_BEGIN static const uint8_t USBD_GSUSB_QualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00, 0x02,             /* USB 2.00 */
  0x00, 0x00, 0x00,       /* per-interface, no subclass/protocol */
  GSUSB_MAX_PACKET_SIZE,
  0x01,                   /* one configuration */
  0x00
};

/* Configuration Descriptor (1 interface, 2 bulk EPs) */
__ALIGN_BEGIN static const uint8_t USBD_GSUSB_CfgFSDesc[USBD_GSUSB_CONFIG_DESC_SIZ] __ALIGN_END = {
  /* Configuration Descriptor */
  0x09, USB_DESC_TYPE_CONFIGURATION,
  LOBYTE(USBD_GSUSB_CONFIG_DESC_SIZ), HIBYTE(USBD_GSUSB_CONFIG_DESC_SIZ),
  0x01,        /* bNumInterfaces */
  0x01,        /* bConfigurationValue */
  0x00,        /* iConfiguration */
  0x80,        /* bmAttributes: bus-powered */
  0x32,        /* bMaxPower = 100mA */

  /* Interface Descriptor */
  0x09, USB_DESC_TYPE_INTERFACE,
  0x00,        /* bInterfaceNumber */
  0x00,        /* bAlternateSetting */
  0x02,        /* bNumEndpoints */
  0xFF,        /* bInterfaceClass = vendor */
  0xFF,        /* bInterfaceSubClass */
  0xFF,        /* bInterfaceProtocol */
  0x00,        /* iInterface */

  /* Endpoint IN Descriptor */
  0x07, USB_DESC_TYPE_ENDPOINT,
  GSUSB_IN_EP,        /* bEndpointAddress = EP1 IN */
  0x02,               /* bmAttributes = Bulk */
  LOBYTE(GSUSB_MAX_PACKET_SIZE), HIBYTE(GSUSB_MAX_PACKET_SIZE),
  0x00,               /* bInterval */

  /* Endpoint OUT Descriptor */
  0x07, USB_DESC_TYPE_ENDPOINT,
  GSUSB_OUT_EP,       /* bEndpointAddress = EP2 OUT */
  0x02,               /* bmAttributes = Bulk */
  LOBYTE(GSUSB_MAX_PACKET_SIZE), HIBYTE(GSUSB_MAX_PACKET_SIZE),
  0x00                /* bInterval */
};

/* Forward declarations */
static uint8_t  USBD_GSUSB_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_GSUSB_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_GSUSB_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  USBD_GSUSB_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t  USBD_GSUSB_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_GSUSB_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_GSUSB_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_GSUSB_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_GSUSB_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_GSUSB_GetDeviceQualifierDesc(uint16_t *length);
#endif

/* Class driver structure */
USBD_ClassTypeDef USBD_GSUSB = {
  USBD_GSUSB_Init,
  USBD_GSUSB_DeInit,
  USBD_GSUSB_Setup,
  NULL,                  /* EP0_TxSent */
  USBD_GSUSB_EP0_RxReady,
  USBD_GSUSB_DataIn,
  USBD_GSUSB_DataOut,
  NULL, NULL, NULL,
#ifndef USE_USBD_COMPOSITE
  USBD_GSUSB_GetHSCfgDesc,
  USBD_GSUSB_GetFSCfgDesc,
  USBD_GSUSB_GetOtherSpeedCfgDesc,
  USBD_GSUSB_GetDeviceQualifierDesc,
#endif
};

/* One instance of our class state */
static USBD_GSUSB_HandleTypeDef gsusb_handle;

/* Register interface (no callbacks needed beyond CAN HAL) */
uint8_t USBD_GSUSB_RegisterInterface(USBD_HandleTypeDef *pdev) {
  /* nothing additional to register */
  return USBD_OK;
}

/* Initialize: open bulk endpoints & prepare first RX */
static uint8_t USBD_GSUSB_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
  (void)cfgidx;
  USBD_LL_OpenEP(pdev, GSUSB_IN_EP,  USBD_EP_TYPE_BULK, GSUSB_MAX_PACKET_SIZE);
  USBD_LL_OpenEP(pdev, GSUSB_OUT_EP, USBD_EP_TYPE_BULK, GSUSB_MAX_PACKET_SIZE);
  memset(&gsusb_handle, 0, sizeof(gsusb_handle));
  pdev->pClassData = &gsusb_handle;
  USBD_LL_PrepareReceive(pdev, GSUSB_OUT_EP,
                         gsusb_handle.rx_buffer, GSUSB_MAX_PACKET_SIZE);
  return USBD_OK;
}

/* DeInit: close endpoints */
static uint8_t USBD_GSUSB_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx) {
  (void)cfgidx;
  USBD_LL_CloseEP(pdev, GSUSB_IN_EP);
  USBD_LL_CloseEP(pdev, GSUSB_OUT_EP);
  pdev->pClassData = NULL;
  return USBD_OK;
}

/* Handle standard & vendor requests */
static uint8_t USBD_GSUSB_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req) {
  USBD_GSUSB_HandleTypeDef *h = &gsusb_handle;

  /* Vendor-specific requests */
  if ((req->bmRequest & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_VENDOR) {
    switch (req->bRequest) {
      case GS_USB_BREQ_HOST_FORMAT:
        /* Host tells us its byte-order (we just swallow it) */
        USBD_CtlPrepareRx(pdev,
                          (uint8_t*)&h->host_byte_order,
                          sizeof(h->host_byte_order));
        break;

      case GS_USB_BREQ_DEVICE_CONFIG:
        /* Tell host about our one CAN channel + versions */

    	  USBD_CtlSendData(pdev, (uint8_t*)&device_config, sizeof(device_config));
        break;

      case GS_USB_BREQ_BT_CONST: {
        /* (same as before) */
        gs_device_bt_const btc = {
          .feature   = GS_CAN_FEATURE_LISTEN_ONLY
                     | GS_CAN_FEATURE_LOOP_BACK,
          .fclk_can  = 45000000,
          .tseg1_min = 1,  .tseg1_max = 16,
          .tseg2_min = 1,  .tseg2_max = 8,
          .sjw_max   = 4,
          .brp_min   = 1,  .brp_max   = 1024, .brp_inc = 1
        };
        USBD_CtlSendData(pdev,
                         (uint8_t*)&btc,
                         sizeof(btc));
        break;
      }

      case GS_USB_BREQ_BITTIMING:
        USBD_CtlPrepareRx(pdev,
             (uint8_t*)&h->pending_bittiming,
             sizeof(gs_device_bittiming));
        break;

      case GS_USB_BREQ_MODE:
        USBD_CtlPrepareRx(pdev,
             (uint8_t*)&h->pending_mode,
             sizeof(gs_device_mode));
        break;

      case GS_USB_BREQ_IDENTIFY:
        USBD_CtlPrepareRx(pdev,
             (uint8_t*)&h->identify_state,
             sizeof(h->identify_state));
        break;

      default:
        USBD_CtlError(pdev, req);
        return USBD_FAIL;
    }
    return USBD_OK;
  }

  /* Standard requests are handled by the USB core (GetDescriptor etc.) */
  return USBD_OK;
}

/* After EP0 data-out completes, apply CAN config/mode/identify */
static uint8_t USBD_GSUSB_EP0_RxReady(USBD_HandleTypeDef *pdev) {
	  USBD_GSUSB_HandleTypeDef *h = &gsusb_handle;

	  /* Handle host byte-order handshake (GS_USB_BREQ_HOST_FORMAT) */
	  if (h->host_byte_order == 0x0000BEEF) {
	    // Linux host sent expected handshake
	    h->host_byte_order = 0;  // Clear it (optional)
	  }

	  /* Apply bit timing if provided */
	  if (memcmp(&h->pending_bittiming, &(gs_device_bittiming){0}, sizeof(gs_device_bittiming)) != 0) {
	    // TODO: apply bit timing
	  }

	  if (h->pending_mode.mode == GS_CAN_MODE_START) {
		  // Apply loopback or listen-only if requested
		  if (h->pending_mode.flags & GS_CAN_FEATURE_LOOP_BACK) {
		      hcan2.Init.Mode = CAN_MODE_LOOPBACK;
		  } else if (h->pending_mode.flags & GS_CAN_FEATURE_LISTEN_ONLY) {
		      hcan2.Init.Mode = CAN_MODE_SILENT;
		  } else {
		      hcan2.Init.Mode = CAN_MODE_NORMAL;
		  }
	    HAL_CAN_Start(&hcan2);
	    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);
	  } else {
	    HAL_CAN_Stop(&hcan2);
	  }

	  return USBD_OK;
}

/* Bulk IN complete: you could flush a TX queue here */
static uint8_t USBD_GSUSB_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum) {
  (void)pdev; (void)epnum;
  /* Nothing special: next TX will be sent from DataOut echo or CAN RX handler */
  return USBD_OK;
}

/* Bulk OUT: host → CAN bus */
static uint8_t USBD_GSUSB_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum) {
  USBD_GSUSB_HandleTypeDef *h = &gsusb_handle;
  gs_host_frame *f = (gs_host_frame*)h->rx_buffer;

  /* Parse CAN ID & flags */
  CAN_TxHeaderTypeDef txh;
  uint8_t txdata[8];
  if (f->can_id & 0x80000000U) {
    txh.IDE   = CAN_ID_EXT;
    txh.ExtId = f->can_id & 0x1FFFFFFFU;
  } else {
    txh.IDE   = CAN_ID_STD;
    txh.StdId = f->can_id & 0x7FFU;
  }
  txh.RTR = (f->can_id & 0x20000000U) ? CAN_RTR_REMOTE : CAN_RTR_DATA;
  txh.DLC = f->can_dlc;
  txh.TransmitGlobalTime = DISABLE;
  memcpy(txdata, f->data, txh.DLC);

  /* Send on CAN bus */
  HAL_CAN_AddTxMessage(&hcan2, &txh, txdata, &(uint32_t){0});

  /* Echo back to host (use same buffer) */
  USBD_LL_Transmit(pdev, GSUSB_IN_EP, h->rx_buffer, (uint16_t)(12 + f->can_dlc));

  /* Re-arm for next OUT packet */
  USBD_LL_PrepareReceive(pdev, GSUSB_OUT_EP,
                         h->rx_buffer, GSUSB_MAX_PACKET_SIZE);
  return USBD_OK;
}

#ifndef USE_USBD_COMPOSITE
/* Return FS config descriptor */
static uint8_t *USBD_GSUSB_GetFSCfgDesc(uint16_t *length) {
  *length = USBD_GSUSB_CONFIG_DESC_SIZ;
  return (uint8_t*)USBD_GSUSB_CfgFSDesc;
}

/* For HS, mirror FS descriptor or provide a true HS descriptor */
static uint8_t *USBD_GSUSB_GetHSCfgDesc(uint16_t *length) {
  *length = USBD_GSUSB_CONFIG_DESC_SIZ;
  return (uint8_t*)USBD_GSUSB_CfgFSDesc;
}

/* Other-speed: same as FS */
static uint8_t *USBD_GSUSB_GetOtherSpeedCfgDesc(uint16_t *length) {
  *length = USBD_GSUSB_CONFIG_DESC_SIZ;
  return (uint8_t*)USBD_GSUSB_CfgFSDesc;
}

/* Device qualifier for HS clients */
static uint8_t *USBD_GSUSB_GetDeviceQualifierDesc(uint16_t *length) {
  *length = USB_LEN_DEV_QUALIFIER_DESC;
  return (uint8_t*)USBD_GSUSB_QualifierDesc;
}
#endif
