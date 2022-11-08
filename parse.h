#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "hid_constants.h"

// see page 35 of https://www.usb.org/sites/default/files/hid1_11.pdf
struct global_context{
    uint32_t usage_page;
    int32_t logical_minimum;
    int32_t logical_maximum;
    int32_t physical_minimum;
    int32_t physical_maximum;
    int32_t unit_exponent;
    uint32_t unit;
    uint32_t report_size;
    uint32_t report_id;
    uint32_t report_count;
};

struct local_usage{
    uint8_t range_usage;   // defined by USAGE MINIMUM and USAGE MAXIMUM instead of USAGE
    union{
        uint32_t usage;
        struct{
            uint32_t usage_minimum;
            uint32_t usage_maximum;
        };
    };
};

enum main_item_tag{
    INPUT = HID_ITEM_MAIN_INPUT,
    OUTPUT = HID_ITEM_MAIN_OUTPUT,
    FEATURE = HID_ITEM_MAIN_FEATURE,
    COLLECTION = HID_ITEM_MAIN_COLLECTION,
    END_COLLECTION = HID_ITEM_MAIN_END_COLLECTION
}; 

struct main_item_context{
    struct global_context global;
    struct local_usage usages[8];
    uint8_t usages_count;
    uint32_t main_item_data;
    enum main_item_tag main_item_type;
};

typedef struct{
    uint8_t report_id_count;    // 0 if not defined
    uint8_t main_items_count;
    struct main_item_context *main_items[16];
}parse_result_t;

parse_result_t *parse_report_map(uint8_t *HID_ITEM_GLOBAL, size_t HID_ITEM_GLOBAL_len);

