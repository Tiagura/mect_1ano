#include <zephyr.h>
#include <device.h>
#include <stdlib.h>
#include <stdio.h>

#include <drivers/uart.h>
#include <string.h>
#include "cab.h"
#include <math.h>
#include <sys/time.h>

/* CONSTANT VALUES */
#define IMAGE_WIDTH 128
#define N_BUFFERS 5
#define SLICE_TIME_MS 100
#define MAIN_SLEEP_TIME_MS 1000       /* Time between main() activations */
#define FATAL_ERR -1                  /* Fatal error return code, app terminates */
#define UART_NODE DT_NODELABEL(uart0) /* UART Node label, see dts */
#define RXBUF_SIZE 16384                 /* Rbuffer size */
#define TXBUF_SIZE 64                 /* Tbuffer size */
#define RX_TIMEOUT 2000               /* Inactivity period after the instant when last char was received that triggers an revent (in us) */



// extern all function of imageProcAlgo.c
extern int nearObstSearch(uint8_t imageBuf[IMAGE_WIDTH][IMAGE_WIDTH]);
extern int obstCount(uint8_t imageBuf[IMAGE_WIDTH][IMAGE_WIDTH]);
extern int guideLineSearch(uint8_t imageBuf[IMAGE_WIDTH][IMAGE_WIDTH], int16_t *pos, float *angle);

/* ---------------------------------------- thread ---------------------------------------- */

/* Size of stack area used by each thread (can be thread specific, if necessary)*/
#define STACK_SIZE 16384

/* Thread scheduling priority */
#define thread_READ_IMG_prio 2  // should be too high because it can block most important threads and shouldn't be too low priority because it should be executed as fast as possible
#define thread_NEAR_OBSTACLES_prio 1
#define thread_COUNT_OBSTACLES_prio 5
#define thread_ORIENTATION_POSITION_prio 4
#define thread_INFO_prio 1

/* Create thread stack space */
K_THREAD_STACK_DEFINE(thread_READ_IMG_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_NEAR_OBSTACLES_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_COUNT_OBSTACLES_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_ORIENTATION_POSITION_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_INFO_stack, STACK_SIZE);

/* Create variables for thread data */
struct k_thread thread_READ_IMG_data;             // make this a sporadic thread
struct k_thread thread_NEAR_OBSTACLES_data;       // most important thread, exec at highest possible rate
struct k_thread thread_COUNT_OBSTACLES_data;      // non real time thread, third priority level
struct k_thread thread_ORIENTATION_POSITION_data; // soft real time thread, second priority level
struct k_thread thread_INFO_data;                 // most important thread, exec at highest possible rate, exec ALWAYS after the near_obstacles thread or the count_obstacles thread or the orientation_position thread

/* Create task IDs */
k_tid_t thread_READ_IMG_tid;
k_tid_t thread_NEAR_OBSTACLES_tid;
k_tid_t thread_COUNT_OBSTACLES_tid;
k_tid_t thread_ORIENTATION_POSITION_tid;
k_tid_t thread_INFO_tid;

/* Thread code prototypes */
void thread_READ_IMG_code(void *argA, void *argB, void *argC);
void thread_NEAR_OBSTACLES_code(void *argA, void *argB, void *argC);
void thread_COUNT_OBSTACLES_code(void *argA, void *argB, void *argC);
void thread_ORIENTATION_POSITION_code(void *argA, void *argB, void *argC);
void thread_INFO_code(void *argA, void *argB, void *argC);

/* Create semaphore */
struct k_sem sem_acess_info;
struct k_sem sem_update_info;
struct k_sem new_img;
// 3 semaphores for the 3 threads that need to access the image
struct k_sem sem_acess_img_near_obstacles;
struct k_sem sem_acess_img_count_obstacles;
struct k_sem sem_acess_img_orientation_position;

// time



/*-----------s--------------------------------------------------------------------------*/

/* Store variables */
int near_obstacles;
int count_obstacles;
int16_t position;
float angle;

/* Create cab */
static cab *cab_id;

/* uart */

const struct uart_config uart_cfg = {
    .baudrate = 115200,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
    .data_bits = UART_CFG_DATA_BITS_8,
    .flow_ctrl = UART_CFG_FLOW_CTRL_NONE};

/* UAR related variables */
const struct device *uart_dev;       /* Pointer to device struct */
static uint8_t rx_buf[RXBUF_SIZE];   /* Rbuffer, to store received data */
static uint8_t rx_chars[RXBUF_SIZE]; /* chars actually received  */
volatile int uart_rxbuf_nchar = 0;   /* Number of chars currnetly on the rbuffer */

// uint8_t image[IMAGE_WIDTH*IMAGE_WIDTH];

/* UART callback function prototype */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

/* main */
// Initialize all elements to 0x00
uint8_t arr[IMAGE_WIDTH][IMAGE_WIDTH];

int main(void)
{
    // IMITATE IMAGE
    for (int i = 0; i < IMAGE_WIDTH ; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            arr[i][j] = 0x00;
        }
    }
    // Set all elements in the 30th row to 0xff
    for (int j = 0; j < IMAGE_WIDTH; j++) {
        arr[j][29] = 0xff;
    }

    // init semaphores
    k_sem_init(&sem_update_info, 1, 1); // init semaphore with 0 tokens
    k_sem_init(&sem_acess_info, 0, 1);  // init semaphore with 0 tokens
    k_sem_init(&new_img, 0, 1);         // init semaphore with 0 tokens

    k_sem_init(&sem_acess_img_near_obstacles, 0, 1);         // init semaphore with 0 tokens
    k_sem_init(&sem_acess_img_count_obstacles, 0, 1);        // init semaphore with 0 tokens
    k_sem_init(&sem_acess_img_orientation_position, 0, 1);   // init semaphore with 0 tokens

    // init global variables
    near_obstacles = -1;
    count_obstacles = -1;
    position = -1;
    angle = -1.0;

    // UART INIT
    uart_rxbuf_nchar = 0;
    memset(rx_buf, 0, RXBUF_SIZE);
    memset(rx_chars, 0, RXBUF_SIZE);
    int err = 0; /* Generic error */

    /* Bind to UART */
    uart_dev = device_get_binding(DT_LABEL(UART_NODE));
    if (uart_dev == NULL){
        printf("device_get_binding() error for device %s!\n\r", DT_LABEL(UART_NODE));
        return -1;
    } else {
        printf("UART binding successful\n\r");
    }

    /* Configure UART */
    err = uart_configure(uart_dev, &uart_cfg);
    if (err == -ENOSYS)
    { /* If invalid configuration */
        printf("uart_configure() error. Invalid configuration\n\r");
        return -1;
    }
    // change uart priority

    /* Register callback */
    err = uart_callback_set(uart_dev, uart_cb, NULL);
    if (err)
    {
        printf("uart_callback_set() error. Error code:%d\n\r", err);
        return -1;
    }

    /* Enable data reception */
    err = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), RX_TIMEOUT);
    if (err)
    {
        printf("uart_rx_enable() error. Error code:%d\n\r", err);
        return -1;
    }
    // print the atual priority of the uart
    // printf("UART priority: %d\n\r",uart_devf->config->irq_config_func->priority);

    printf("INIT\n\r");


    cab_id = open_cab("cab_img", N_BUFFERS , IMAGE_WIDTH * IMAGE_WIDTH, &arr);
    
    
    
    // init threads     //SOME CHANGED NEEDED HERE
    thread_NEAR_OBSTACLES_tid = k_thread_create(&thread_NEAR_OBSTACLES_data, thread_NEAR_OBSTACLES_stack, K_THREAD_STACK_SIZEOF(thread_NEAR_OBSTACLES_stack), thread_NEAR_OBSTACLES_code, NULL, NULL, NULL, thread_NEAR_OBSTACLES_prio, 0, K_NO_WAIT);
    thread_COUNT_OBSTACLES_tid = k_thread_create(&thread_COUNT_OBSTACLES_data, thread_COUNT_OBSTACLES_stack, K_THREAD_STACK_SIZEOF(thread_COUNT_OBSTACLES_stack), thread_COUNT_OBSTACLES_code, NULL, NULL, NULL, thread_COUNT_OBSTACLES_prio, 0, K_NO_WAIT);
    thread_ORIENTATION_POSITION_tid = k_thread_create(&thread_ORIENTATION_POSITION_data, thread_ORIENTATION_POSITION_stack, K_THREAD_STACK_SIZEOF(thread_ORIENTATION_POSITION_stack), thread_ORIENTATION_POSITION_code, NULL, NULL, NULL, thread_ORIENTATION_POSITION_prio, 0, K_NO_WAIT);
    thread_INFO_tid = k_thread_create(&thread_INFO_data, thread_INFO_stack, K_THREAD_STACK_SIZEOF(thread_INFO_stack), thread_INFO_code, NULL, NULL, NULL, thread_INFO_prio, 0, K_NO_WAIT);

    thread_READ_IMG_tid = k_thread_create(&thread_READ_IMG_data, thread_READ_IMG_stack, K_THREAD_STACK_SIZEOF(thread_READ_IMG_stack), thread_READ_IMG_code, NULL, NULL, NULL, thread_READ_IMG_prio, 0, K_NO_WAIT);

    // Set the time slice of the thread_READ_IMG_tid
    // k_sched_time_slice_set(thread_READ_IMG_tid, K_MSEC(SLICE_TIME_MS), K_NO_WAIT);
    return 0;
}

/* Thread READ_IMG code */
void thread_READ_IMG_code(void *argA, void *argB, void *argC){
    
    int err = 0;  // Generic error
    char ACK = 0x24; // ACK
    int index = 0; // bytes received
    int imageindex = 0; // number of images received

    uint8_t *cab_img = NULL;

    printf("thread_READ_IMG_code\n\r");
    
    while (1){
        
        k_sem_take(&new_img, K_FOREVER);  // wait for new bytes of image

        
        
        // printf("Number of bytes received: %d\n", uart_rxbuf_nchar);
        rx_chars[uart_rxbuf_nchar] = 0; /* Terminate the string */
        uart_rxbuf_nchar = 0;           /* Reset counter */


        if (index == 0){
            cab_img = (uint8_t *)reserve(cab_id); // reserve a buffer to store the image in the start of the image
        }

        memcpy(&cab_img[index], rx_chars, 64); // copy the bytes received to the buffer

        index += 64;    // increment the number of bytes received
        if (index >= IMAGE_WIDTH * IMAGE_WIDTH) { // finish the image

            index = 0; // reset the number of bytes received
            printf("\n\nImage received : %d\n", imageindex); // print the number of images received
            imageindex++; // increment the number of images received

            //  code to print the image

            // for (int i = 0; i < IMAGE_WIDTH; i++)
            // {
            //     for (int j = 0; j < IMAGE_WIDTH; j++)
            //     {
            //         printf("%02x ", cab_img[i * IMAGE_WIDTH + j]);
            //     }
            //     printf("\n\r");
            // }
            // printf("end\n\r");
            

            // put the image in the buffer and verify if there is an error
            if (put_mes(cab_img, cab_id) == -1)
            {
                printf("put_mes() error. Error code:%d\n\r", err);
                return;
            }

            
            k_sem_give(&sem_acess_img_near_obstacles); // give semaphore to thread_NEAR_OBSTACLES
            k_sem_give(&sem_acess_img_count_obstacles); // give semaphore to thread_COUNT_OBSTACLES
            k_sem_give(&sem_acess_img_orientation_position); // give semaphore to thread_ORIENTATION_POSITION
        }
        
        //k_msleep(2);                    // sleep 2ms
        err = uart_tx(uart_dev, &ACK, 1, SYS_FOREVER_MS); // send ACK
        if (err) {
            printf("uart_tx() error. Error code:%d\n\r",err);
        }
    }
    
}

/* Thread NEAR_OBSTACLES code */
void thread_NEAR_OBSTACLES_code(void *argA, void *argB, void *argC)
{
    // k_sleep(K_MSEC(1000));
    // printf("Thread near init \n");

    while (1)
    {
        k_sem_take(&sem_acess_img_near_obstacles, K_FOREVER);
        k_sem_take(&sem_update_info, K_FOREVER);
        
        //uint32_t start = k_uptime_get();
        

        printf("Thread near \n");
        // reserve buffer from cab
        uint8_t *img = (uint8_t *)get_mes(cab_id);

        near_obstacles = nearObstSearch(img);

        // release buffer
        unget(img, cab_id);
        k_sem_give(&sem_acess_info);
    }
}

/* Thread COUNT_OBSTACLES code */
void thread_COUNT_OBSTACLES_code(void *argA, void *argB, void *argC)
{
    // printf("Thread count init\n");
    while (1)
    {
        // reserve buffer from cab
        k_sem_take(&sem_acess_img_count_obstacles, K_FOREVER);
        k_sem_take(&sem_update_info, K_FOREVER);
        
        printf("Thread count \n");

        uint8_t *img = (uint8_t *)get_mes(cab_id);
        

        count_obstacles = obstCount(img);
        unget(img, cab_id); // release buffer
        
        k_sem_give(&sem_acess_info);
    }
}

/* Thread ORIENTATION_POSITION code */
void thread_ORIENTATION_POSITION_code(void *argA, void *argB, void *argC)
{
    //printf("Thread orientation init\n");
    while (1)
    {
        k_sem_take(&sem_acess_img_orientation_position, K_FOREVER);
        k_sem_take(&sem_update_info, K_FOREVER);
 
        printf("Thread orientation \n");
        // reserve buffer from cab
        uint8_t *img = (uint8_t *)get_mes(cab_id);
        guideLineSearch(img, &position, &angle);
        
        // release buffer
        unget(img, cab_id);
        
        k_sem_give(&sem_acess_info);
    }
}

/* Thread INFO code */
void thread_INFO_code(void *argA, void *argB, void *argC)
{
    
    int angle_int;
    int a;
    int b;
    int c;
    
    printf("Thread info init\n");
    while (1)
    {
        k_sem_take(&sem_acess_info, K_FOREVER);
        
        // convert float angle to 3 int
        angle_int = (int)(angle * 100);
        a = angle_int / 100;
        b = (angle_int - a * 100) / 10;
        c = angle_int - a * 100 - b * 10;

        // print info
        printf("\nINFO THREAD\n");
        printf("Near obstacles: %d\n", near_obstacles);
        printf("Count obstacles: %d\n", count_obstacles);
        printf("Position: %d\n", position);
        if ((int)((angle) * 180 / 3.14) < 0 && a == 0){
            printf("Angle: -%d.%d%d radian , %d degree\n", a, abs(b), abs(c), (int)((angle) * 180 / 3.14));
        }
        else{
            printf("Angle: %d.%d%d radian , %d degree\n", a, abs(b), abs(c),(int)((angle) * 180 / 3.14));
        }
        printf("\n");

        k_sem_give(&sem_update_info);
    }
}

/* UART CALLBACK */
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    int err;

    switch (evt->type)
    {

    case UART_TX_DONE:
        // printf("UART_TX_DONE event \n\r");
        break;

    case UART_TX_ABORTED:
        // printf("UART_TX_ABORTED event \n\r");
        break;

    case UART_RX_RDY:

        // printf("UART_RX_RDY event \n\r");
        /* Just copy data to a buffer. Usually it is preferable to use e.g. a FIFO to communicate with a task that shall process the messages*/
        // printf("len: %d",evt->data.rx.len);
        memcpy(&rx_chars[uart_rxbuf_nchar], &(rx_buf[evt->data.rx.offset]), evt->data.rx.len);
        // print the value of the received char
        // printf("Received char: %02\n\r",rx_buf[evt->data.rx.offset]);

        uart_rxbuf_nchar += evt->data.rx.len;
        // k_thread_resume(thread_READ_IMG_tid);
        k_sem_give(&new_img);
        break;

    case UART_RX_BUF_REQUEST:
        break;

    case UART_RX_BUF_RELEASED:
        break;

    case UART_RX_DISABLED:
        /* When the RX_BUFF becomes full Ris is disabled automaticaly.  */
        /* It must be re-enabled manually for continuous reception */
        // printf("UART_RX_DISABLED event \n\r");
        err = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), RX_TIMEOUT);
        if (err)
        {
            printf("uart_rx_enable() error. Error code:%d\n\r", err);
            exit(FATAL_ERR);
        }
        break;

    case UART_RX_STOPPED:
        break;

    default:
        break;
    }
}


// function to change the image from a 1D array to a 2D array
uint8_t **change_image(uint8_t *address, int n_bytes, int rows, int cols)
{
    uint8_t **array = malloc(rows * sizeof(uint8_t));
    for (int i = 0; i < rows; i++)
    {
        array[i] = malloc(cols * sizeof(uint8_t));
    }
    for (int i = 0; i < n_bytes; i++)
    {
        int row = i / cols;
        int col = i % cols;
        array[row][col] = *(address + i);
    }
    return array;
}