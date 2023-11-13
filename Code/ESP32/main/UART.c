#include "./UART.h"
QueueHandle_t qSTM32_event,qLOG_event,qUART_BigSize;
uint8_t countQueueBigSize=0;
EventGroupHandle_t evgUART;
TaskHandle_t TaskHandleBigSize;


#define QUEUE_SIZE_OF_QBIG_SIZE 10

/**
 * @brief 
 * 
 * @param pvParameters 
 */
void TaskUartHandleBigSize(void *pvParameters){
    ESP_LOGI("TaskUartHandleBigSize","Created");
    uint8_t *QueueAreWaiting = (uint8_t*)pvParameters;
    if(*QueueAreWaiting > QUEUE_SIZE_OF_QBIG_SIZE){
        ESP_LOGE("TaskUartHandleBigSize","Queue oversize");
        while(1);
    }
    ESP_LOGI("TaskUartHandleBigSize","QueueAreWaiting:%u",*QueueAreWaiting);
    char *s = (char *)calloc(sizeof(char),((*QueueAreWaiting)*120 + 70));
    char *a = NULL;
    while(1){
        if(xQueueReceive(qUART_BigSize,&a,10/portTICK_PERIOD_MS)){
            strcat(s,a);
            free(a);
            *QueueAreWaiting-=1;  
            if(!*QueueAreWaiting) strcat(s,"\0");
        }
        EventBits_t e = xEventGroupGetBits(evgUART);
        // Check event to avoid send queue (only send queue once) after this task had been deleted
        if(!*QueueAreWaiting && !CHECKFLAG(e,EVT_UART_DELETE_TASK_BIG_SIZE)){
            ESP_LOGI("TaskUartHandleBigSize","All queue item had been received, about to free task");
            // ESP_LOGI("TaskUartHandleBigSize ","Send:%p",s);
            xQueueSend(QUEUE_RX,(void*)&s,2/portTICK_PERIOD_MS);
            xEventGroupSetBits(evgUART,EVT_UART_DELETE_TASK_BIG_SIZE);
            vTaskDelete(TaskHandleBigSize);
        }
    }
}

void TaskUart(void *pvParameters)
{
    uart_event_t event;
    char *s;
    EventBits_t e;
    for(;;) {
        if(xQueueReceive(qSTM32_event, (void * )&event, (TickType_t)10/portTICK_PERIOD_MS)) {
            uartTarget = UART_NUM_2;
            switch(event.type) {
                case UART_DATA:
                    char *dtmp = (char *) malloc(event.size + 1);
                    uart_read_bytes(UART_NUM_2, dtmp, event.size, portMAX_DELAY);
                    dtmp[event.size] = '\0';         
                    xQueueSend(QUEUE_RX,(void*)&dtmp,2/portTICK_PERIOD_MS);
                    break;
                case UART_BREAK: break;
                default: break;
            }
        }
        if(xQueueReceive(qLOG_event, (void * )&event, (TickType_t)10/portTICK_PERIOD_MS)) {
            uartTarget = UART_NUM_0;
            switch(event.type) {
                case UART_DATA:{
                    if(event.size < 120){
                        char *dtmp = (char *) malloc(event.size + 1);
                        uart_read_bytes(UART_NUM_0, dtmp, event.size, portMAX_DELAY);
                        dtmp[event.size] = '\0';
                        e = xEventGroupGetBits(evgUART);

                        // if event EVT_UART_OVERSIZE_HW_FIFO is not trigger
                        if(!CHECKFLAG(e,EVT_UART_OVERSIZE_HW_FIFO)){
                            //send data to QUEUE_RX
                            xQueueSend(QUEUE_RX,(void*)&dtmp,2/portTICK_PERIOD_MS); 
                        } 
                        else { // if data is over 120 bytes
                            // send it to temp queue name qUART_BigSize
                            xQueueSend(qUART_BigSize,(void*)&dtmp,2/portTICK_PERIOD_MS);
                            // increase index queue
                            countQueueBigSize++; 
                            // check if current queue item is smaller than total size item
                            if(countQueueBigSize >= QUEUE_SIZE_OF_QBIG_SIZE){
                                ESP_LOGE("TaskUart","Oversize queue");
                                while (1);
                            }

                            xEventGroupClearBits(evgUART,EVT_UART_OVERSIZE_HW_FIFO);
                            // set bit to create TaskHandleBigSize
                            xEventGroupSetBits(evgUART,EVT_UART_TASK_BIG_SIZE); 
                        }
                    } 
                    else {
                        // set bit to signal data is over 120 bytes
                        xEventGroupSetBits(evgUART,EVT_UART_OVERSIZE_HW_FIFO); 
                        char *dtmp = (char *) malloc(event.size);
                        uart_read_bytes(UART_NUM_0, dtmp, event.size, portMAX_DELAY);
                        xQueueSend(qUART_BigSize,(void*)&dtmp,2/portTICK_PERIOD_MS);
                        countQueueBigSize++;
                        if(countQueueBigSize >= QUEUE_SIZE_OF_QBIG_SIZE){
                            ESP_LOGE("TaskUart","Oversize queue");
                            while (1);
                        }
                    }
                }
				break;
                case UART_BREAK: 
                break;
                default: break;
            }
        }
        //Send data to STM32
        if(xQueueReceive(qSTM32Tx, (void * )&s, (TickType_t)10/portTICK_PERIOD_MS)) {
                uart_write_bytes(UART_NUM_2,s,strlen(s));
                free(s);
        }
        e = xEventGroupWaitBits(evgUART,(EVT_UART_DELETE_TASK_BIG_SIZE | EVT_UART_TASK_BIG_SIZE),pdTRUE,pdFALSE,0);
        if(CHECKFLAG(e,EVT_UART_TASK_BIG_SIZE)){
            xTaskCreate(TaskUartHandleBigSize,"TaskUartHandleBigSize",2048,(void*)&countQueueBigSize,3,&TaskHandleBigSize);
        }
    }
}
void UARTConfig()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    qUART_BigSize = xQueueCreate(QUEUE_SIZE_OF_QBIG_SIZE,sizeof(char*));
    evgUART = xEventGroupCreate();
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, UART_BUFFER_EVENT_SIZE, UART_BUFFER_EVENT_SIZE, UART_QUEUE_EVENT_SIZE, &qSTM32_event, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    uart_driver_install(UART_NUM_0, UART_BUFFER_EVENT_SIZE, UART_BUFFER_EVENT_SIZE, UART_QUEUE_EVENT_SIZE, &qLOG_event, 0);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

}

