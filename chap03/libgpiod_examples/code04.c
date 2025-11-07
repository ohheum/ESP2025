#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define CHIP_NAME "/dev/gpiochip0" // Adjust as needed
bool running = true;

void *monitoring_gpio(void *req) {
    struct gpiod_line_request *request = (struct gpiod_line_request *)req;
    struct gpiod_edge_event_buffer *event_buffer;
    struct gpiod_edge_event *event;
    int ret;

    printf("Waiting for edge events on line...\n");

    // Allocate an event buffer
    event_buffer = gpiod_edge_event_buffer_new(1); // Buffer for single event at a time
    if (!event_buffer) {
        perror("gpiod_edge_event_buffer_new");
        goto cleanup_buffer;
    }

    while (running) {
        if (gpiod_line_request_wait_edge_events(request, 5000)) {
            ret = gpiod_line_request_read_edge_events(request, event_buffer, 10);
            if (ret < 0) {
                perror("gpiod_line_request_read_edge_events");
                return NULL;
            }
            for (unsigned int j = 0; j < ret; j++) {
                event = gpiod_edge_event_buffer_get_event(event_buffer, j);
                if (gpiod_edge_event_get_event_type(event) == GPIOD_EDGE_EVENT_RISING_EDGE) {
                    printf("Rising edge event detected: timestamp %llu ns, sequence %u\n", 
                           gpiod_edge_event_get_timestamp_ns(event), gpiod_edge_event_get_line_offset(event));
                } else {
                    printf("Falling edge event detected: timestamp %llu ns, sequence %u\n", 
                           gpiod_edge_event_get_timestamp_ns(event), gpiod_edge_event_get_line_offset(event));
                }
            }
        }
    }

cleanup_buffer:
    gpiod_edge_event_buffer_free(event_buffer);

    return NULL;
}

int main(void) {
    struct gpiod_chip *chip;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;
    struct gpiod_line_config *line_cfg;
    struct gpiod_line_settings *line_setting;

    // Open the GPIO chip
    chip = gpiod_chip_open(CHIP_NAME);
    if (!chip) {
        perror("gpiod_chip_open");
        return EXIT_FAILURE;
    }

    // Allocate and configure the request config
    req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "edge_event_example");

    // Allocate line config
    line_cfg = gpiod_line_config_new();   

    // Configure line settings for events
    line_setting = gpiod_line_settings_new();
    if (!line_setting) {
        perror("gpiod_line_settings_new");
        goto cleanup_chip;
    }
    gpiod_line_settings_set_direction(line_setting, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(line_setting, GPIOD_LINE_EDGE_BOTH); 
    gpiod_line_settings_set_bias(line_setting, GPIOD_LINE_BIAS_PULL_UP); // Example: use pull-up if needed

    unsigned int offsets_buttons[] = {5, 6};
    gpiod_line_config_add_line_settings(line_cfg, offsets_buttons, sizeof(offsets_buttons)/sizeof(offsets_buttons[0]), line_setting);
 
    // Request the line
    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        perror("gpiod_chip_request_lines");
        goto cleanup_settings;
    }

    pthread_t thread;       
    if(pthread_create(&thread, NULL, &monitoring_gpio, request)!=0){
        perror("Failed to create the thread");
        return 1;
    }

    // monitoring_gpio(request);
    char command[10];
    void* result;
    scanf("%s", command);
    if (strcmp(command, "stop")==0) {
        running = false;
    }
    pthread_join(thread, &result);

cleanup_request:
    gpiod_line_request_release(request);
cleanup_settings:
    gpiod_line_settings_free(line_setting);
cleanup_chip:
    gpiod_chip_close(chip);

    return EXIT_SUCCESS;
}
