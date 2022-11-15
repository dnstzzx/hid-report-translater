#pragma once
#include "parse.h"

typedef enum translate_model{NONE, MOUSE, KEYBOARD} translate_model_t;

#pragma pack(1)
typedef struct {
    uint8_t report_id;
    uint8_t buttons;    // 8 buttons available
    int16_t x;  // -32767 to 32767
    int16_t y;  // -32767 to 32767
    int16_t wheel;  // -32767 to 32767
}standard_mouse_report_t;
#pragma pack()

extern const uint8_t standard_mouse_report_desc[];
extern const size_t standard_mouse_report_desc_length;

typedef struct {
    unsigned defined: 1;
    unsigned data_signed: 1;
    uint8_t byte_offset;
    uint8_t bit_offset;
    uint8_t bit_count;

    // data transform, out = (in + pre_scale_bias) * scale_factor + post_scale_bias
    int32_t pre_scale_bias;
    int32_t post_scale_bias;
    double scale_factor;
}translate_item_t;

typedef struct{
    uint8_t report_id;
    translate_item_t buttons;
    translate_item_t x;
    translate_item_t y;
    translate_item_t wheel;
}mouse_translate_t;

translate_model_t detect_translate_model(uint8_t *raw_report_map, size_t length);
void make_mouse_translate(parse_result_t *parse_result, mouse_translate_t *translate);
uint8_t translate_mouse_report(mouse_translate_t *translate, uint8_t *report_in, uint8_t report_in_length, standard_mouse_report_t *report_out);