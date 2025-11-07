#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
 
int main(int argc, char **argv)
{  

    const char *chipname = "/dev/gpiochip0";
    unsigned int line_num = 24;
    struct gpiod_chip *chip;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_config *line_cfg;
    struct gpiod_line_request *request;

    // Open the GPIO chip
    chip = gpiod_chip_open(chipname);
    if (!chip) {
        perror("Open chip failed");
        return 1;
    }

    // Allocate and configure the request config
    req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "read_button");

    // Allocate line config
    line_cfg = gpiod_line_config_new();

    // Request the line
    unsigned int offsets[] = {line_num};

    struct gpiod_line_settings *line_setting = gpiod_line_settings_new();
    if (gpiod_line_settings_set_direction(line_setting, GPIOD_LINE_DIRECTION_INPUT))
    {
        perror("Set line direction failed");
        return 1;
    }

    if (gpiod_line_config_add_line_settings(line_cfg, offsets, sizeof(offsets)/sizeof(offsets[0]), line_setting)) {
        perror("Add line settings failed");
        return 1;
    }


    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        perror("Request lines failed");
        gpiod_chip_close(chip);
        return 1;
    }

    // Blink the LED

    /* Read input 10 times */
    for (int i = 10; i > 0; i--) {
        int val = gpiod_line_request_get_value(request, line_num);
        if (val < 0) {
            perror("Read line input failed\n");
            return 1;
        }
        printf("Intput %d on line #%u\n", val, line_num);
        sleep(1);
    }

    // Clean up
    gpiod_line_request_release(request);
    gpiod_line_settings_free(line_setting);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg); // Free line_cfg
    gpiod_chip_close(chip);
}