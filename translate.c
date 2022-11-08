#include <stdlib.h>
#include <string.h>
#include "translate.h"

translate_model_t detect_translate_model(uint8_t *raw_report_map, size_t length){
    if(length < 4)  return NONE;
    if(raw_report_map[0] != 0x05 || raw_report_map[1] != 0x01)  return NONE;    // Usage Page (Generic Desktop Ctrls)
    if(raw_report_map[1] == 0x09 && raw_report_map[2] == 0x02)  return MOUSE;   // Usage (Mouse)
}

static inline void mouse_translate_usage_handler(struct main_item_context *main_item, struct local_usage *usage, mouse_translate_t *translate, uint8_t byte_offset, uint8_t bit_offset){
    // button translate
    if(main_item->global.usage_page == HID_USAGE_PAGE_BUTTON && main_item->global.logical_minimum == 0 && main_item->global.logical_maximum == 1 \
        && usage->range_usage && usage->usage_minimum == 1){

        translate_item_t *buttons = &translate->buttons;
        if(buttons->defined)  printf("warning:buttons translate already existed, overriding\n");
        buttons->defined = 1;
        buttons->byte_offset = byte_offset;
        buttons->bit_offset = bit_offset;
        buttons->bit_count = usage->usage_maximum;
    }

    // x, y, wheel
    else if(main_item->global.usage_page == HID_USAGE_PAGE_GENERIC_DESKTOP && !usage->range_usage){
        translate_item_t *translate_item = NULL;
        const char *field_name;
        switch(usage->usage){
            case 0x30: translate_item = &translate->x; field_name = "x";  break;
            case 0x31: translate_item = &translate->y; field_name = "y";  break;
            case 0x38: translate_item = &translate->wheel; field_name = "wheel";  break;
        };
        if(translate_item != NULL){
            if(translate_item->defined) printf("warning:%s translate already existed, overriding\n", field_name);
            translate_item->bit_count = main_item->global.report_size;
            translate_item->byte_offset = byte_offset;
            translate_item->bit_offset = bit_offset;
            
            int32_t logical_minimum = main_item->global.logical_minimum;
            int32_t logical_maximum = main_item->global.logical_maximum;
            if(logical_minimum + logical_minimum == 0){
                // already centred to 0
                translate_item->post_scale_bias = translate_item->post_scale_bias = 0;
            }else{
                //output = (input - logical_minimum) / (logical_maximum - logical_minimum) * (standard_maximum - standard_minimum) + standard_minimum;
                translate_item->pre_scale_bias = -1 * main_item->global.logical_minimum;
                translate_item->scale_factor = (32767 - (-32767))/ (logical_maximum - logical_minimum);
                translate_item->post_scale_bias = -32767;
            }
        }
    }
}


void make_mouse_translate(parse_result_t *parse_result, mouse_translate_t *translate){
    memset(translate, 0, sizeof(mouse_translate_t));
    if(parse_result->report_id_count > 1){
        printf("error: translating report map with more than 1 report id is not currently supported\n");
        return;
    }
    uint8_t byte_offset, bit_offset, main_item_byte_offset, main_item_bit_offset;
    for(int i=0;i<parse_result->main_items_count;i++){
        struct main_item_context *main_item = parse_result->main_items[i];
        if(main_item->main_item_type != INPUT) continue;    // ignore non INPUT main items
        if(!(main_item->main_item_data & 1)){  // DATA INPUT instead of CONSTANT INPUT
            for(int j=0;j<main_item->usages_count;j++){
                mouse_translate_usage_handler(main_item, &main_item->usages[j], translate, byte_offset, bit_offset);
                // calc usage offset
                bit_offset += main_item->global.report_size;
                byte_offset += bit_offset >> 3;
                bit_offset &= 7;
            }
        }
        //calc main item offset, main item offset might not the same as usage offset for byte padding, see section 6.2.2.9 Padding of hid def 1.1
        main_item_bit_offset += main_item->global.report_size * main_item->global.report_count;
        byte_offset = main_item_byte_offset = main_item_byte_offset + main_item_bit_offset >> 3;
        bit_offset = main_item_bit_offset = main_item_bit_offset & 7;
    }
}

const uint8_t standard_mouse_report_desc[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x08,        //     Usage Maximum (0x08)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x08,        //     Report Count (8)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x16, 0x01, 0x80,  //     Logical Minimum (-32767)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

const size_t standard_mouse_report_desc_length = sizeof(standard_mouse_report_desc);
