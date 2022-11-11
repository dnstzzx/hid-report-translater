#include <stdlib.h>
#include "parse.h"
#include "translate.h"

uint8_t report_map[]={
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x08,        //     Report Count (8)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

void main(){
    parse_result_t *result = parse_report_map(report_map, sizeof(report_map));
    mouse_translate_t translate;
    make_mouse_translate(result, &translate);
    free_parse_result(result);
    uint8_t report_in[] = {0x05, 0x40, 0xE2, 0x81};    // x=64, y=-30, z=-127
    standard_mouse_report_t report_out;
    translate_mouse_report(&translate, report_in, sizeof(report_in), &report_out);
    printf("out: [");
    for(int i=0;i<7;i++){
        printf("%02x ", ((uint8_t *)&report_out)[i]);
    }
    printf("]");
    
    
}