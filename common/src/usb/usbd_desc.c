#include "usb/usbd_desc.h"

#include "usb/usbd_conf.h"
#include "usb/usbd_core.h"

#include "utils/utils.h"

#define USBD_VID 0x6969
#define USBD_LANGID_STRING 1033
#define USBD_MANUFACTURER_STRING "Yarman Production"
#define USBD_PRODUCT_STRING "Yarman Keyboard"
#define USBD_CONFIGURATION_STRING "HID Config"
#define USBD_INTERFACE_STRING "HID Interface"

static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);

uint8_t *USBD_HID_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HID_LangIDStrDescriptor(USBD_SpeedTypeDef speed,
                                      uint16_t *length);
uint8_t *USBD_HID_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed,
                                            uint16_t *length);
uint8_t *USBD_HID_ProductStrDescriptor(USBD_SpeedTypeDef speed,
                                       uint16_t *length);
uint8_t *USBD_HID_SerialStrDescriptor(USBD_SpeedTypeDef speed,
                                      uint16_t *length);
uint8_t *USBD_HID_ConfigStrDescriptor(USBD_SpeedTypeDef speed,
                                      uint16_t *length);
uint8_t *USBD_HID_InterfaceStrDescriptor(USBD_SpeedTypeDef speed,
                                         uint16_t *length);

USBD_DescriptorsTypeDef HID_Desc = {
    USBD_HID_DeviceDescriptor,          USBD_HID_LangIDStrDescriptor,
    USBD_HID_ManufacturerStrDescriptor, USBD_HID_ProductStrDescriptor,
    USBD_HID_SerialStrDescriptor,       USBD_HID_ConfigStrDescriptor,
    USBD_HID_InterfaceStrDescriptor};

__ALIGN_BEGIN uint8_t USBD_HID_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
    0x12,                 /*bLength */
    USB_DESC_TYPE_DEVICE, /*bDescriptorType*/
    0x00,                 /*bcdUSB */
    0x02,
    0x00,             /*bDeviceClass*/
    0x00,             /*bDeviceSubClass*/
    0x00,             /*bDeviceProtocol*/
    USB_MAX_EP0_SIZE, /*bMaxPacketSize*/
    LOBYTE(USBD_VID), /*idVendor*/
    HIBYTE(USBD_VID), /*idVendor*/
    LOBYTE(USBD_PID), /*idProduct*/
    HIBYTE(USBD_PID), /*idProduct*/
    0x00,             /*bcdDevice rel. 2.00*/
    0x02,
    USBD_IDX_MFC_STR,          /*Index of manufacturer  string*/
    USBD_IDX_PRODUCT_STR,      /*Index of product string*/
    USBD_IDX_SERIAL_STR,       /*Index of serial number string*/
    USBD_MAX_NUM_CONFIGURATION /*bNumConfigurations*/
};

/** USB lang identifier descriptor. */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
    USB_LEN_LANGID_STR_DESC, USB_DESC_TYPE_STRING, LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING)};

/* Internal string descriptor. */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
    USB_SIZ_STRING_SERIAL,
    USB_DESC_TYPE_STRING,
};

uint8_t *USBD_HID_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
    UNUSED(speed);
    *length = sizeof(USBD_HID_DeviceDesc);
    return USBD_HID_DeviceDesc;
}

uint8_t *USBD_HID_LangIDStrDescriptor(USBD_SpeedTypeDef speed,
                                      uint16_t *length) {
    UNUSED(speed);
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

uint8_t *USBD_HID_ProductStrDescriptor(USBD_SpeedTypeDef speed,
                                       uint16_t *length) {
    if (speed == 0) {
        USBD_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_StrDesc, length);
    } else {
        USBD_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_StrDesc, length);
    }
    return USBD_StrDesc;
}

uint8_t *USBD_HID_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed,
                                            uint16_t *length) {
    UNUSED(speed);
    USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

uint8_t *USBD_HID_SerialStrDescriptor(USBD_SpeedTypeDef speed,
                                      uint16_t *length) {
    UNUSED(speed);
    *length = USB_SIZ_STRING_SERIAL;

    /* Update the serial number string descriptor with the data from the unique
     * ID */
    Get_SerialNum();

    return (uint8_t *)USBD_StringSerial;
}

uint8_t *USBD_HID_ConfigStrDescriptor(USBD_SpeedTypeDef speed,
                                      uint16_t *length) {
    if (speed == USBD_SPEED_HIGH) {
        USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING, USBD_StrDesc,
                       length);
    } else {
        USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING, USBD_StrDesc,
                       length);
    }
    return USBD_StrDesc;
}

uint8_t *USBD_HID_InterfaceStrDescriptor(USBD_SpeedTypeDef speed,
                                         uint16_t *length) {
    if (speed == 0) {
        USBD_GetString((uint8_t *)USBD_INTERFACE_STRING, USBD_StrDesc, length);
    } else {
        USBD_GetString((uint8_t *)USBD_INTERFACE_STRING, USBD_StrDesc, length);
    }
    return USBD_StrDesc;
}

static void Get_SerialNum(void) {
    uint32_t deviceserial0;
    uint32_t deviceserial1;
    uint32_t deviceserial2;

    deviceserial0 = *(uint32_t *)DEVICE_ID1;
    deviceserial1 = *(uint32_t *)DEVICE_ID2;
    deviceserial2 = *(uint32_t *)DEVICE_ID3;

    deviceserial0 += deviceserial2;

    if (deviceserial0 != 0) {
        IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
        IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
    }
}

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len) {
    uint8_t idx = 0;

    for (idx = 0; idx < len; idx++) {
        if (((value >> 28)) < 0xA) {
            pbuf[2 * idx] = (value >> 28) + '0';
        } else {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }

        value = value << 4;

        pbuf[2 * idx + 1] = 0;
    }
}
