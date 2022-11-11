#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include "parse.h"

/* originally from esp_hid_common.c of ESP-IDF by espressif*/
typedef struct {
    uint8_t cmd;
    uint8_t len;
    union {
        uint32_t value;
        uint8_t data[4];
    };
} hid_report_cmd_t;


/* originally from esp_hid_common.c of ESP-IDF by espressif*/
static int parse_cmd(const uint8_t *data, size_t len, size_t index, hid_report_cmd_t **out)
{
    if (index == len) {
        return 0;
    }
    hid_report_cmd_t *cmd = (hid_report_cmd_t *)malloc(sizeof(hid_report_cmd_t));
    if (cmd == NULL) {
        return -1;
    }
    const uint8_t *dp = data + index;
    cmd->cmd = *dp & 0xFC;
    cmd->len = *dp & 0x03;
    cmd->value = 0;
    if (cmd->len == 3) {
        cmd->len = 4;
    }
    if ((len - index - 1) < cmd->len) {
        printf("not enough bytes! cmd: 0x%02x, len: %u, index: %u\n", cmd->cmd, cmd->len, index);
        free(cmd);
        return -1;
    }
    memcpy(cmd->data, dp + 1, cmd->len);
    *out = cmd;
    return cmd->len + 1;
}


struct parse_context{
    struct main_item_context *item_context;
    parse_result_t *out;
    uint8_t collection_depth;
    int8_t report_ids[10];
};

static void dump_cmd(hid_report_cmd_t *item){
    printf("command %02x, data %d\n", item->cmd, item->value);
}

static inline int32_t sign_int(uint32_t data, uint8_t origin_size){
    switch(origin_size){
        case 0: return 0;
        case 1: return (int8_t)(data);
        case 2: return (int16_t)(data);
        case 4: return data;
        default: return data;
    };
}


static int handle_cmd(hid_report_cmd_t *item,struct parse_context *parse_context){
    uint8_t item_type = item->cmd & HID_ITEM_BTYPE_MASK;
    uint32_t *item_data = &item->value;
    //dump_cmd(item);

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    *item_data = ((*item_data&0x000000ff) << 24 | (*item_data&0x0000ff00) << 8 | (*item_data&0x00ff0000) >> 8 | (*item_data&0xff000000) >> 24);
#endif

    struct main_item_context *item_context = parse_context->item_context;
    parse_result_t *out = parse_context->out;
    
    if (item_type == HID_ITEM_BTYPE_GLOBAL){
        switch (item->cmd){
            case HID_ITEM_GLOBAL_USAGE_PAGE:
                item_context->global.usage_page = *item_data;
                break;
            case HID_ITEM_GLOBAL_LOGICAL_MINIMUM:
                item_context->global.logical_minimum = sign_int(*item_data, item->len);
                break;
            case HID_ITEM_GLOBAL_LOGICAL_MAXIMUM:
                item_context->global.logical_maximum = sign_int(*item_data, item->len);
                break;
            case HID_ITEM_GLOBAL_PHYSICAL_MINIMUM:
                item_context->global.physical_minimum = sign_int(*item_data, item->len);
                break;
            case HID_ITEM_GLOBAL_PHYSICAL_MAXIMUM:
                item_context->global.physical_maximum = sign_int(*item_data, item->len);
                break;
            case HID_ITEM_GLOBAL_UNIT_EXPONENT:
                item_context->global.unit_exponent = (int32_t)*item_data;
                break;
            case HID_ITEM_GLOBAL_UNIT:
                item_context->global.unit = *item_data;
                break;
            case HID_ITEM_GLOBAL_REPORT_SIZE:
                item_context->global.report_size = *item_data;
                break;
            case HID_ITEM_GLOBAL_REPORT_ID:
                item_context->global.report_id = *item_data;
                uint8_t is_new_report_id = 1;
                for(int i=0;i<parse_context->out->report_id_count;i++){
                    if(parse_context->report_ids[i] == *item_data){
                        is_new_report_id = 0;
                        break;
                    }
                }
                if(is_new_report_id){
                    if(parse_context->out->report_id_count >= 10){
                        printf("warning: more than 10 report id, stop counting\n");
                    }else{
                        parse_context->report_ids[parse_context->out->report_id_count] = *item_data;
                        parse_context->out->report_id_count ++;
                    }
                }
                break;
            case HID_ITEM_GLOBAL_REPORT_COUNT:
                item_context->global.report_count = *item_data;
                break;
            default:
                printf("warning:ignored unsupported global tag %02x\n", item->cmd);
                break;
        };
    }else if (item_type == HID_ITEM_BTYPE_LOCAL){
        switch (item->cmd){
            case HID_ITEM_LOCAL_USAGE_MINIMUM:{
                struct local_usage *usage_item = &item_context->usages[item_context->usages_count];
                item_context->usages_count ++;

                usage_item->range_usage = 1;
                usage_item->usage_minimum = *item_data;
                usage_item->usage_maximum = 0;
            }break;
            case HID_ITEM_LOCAL_USAGE_MAXIMUM:{
                struct local_usage *last_usage_item = &item_context->usages[item_context->usages_count - 1];
                if(!last_usage_item->range_usage || last_usage_item->usage_maximum != 0){
                    printf("error: no USAGE MINIMUM precceds USAGE MAXIMUM\n");
                    return -1;
                }else{
                    last_usage_item->usage_maximum = *item_data;
                }
            }break;
            case HID_ITEM_LOCAL_USAGE:{
                struct local_usage *usage_item = &item_context->usages[item_context->usages_count];
                item_context->usages_count ++;

                usage_item->range_usage = 0;
                usage_item->usage = *item_data;
            }break;
            default:
                printf("warning:ignored unsupported local tag %02x\n", item->cmd);
                break;
        };
    }else if (item_type == HID_ITEM_BTYPE_MAIN){
        struct main_item_context *old_context = item_context;
        struct main_item_context *new_context = calloc(1, sizeof(struct main_item_context));
        new_context->global = old_context->global;
        item_context = parse_context->item_context = new_context;

        if(item->cmd == HID_ITEM_MAIN_INPUT || item->cmd == HID_ITEM_MAIN_OUTPUT || item->cmd == HID_ITEM_MAIN_FEATURE){
            old_context->main_item_type = item->cmd;
            old_context->main_item_data = *item_data;
            out->main_items[out->main_items_count] = old_context;
            out->main_items_count ++;
            
        }else if(item->cmd == HID_ITEM_MAIN_COLLECTION){
            parse_context->collection_depth ++;
            free(old_context);
        }else if(item->cmd == HID_ITEM_MAIN_END_COLLECTION){
            free(old_context);
            if(parse_context->collection_depth == 0){
                printf("error:illegal report map trying to END COLLECTION with no matched COLLECTION\n");
                return -1;
            }
            parse_context->collection_depth --;
        }else{
            free(old_context);
            printf("warning:ignored unsupported local tag %02x\n", item->cmd);
        }
        
    }else{
        printf("warning:ignored unsupported tag %02x\n", item->cmd);
    }
    return 0;
}

parse_result_t *parse_report_map(uint8_t *raw_report_map, size_t raw_report_map_len)
{
    size_t index = 0;
    int res;
    parse_result_t *out = calloc(1, sizeof(parse_result_t));

    struct parse_context context = {
        .collection_depth = 0,
        .item_context = calloc(1, sizeof(struct main_item_context)),
        .out = out
    };

    while (index < raw_report_map_len) {
        hid_report_cmd_t *cmd;
        res = parse_cmd(raw_report_map, raw_report_map_len, index, &cmd);
        if (res < 0) {
            printf("Failed decoding the descriptor at index: %u\n", index);
            return NULL;
        }
        index += res;
        res = handle_cmd(cmd, &context);
        free(cmd);
        if (res != 0) {
            printf("Failed parsing the descriptor at index: %u\n", index);
            return NULL;
        }
    }
    return out;
}

void free_parse_result(parse_result_t *rst){
    for(int i=0;i<rst->main_items_count;i++){
        free(rst->main_items[i]);
    }
    free(rst);
}

