#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
 
int main(int argc, char **argv)
{  
    const char *chipname = "/dev/gpiochip0";
    unsigned int line_red = 24, line_green = 25, line_blue = 5, line_button = 6;
    struct gpiod_chip *chip;
    struct gpiod_request_config *req_cfg_leds, *req_cfg_button;
    struct gpiod_line_config *line_cfg_leds, *line_cfg_button;
    struct gpiod_line_settings *line_setting_leds, *line_setting_button;
    struct gpiod_line_request *request_leds, *request_button;

    // Open the GPIO chip
    chip = gpiod_chip_open(chipname);
    if (!chip) {
        perror("Open chip failed");
        goto cleanup_chip;
    }

    // Allocate and configure the request config
    req_cfg_leds = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg_leds, "led_control");
    req_cfg_button = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg_button, "button_control");

    // Allocate line config
    line_cfg_leds = gpiod_line_config_new();
    line_cfg_button = gpiod_line_config_new();

    // Request the line
    unsigned int offsets_leds[] = {line_red, line_green, line_blue};
    unsigned int offsets_buttons[] = {line_button};

    line_setting_leds = gpiod_line_settings_new();
    if (gpiod_line_settings_set_direction(line_setting_leds, GPIOD_LINE_DIRECTION_OUTPUT))
    {
        perror("Set led line direction failed");
        goto cleanup_line_setting;
    }

    if (gpiod_line_config_add_line_settings(line_cfg_leds, offsets_leds, sizeof(offsets_leds)/sizeof(offsets_leds[0]), line_setting_leds)) {
        perror("Add led line settings failed");
        goto cleanup_line_setting;
    }

    line_setting_button = gpiod_line_settings_new();
    if (gpiod_line_settings_set_direction(line_setting_button, GPIOD_LINE_DIRECTION_INPUT))
    {
        perror("Set button line direction failed");
        goto cleanup_line_setting;
    }

    if (gpiod_line_config_add_line_settings(line_cfg_button, offsets_buttons, sizeof(offsets_buttons)/sizeof(offsets_buttons[0]), line_setting_button)) {
        perror("Add button line settings failed");
        goto cleanup_line_setting;
    }

    request_leds = gpiod_chip_request_lines(chip, req_cfg_leds, line_cfg_leds);
    if (!request_leds) {
        perror("Request led lines failed");
        goto cleanup_request;
    }

    request_button = gpiod_chip_request_lines(chip, req_cfg_button, line_cfg_button);
    if (!request_button) {
        perror("Request button lines failed");
        goto cleanup_request;
    }

    // Blink LEDs in a binary pattern
    while (true) {
        gpiod_line_request_set_value(request_leds, line_red, (i & 1) != 0);
        gpiod_line_request_set_value(request_leds, line_green, (i & 2) != 0);
        gpiod_line_request_set_value(request_leds, line_blue, (i & 4) != 0);
        // Read button status and exit if pressed
        int val = gpiod_line_request_get_value(request_button, line_button);
        if (val == 0) {
            break;
        }
        usleep(100000);
    }

    // Clean up
cleanup_request:
    gpiod_line_request_release(request_leds);
    gpiod_line_request_release(request_button);
cleanup_line_setting:
    gpiod_line_settings_free(line_setting_leds);
    gpiod_line_settings_free(line_setting_button);
cleanup_request_config:   
    gpiod_request_config_free(req_cfg_leds);
    gpiod_request_config_free(req_cfg_button);
cleanup_line_config:     
    gpiod_line_config_free(line_cfg_leds); // Free line_cfg
    gpiod_line_config_free(line_cfg_button); // Free line_cfg
cleanup_chip:    
    gpiod_chip_close(chip);
}