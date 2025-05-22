/*
 * usb_can_bridge.c
 *
 *  Created on: May 23, 2025
 *      Author: nikhi
 */

#include "usb_can_bridge.h"

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN2) {
        CAN_RxHeaderTypeDef rxh;
        uint8_t rxdata[8];
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxh, rxdata) != HAL_OK) {
            return;
        }

        gs_host_frame f = {0};
        f.echo_id = 0;
        f.can_dlc = rxh.DLC;
        f.channel = 0;

        if (rxh.RTR == CAN_RTR_REMOTE)
            f.can_id |= 0x20000000U;

        if (rxh.IDE == CAN_ID_EXT) {
            f.can_id |= 0x80000000U | rxh.ExtId;
        } else {
            f.can_id |= rxh.StdId;
        }

        memcpy(f.data, rxdata, rxh.DLC);

        // Send this frame to the PC host via USB
        USBD_LL_Transmit(&hUsbDeviceFS, GSUSB_IN_EP, (uint8_t*)&f, 12 + f.can_dlc);
    }
}

