#pragma once

#define HID_ITEM_BTAG_MASK  (0b11110000)
#define HID_ITEM_BTYPE_MASK (0b1100)
#define HID_ITEM_BTYPE_MAIN (0)
#define HID_ITEM_BTYPE_GLOBAL   (1<<2)
#define HID_ITEM_BTYPE_LOCAL    (2<<2)
#define HID_ITEM_BTYPE_RESERVED (3<<2)

#define HID_ITEM_MAIN_INPUT (0x80)
#define HID_ITEM_MAIN_OUTPUT (0x90)
#define HID_ITEM_MAIN_COLLECTION (0xa0)
#define HID_ITEM_MAIN_FEATURE (0xb0)
#define HID_ITEM_MAIN_END_COLLECTION (0xc0)

#define HID_ITEM_GLOBAL_USAGE_PAGE  (0X04)
#define HID_ITEM_GLOBAL_LOGICAL_MINIMUM  (0X14)
#define HID_ITEM_GLOBAL_LOGICAL_MAXIMUM  (0X24)
#define HID_ITEM_GLOBAL_PHYSICAL_MINIMUM  (0X34)
#define HID_ITEM_GLOBAL_PHYSICAL_MAXIMUM  (0X44)
#define HID_ITEM_GLOBAL_UNIT_EXPONENT   (0X54)
#define HID_ITEM_GLOBAL_UNIT   (0X64)
#define HID_ITEM_GLOBAL_REPORT_SIZE   (0X74)
#define HID_ITEM_GLOBAL_REPORT_ID   (0X84)
#define HID_ITEM_GLOBAL_REPORT_COUNT   (0X94)
// PUSH POP RESERVED ignored

#define HID_USAGE_PAGE_GENERIC_DESKTOP (0x01)
#define HID_USAGE_PAGE_BUTTON (0x09)

#define HID_ITEM_LOCAL_USAGE (0X08)
#define HID_ITEM_LOCAL_USAGE_MINIMUM (0X18)
#define HID_ITEM_LOCAL_USAGE_MAXIMUM (0X28)
// other local tags ignored
