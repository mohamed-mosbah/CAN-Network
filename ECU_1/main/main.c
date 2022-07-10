/* CAN Network Master Example
*/

/*
 * The following example demonstrates a master node in a TWAI network. The master
 * node is responsible for initiating and stopping the transfer of data messages.
 * The example will execute multiple iterations, with each iteration the master
 * node will do the following:
 * 1) Start the TWAI driver
 * 2) Repeatedly send ping messages until a ping response from slave is received
 * 3) Send start command to slave and receive data messages from slave
 * 4) Send stop command to slave and wait for stop response from slave
 * 5) Stop the TWAI driver
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"


/* --------------------- Definitions and static variables ------------------ */
// Configuration
#define TX_GPIO_NUM             GPIO_NUM_5
#define RX_GPIO_NUM             GPIO_NUM_4

#define CLK_PIN                 GPIO_NUM_32
#define MISO_PIN                GPIO_NUM_34
#define CS                      GPIO_NUM_18

#define ID_PRESSURE             0x000
#define ID_TEMPERATURE          0x001

int32_t SPI_Recieve_Data(void);

void app_main(void){
    float temperature;
    int32_t temp=0;

    //Initialize configuration structures using macro initializers
    static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);

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
    twai_message_t message_pressure;
    message_pressure.identifier = 0x0000;
    message_pressure.extd = 0;                  /**< Standard Frame Format (11bit ID) */
    message_pressure.rtr = 0;                   /**< Message is not a Remote Frame (contains data) */
    message_pressure.data_length_code = 4;

    gpio_set_direction(CLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(CLK_PIN, 1);
    gpio_set_direction(MISO_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(CS, GPIO_MODE_OUTPUT);
    gpio_set_level(CS, 1);
    int32_t data=0, pressure=0;   
    vTaskDelay(100); 
    while(1)
    {
        for (int i = 0; i < 4; i++) {
            message_pressure.data[i] = 0;
        }  
        data=(int32_t)SPI_Recieve_Data(); 

        if((data&0x40000000)==0x00000000)
        {
            pressure=(data&0x3fff0000)>>16;
            //equation for converting pressure sensor reading to 0.01 milli bar 
            pressure=(((pressure - 1638)*64639) >> 12) - 103421;    //[0.01 mbar]
            printf("pressure is: %d \n",pressure); 

            message_pressure.data[0]=(uint8_t)(pressure&0x000000ff);  
            message_pressure.data[1]=(uint8_t)((pressure&0x0000ff00)>>8);    
            message_pressure.data[2]=(uint8_t)((pressure&0x00ff0000)>>16);    
            message_pressure.data[3]=(uint8_t)((pressure&0xff000000)>>24);       
        }

        if (twai_transmit(&message_pressure, pdMS_TO_TICKS(1000)) == ESP_OK) {
            printf("Message queued for transmission\n");
        } else {
            printf("Failed to queue message for transmission\n");
        }


        twai_message_t message_temperature;
        message_temperature.identifier = ID_TEMPERATURE;
        message_temperature.extd = 0;
        message_temperature.data_length_code = 2;

        temp=data&0x00000ff70;
        temp=temp>>5;
        //temp = (( temp * 20000) >> 11) - 5000;        //[0.01°C]
        temperature = ((float)( temp * 200)/2047) - 50;           
        
        printf("Temperature is: %d.%d\n",(uint8_t)temperature,((uint16_t)(temperature*100))%100);
        
        message_temperature.data[0]=(uint8_t)temperature;  
        message_temperature.data[1]=((uint16_t)(temperature*100))%100; 

        if (twai_transmit(&message_temperature, pdMS_TO_TICKS(1000)) == ESP_OK) {
            printf("Message queued for transmission\n");
        } else {
            printf("Failed to queue message for transmission\n");
        }
        vTaskDelay(100);     
    }

}


int32_t SPI_Recieve_Data(void)
{
    gpio_set_level(CS, 0);
    int32_t data =0;
    for(uint16_t i=0;i<32;i++)
    {
        gpio_set_level(CLK_PIN,0); 
        //vTaskDelay(1);
        ets_delay_us(10);
        data|= (gpio_get_level(MISO_PIN)<<(31-i));
        gpio_set_level(CLK_PIN,1);
        ets_delay_us(10);
    }
    gpio_set_level(CS, 1);
    return data;
}
