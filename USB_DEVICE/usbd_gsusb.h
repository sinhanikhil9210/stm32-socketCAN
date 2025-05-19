/* usbd_gsusb.h */
#ifndef __USBD_GSUSB_H
#define __USBD_GSUSB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"
#include <stdint.h>

/* Bulk endpoint definitions */
#define GSUSB_IN_EP             0x81U   /* EP1 IN  */
#define GSUSB_OUT_EP            0x02U   /* EP2 OUT */
#define GSUSB_MAX_PACKET_SIZE   64U     /* FS bulk max packet */

//#define GS_USB_BREQ_HOST_FORMAT     0x00  /* host → device:  u32 byte_order */
//#define GS_USB_BREQ_DEVICE_CONFIG   0x05  /* device → host:  struct gs_device_config */

/* Vendor-specific USB requests (bRequest values) */
#define GS_USB_BREQ_HOST_FORMAT     0x00
#define GS_USB_BREQ_BITTIMING       0x01
#define GS_USB_BREQ_MODE            0x02
#define GS_USB_BREQ_BERR            0x03
#define GS_USB_BREQ_BT_CONST        0x04
#define GS_USB_BREQ_DEVICE_CONFIG   0x05
#define GS_USB_BREQ_IDENTIFY        0x07
#define GS_USB_BREQ_USER_ID_SET     0x08
#define GS_USB_BREQ_USER_ID_GET     0x0A
#define GS_USB_BREQ_DATA_BITTIMING  0x0E
#define GS_USB_BREQ_GET_STATE       0x0F

/* CAN feature flags */
#define GS_CAN_FEATURE_LISTEN_ONLY  (1U<<0)
#define GS_CAN_FEATURE_LOOP_BACK    (1U<<1)

/* CAN mode values */
#define GS_CAN_MODE_RESET           0x00000000U
#define GS_CAN_MODE_START           0x00000001U

/* Packed structs for control-transfer payloads */
//#pragma pack(push,1)
typedef struct __attribute__((packed)){
    uint32_t icount;    /* number of interfaces (channels) - 1 */
    uint32_t sw_version;
    uint32_t hw_version;
//    uint32_t reserved[1];
} gs_device_config;



typedef struct __attribute__((packed)){
    uint32_t feature;
    uint32_t fclk_can;
    uint32_t tseg1_min;
    uint32_t tseg1_max;
    uint32_t tseg2_min;
    uint32_t tseg2_max;
    uint32_t sjw_max;
    uint32_t brp_min;
    uint32_t brp_max;
    uint32_t brp_inc;
} gs_device_bt_const;

typedef struct __attribute__((packed)){
    uint32_t prop_seg;
    uint32_t phase_seg1;
    uint32_t phase_seg2;
    uint32_t sjw;
    uint32_t brp;
} gs_device_bittiming;

typedef struct __attribute__((packed)){
    uint32_t mode;
    uint32_t flags;
} gs_device_mode;

/* Host <-> device CAN frame (max 64 data bytes) */
#define GSUSB_MAX_DATA_LEN 64
typedef struct __attribute__((packed)){
    uint32_t echo_id;
    uint32_t can_id;
    uint8_t  can_dlc;
    uint8_t  channel;
    uint8_t  flags;
    uint8_t  reserved;
    uint8_t  data[GSUSB_MAX_DATA_LEN];
} gs_host_frame;
//#pragma pack(pop)

/* GS_USB class state */
typedef struct __attribute__((packed)){
	uint32_t			host_byte_order;
    gs_device_bittiming pending_bittiming;
    gs_device_mode      pending_mode;
    uint8_t             identify_state;
    uint8_t             rx_buffer[GSUSB_MAX_PACKET_SIZE];
    /* you can add a small TX queue here if bursty RX is expected */
} USBD_GSUSB_HandleTypeDef;

/* Exported interface */
extern USBD_ClassTypeDef USBD_GSUSB;
#define USBD_GSUSB_CLASS  &USBD_GSUSB

/* Registration (no fops needed since CAN logic lives in this class) */
uint8_t USBD_GSUSB_RegisterInterface(USBD_HandleTypeDef *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_GSUSB_H */
