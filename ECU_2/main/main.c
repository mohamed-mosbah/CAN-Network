
/* CAN Network Recceiver Node
 *
 * The following code is for a node in a CAN network. This Node
 * is responsible for receiving data messages which are in the bus. 
 * This node does the following:
 * 1) Initailizes the CAN driver and starts it
 * 2) Listens for pressure and temperature messages
 * 3) Prints the data content of the received messages
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "driver/gpio.h"

/* --------------------- Definitions and static variables ------------------ */
// Configuration
#define TX_GPIO_NUM             GPIO_NUM_5
#define RX_GPIO_NUM             GPIO_NUM_4

#define ID_PRESSURE             0x000
#define ID_TEMPERATURE          0x001


void app_main()
{
    //Initialize configuration structures using macro initializers
    static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
    static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("Driver installed\n");
    } else {
        printf("Failed to install driver\n");
        return;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("Driver started\n");
    } else {
        printf("Failed to start driver\n");
        return;
    }
    int32_t data=0;
    twai_message_t message;
    while(1){
        if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
                if(message.identifier==ID_PRESSURE)
                {
                    printf("Message received\n");
                    if (!(message.rtr)) {
                        data=0;
                        for (int i = 0; i < 4; i++)
                        {
                            //printf("Data byte %d = %d\n", i, (signed char)message.data[i]);
                            data |= message.data[i] << 8*i; 
                        }
                    }
                    printf("Sensor1: Current differential pressure is %d\n\n", data);
                    /*
                    if (message.extd) {
                        printf("Message is in Extended Format\n");
                    } else {
                        printf("Message is in Standard Format\n");
                    }
                    */
                }
                
                if(message.identifier==ID_TEMPERATURE)
                {
                    printf("Message received\n");
                    printf("Sensor1: Current Temperature is %d.%d Celsius\n\n", message.data[0],message.data[1]);
                    /*
                    if (message.extd) {
                        printf("Message is in Extended Format\n");
                    } else {
                        printf("Message is in Standard Format\n");
                    }
                    */
                }

        } 
        
    } 

}