#include "hal/usb.h"

#include "hal/hal_err.h"
#include "stm32wbxx.h"
#include "usb/usb_device.h"

#define BTABLE_ADDRESS 0x000U

usb_ll_handle_t usb_ll_handle;

void hal_usb_enable_vdd() { SET_BIT(PWR->CR2, PWR_CR2_USV); }

void hal_usb_disable_interrupts() {
    uint32_t winterruptmask = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM |
                              USB_CNTR_ERRM | USB_CNTR_SOFM | USB_CNTR_ESOFM |
                              USB_CNTR_RESETM | USB_CNTR_L1REQM;
    USB->CNTR &= (uint16_t)(~winterruptmask);
    return;
}

void hal_usb_enable_interrupts() {
    uint32_t winterruptmask;
    /* Clear pending interrupts */
    USB->ISTR = 0U;
    /* Set winterruptmask variable */
    winterruptmask = USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM |
                     USB_CNTR_ERRM | USB_CNTR_SOFM | USB_CNTR_ESOFM |
                     USB_CNTR_RESETM | USB_CNTR_L1REQM;
    /* Set interrupt mask */
    USB->CNTR = (uint16_t)winterruptmask;
}

hal_err hal_usb_init(usb_ll_handle_t *handle) {

    if (!handle) {
        return ERR_USB_INIT_HANDLE_NULL;
    }

    if (!handle->instance) {
        return ERR_USB_INIT_INSTANCE_NULL;
    }

    uint8_t i;

    if (handle->state == USB_COMM_STATE_RESET) {
        /* Allocate lock resource and initialize it */
        handle->lock = USB_LOCK_UNLOCKED;
    }

    handle->state = USB_COMM_STATE_BUSY;

    /* Disable the Interrupts */
    hal_usb_disable_interrupts();

    /* Init endpoints structures */
    for (i = 0U; i < handle->init.dev_endpoints; i++) {
        /* Init ep structure */
        handle->in_ep[i].is_in = 1U;
        handle->in_ep[i].num = i;
        /* Control until ep is activated */
        handle->in_ep[i].type = 0U;
        handle->in_ep[i].maxpacket = 0U;
        handle->in_ep[i].xfer_buff = 0U;
        handle->in_ep[i].xfer_len = 0U;
    }

    for (i = 0U; i < handle->init.dev_endpoints; i++) {
        handle->out_ep[i].is_in = 0U;
        handle->out_ep[i].num = i;
        /* Control until ep is activated */
        handle->out_ep[i].type = 0U;
        handle->out_ep[i].maxpacket = 0U;
        handle->out_ep[i].xfer_buff = 0U;
        handle->out_ep[i].xfer_len = 0U;
    }

    /* Init Device */
    WRITE_REG(USB->CNTR, USB_CNTR_FRES);

    /* CNTR_FRES = 0 */
    WRITE_REG(USB->CNTR, 0U);

    /* Clear pending interrupts */
    WRITE_REG(USB->ISTR, 0U);

    /*Set Btable Address*/
    WRITE_REG(USB->BTABLE, BTABLE_ADDRESS);

    handle->usb_address = 0U;
    handle->state = USB_COMM_STATE_READY;

    /* Activate LPM */
    if (handle->init.lpm_enable == 1U) {
        // TODO
    }

    return OK;
}

void hal_usb_pma_config(usb_ll_handle_t *handle, uint16_t ep_addr,
                        uint16_t ep_kind, uint32_t pmaadress) {
    usb_endpoint_t *ep;

    /* initialize ep structure*/
    if ((0x80U & ep_addr) == 0x80U) {
        ep = &handle->in_ep[ep_addr & EP_ADDR_MSK];
    } else {
        ep = &handle->out_ep[ep_addr];
    }

    /* Here we check if the endpoint is single or double Buffer*/
    if (ep_kind == PCD_SNG_BUF) {
        /* Single Buffer */
        ep->doublebuffer = 0U;
        /* Configure the PMA */
        ep->pmaadress = (uint16_t)pmaadress;
    } else /* USB_DBL_BUF */
    {
        /* Double Buffer Endpoint */
        ep->doublebuffer = 1U;
        /* Configure the PMA */
        ep->pmaaddr0 = (uint16_t)(pmaadress & 0xFFFFU);
        ep->pmaaddr1 = (uint16_t)((pmaadress & 0xFFFF0000U) >> 16);
    }

    return;
}

hal_err hal_usb_activate_endpoint(usb_endpoint_t *ep) {
    uint16_t wEpRegVal;

    wEpRegVal = PCD_GET_ENDPOINT(USB, ep->num) & USB_EP_T_MASK;

    /* initialize Endpoint */
    switch (ep->type) {
    case EP_TYPE_CTRL:
        wEpRegVal |= USB_EP_CONTROL;
        break;

    case EP_TYPE_BULK:
        wEpRegVal |= USB_EP_BULK;
        break;

    case EP_TYPE_INTR:
        wEpRegVal |= USB_EP_INTERRUPT;
        break;

    case EP_TYPE_ISOC:
        wEpRegVal |= USB_EP_ISOCHRONOUS;
        break;

    default:
        return ERR_USBD_ACT_END_INV_EPTYPE;
    }

    PCD_SET_ENDPOINT(USB, ep->num, (wEpRegVal | USB_EP_CTR_RX | USB_EP_CTR_TX));

    PCD_SET_EP_ADDRESS(USB, ep->num, ep->num);

    if (ep->doublebuffer == 0U) {
        if (ep->is_in != 0U) {
            /*Set the endpoint Transmit buffer address */
            PCD_SET_EP_TX_ADDRESS(USB, ep->num, ep->pmaadress);
            PCD_CLEAR_TX_DTOG(USB, ep->num);

            if (ep->type != EP_TYPE_ISOC) {
                /* Configure NAK status for the Endpoint */
                PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_NAK);
            } else {
                /* Configure TX Endpoint to disabled state */
                PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_DIS);
            }
        } else {
            /* Set the endpoint Receive buffer address */
            PCD_SET_EP_RX_ADDRESS(USB, ep->num, ep->pmaadress);

            /* Set the endpoint Receive buffer counter */
            PCD_SET_EP_RX_CNT(USB, ep->num, ep->maxpacket);
            PCD_CLEAR_RX_DTOG(USB, ep->num);

            if (ep->num == 0U) {
                /* Configure VALID status for EP0 */
                PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_VALID);
            } else {
                /* Configure NAK status for OUT Endpoint */
                PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_NAK);
            }
        }
    }
    /* Double Buffer */
    else {
        if (ep->type == EP_TYPE_BULK) {
            /* Set bulk endpoint as double buffered */
            PCD_SET_BULK_EP_DBUF(USB, ep->num);
        } else {
            /* Set the ISOC endpoint in double buffer mode */
            PCD_CLEAR_EP_KIND(USB, ep->num);
        }

        /* Set buffer address for double buffered mode */
        PCD_SET_EP_DBUF_ADDR(USB, ep->num, ep->pmaaddr0, ep->pmaaddr1);

        if (ep->is_in == 0U) {
            /* Clear the data toggle bits for the endpoint IN/OUT */
            PCD_CLEAR_RX_DTOG(USB, ep->num);
            PCD_CLEAR_TX_DTOG(USB, ep->num);

            /* Set endpoint RX count */
            PCD_SET_EP_DBUF_CNT(USB, ep->num, ep->is_in, ep->maxpacket);

            /* Set endpoint RX to valid state */
            PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_VALID);
            PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_DIS);
        } else {
            /* Clear the data toggle bits for the endpoint IN/OUT */
            PCD_CLEAR_RX_DTOG(USB, ep->num);
            PCD_CLEAR_TX_DTOG(USB, ep->num);

            if (ep->type != EP_TYPE_ISOC) {
                /* Configure NAK status for the Endpoint */
                PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_NAK);
            } else {
                /* Configure TX Endpoint to disabled state */
                PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_DIS);
            }

            PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_DIS);
        }
    }

    return OK;
}

void hal_usb_deactivate_endpoint(usb_endpoint_t *ep) {
    if (ep->doublebuffer == 0U) {
        if (ep->is_in != 0U) {
            PCD_CLEAR_TX_DTOG(USB, ep->num);

            /* Configure DISABLE status for the Endpoint */
            PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_DIS);
        }

        else {
            PCD_CLEAR_RX_DTOG(USB, ep->num);

            /* Configure DISABLE status for the Endpoint */
            PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_DIS);
        }
    }
    /* Double Buffer */
    else {
        if (ep->is_in == 0U) {
            /* Clear the data toggle bits for the endpoint IN/OUT*/
            PCD_CLEAR_RX_DTOG(USB, ep->num);
            PCD_CLEAR_TX_DTOG(USB, ep->num);

            /* Reset value of the data toggle bits for the endpoint out*/
            PCD_TX_DTOG(USB, ep->num);

            PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_DIS);
            PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_DIS);
        } else {
            /* Clear the data toggle bits for the endpoint IN/OUT*/
            PCD_CLEAR_RX_DTOG(USB, ep->num);
            PCD_CLEAR_TX_DTOG(USB, ep->num);
            PCD_RX_DTOG(USB, ep->num);

            /* Configure DISABLE status for the Endpoint*/
            PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_DIS);
            PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_DIS);
        }
    }
}

void hal_usb_write_pma(uint8_t *pbUsrBuf, uint16_t wPMABufAddr,
                       uint16_t wNBytes) {
    uint32_t n = ((uint32_t)wNBytes + 1U) >> 1;
    uint32_t BaseAddr = (uint32_t)USB;
    uint32_t count;
    uint16_t WrVal;
    __IO uint16_t *pdwVal;
    uint8_t *pBuf = pbUsrBuf;

    pdwVal =
        (__IO uint16_t *)(BaseAddr + 0x400U + ((uint32_t)wPMABufAddr * 1U));

    for (count = n; count != 0U; count--) {
        WrVal = pBuf[0];
        WrVal |= (uint16_t)pBuf[1] << 8;
        *pdwVal = (WrVal & 0xFFFFU);
        pdwVal++;

        pBuf++;
        pBuf++;
    }
}

void hal_usb_stall_endpoint(usb_ll_handle_t *handle, uint8_t ep_addr) {

    usb_endpoint_t *ep;

    if (((uint32_t)ep_addr & EP_ADDR_MSK) > handle->init.dev_endpoints) {
        return;
    }

    if ((0x80U & ep_addr) == 0x80U) {
        ep = &handle->in_ep[ep_addr & EP_ADDR_MSK];
        ep->is_in = 1U;
    } else {
        ep = &handle->out_ep[ep_addr];
        ep->is_in = 0U;
    }

    ep->is_stall = 1U;
    ep->num = ep_addr & EP_ADDR_MSK;

    // __HAL_LOCK(handle); TODO

    if (ep->is_in != 0U) {
        PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_STALL);
    } else {
        PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_STALL);
    }

    // __HAL_UNLOCK(handle); TODO
}

void hal_usb_device_start_ep_xfer(usb_endpoint_t *ep) {
    uint32_t len;
    uint16_t pmabuffer;
    uint16_t wEPVal;

    /* IN endpoint */
    if (ep->is_in == 1U) {
        /* Multi packet transfer */
        if (ep->xfer_len > ep->maxpacket) {
            len = ep->maxpacket;
        } else {
            len = ep->xfer_len;
        }

        /* configure and validate Tx endpoint */
        if (ep->doublebuffer == 0U) {
            hal_usb_write_pma(ep->xfer_buff, ep->pmaadress, (uint16_t)len);
            PCD_SET_EP_TX_CNT(USB, ep->num, len);
        } else {
            /* double buffer bulk management */
            if (ep->type == EP_TYPE_BULK) {
                if (ep->xfer_len_db > ep->maxpacket) {
                    /* enable double buffer */
                    PCD_SET_BULK_EP_DBUF(USB, ep->num);

                    /* each Time to write in PMA xfer_len_db will */
                    ep->xfer_len_db -= len;

                    /* Fill the two first buffer in the Buffer0 & Buffer1 */
                    if ((PCD_GET_ENDPOINT(USB, ep->num) & USB_EP_DTOG_TX) !=
                        0U) {
                        /* Set the Double buffer counter for pmabuffer1 */
                        PCD_SET_EP_DBUF1_CNT(USB, ep->num, ep->is_in, len);
                        pmabuffer = ep->pmaaddr1;

                        /* Write the user buffer to USB PMA */
                        hal_usb_write_pma(ep->xfer_buff, pmabuffer,
                                          (uint16_t)len);
                        ep->xfer_buff += len;

                        if (ep->xfer_len_db > ep->maxpacket) {
                            ep->xfer_len_db -= len;
                        } else {
                            len = ep->xfer_len_db;
                            ep->xfer_len_db = 0U;
                        }

                        /* Set the Double buffer counter for pmabuffer0 */
                        PCD_SET_EP_DBUF0_CNT(USB, ep->num, ep->is_in, len);
                        pmabuffer = ep->pmaaddr0;

                        /* Write the user buffer to USB PMA */
                        hal_usb_write_pma(ep->xfer_buff, pmabuffer,
                                          (uint16_t)len);
                    } else {
                        /* Set the Double buffer counter for pmabuffer0 */
                        PCD_SET_EP_DBUF0_CNT(USB, ep->num, ep->is_in, len);
                        pmabuffer = ep->pmaaddr0;

                        /* Write the user buffer to USB PMA */
                        hal_usb_write_pma(ep->xfer_buff, pmabuffer,
                                          (uint16_t)len);
                        ep->xfer_buff += len;

                        if (ep->xfer_len_db > ep->maxpacket) {
                            ep->xfer_len_db -= len;
                        } else {
                            len = ep->xfer_len_db;
                            ep->xfer_len_db = 0U;
                        }

                        /* Set the Double buffer counter for pmabuffer1 */
                        PCD_SET_EP_DBUF1_CNT(USB, ep->num, ep->is_in, len);
                        pmabuffer = ep->pmaaddr1;

                        /* Write the user buffer to USB PMA */
                        hal_usb_write_pma(ep->xfer_buff, pmabuffer,
                                          (uint16_t)len);
                    }
                }
                /* auto Switch to single buffer mode when transfer <Mps no need
                   to manage in double buffer */
                else {
                    len = ep->xfer_len_db;

                    /* disable double buffer mode for Bulk endpoint */
                    PCD_CLEAR_BULK_EP_DBUF(USB, ep->num);

                    /* Set Tx count with nbre of byte to be transmitted */
                    PCD_SET_EP_TX_CNT(USB, ep->num, len);
                    pmabuffer = ep->pmaaddr0;

                    /* Write the user buffer to USB PMA */
                    hal_usb_write_pma(ep->xfer_buff, pmabuffer, (uint16_t)len);
                }
            } else /* Manage isochronous double buffer IN mode */
            {
                /* Each Time to write in PMA xfer_len_db will */
                ep->xfer_len_db -= len;

                /* Fill the data buffer */
                if ((PCD_GET_ENDPOINT(USB, ep->num) & USB_EP_DTOG_TX) != 0U) {
                    /* Set the Double buffer counter for pmabuffer1 */
                    PCD_SET_EP_DBUF1_CNT(USB, ep->num, ep->is_in, len);
                    pmabuffer = ep->pmaaddr1;

                    /* Write the user buffer to USB PMA */
                    hal_usb_write_pma(ep->xfer_buff, pmabuffer, (uint16_t)len);
                } else {
                    /* Set the Double buffer counter for pmabuffer0 */
                    PCD_SET_EP_DBUF0_CNT(USB, ep->num, ep->is_in, len);
                    pmabuffer = ep->pmaaddr0;

                    /* Write the user buffer to USB PMA */
                    hal_usb_write_pma(ep->xfer_buff, pmabuffer, (uint16_t)len);
                }
            }
        }

        PCD_SET_EP_TX_STATUS(USB, ep->num, USB_EP_TX_VALID);
    } else /* OUT endpoint */
    {
        if (ep->doublebuffer == 0U) {
            if ((ep->xfer_len == 0U) && (ep->type == EP_TYPE_CTRL)) {
                /* This is a status out stage set the OUT_STATUS */
                PCD_SET_OUT_STATUS(USB, ep->num);
            } else {
                PCD_CLEAR_OUT_STATUS(USB, ep->num);
            }

            /* Multi packet transfer */
            if (ep->xfer_len > ep->maxpacket) {
                ep->xfer_len -= ep->maxpacket;
            } else {
                ep->xfer_len = 0U;
            }
        } else {
            /* First Transfer Coming From HAL_PCD_EP_Receive & From ISR */
            /* Set the Double buffer counter */
            if (ep->type == EP_TYPE_BULK) {
                /* Coming from ISR */
                if (ep->xfer_count != 0U) {
                    /* Update last value to check if there is blocking state */
                    wEPVal = PCD_GET_ENDPOINT(USB, ep->num);

                    /* Blocking State */
                    if ((((wEPVal & USB_EP_DTOG_RX) != 0U) &&
                         ((wEPVal & USB_EP_DTOG_TX) != 0U)) ||
                        (((wEPVal & USB_EP_DTOG_RX) == 0U) &&
                         ((wEPVal & USB_EP_DTOG_TX) == 0U))) {
                        PCD_FREE_USER_BUFFER(USB, ep->num, 0U);
                    }
                }
            }
            /* iso out double */
            else if (ep->type == EP_TYPE_ISOC) {
                /* Only single packet transfer supported in FS */
                ep->xfer_len = 0U;
            } else {
                return;
            }
        }

        PCD_SET_EP_RX_STATUS(USB, ep->num, USB_EP_RX_VALID);
    }
}

void hal_usb_start() {

    // __HAL_LOCK(hpcd);

    hal_usb_enable_interrupts();

    USB->BCDR |= (uint16_t)USB_BCDR_DPPU;

    // __HAL_UNLOCK(hpcd);
}

void hal_usb_deactivate_remote_wakeup() {
    CLEAR_BIT(USB->CNTR, USB_CNTR_RESUME);
}

void hal_usb_activate_remote_wakeup() { SET_BIT(USB->CNTR, USB_CNTR_RESUME); }
