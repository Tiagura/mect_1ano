/*
Bruno Lemos 98221
Tiago Marques 98459
*/

/* Includes. zephyr.h is always necessary. Add the othert ones according with the resourtces used*/ 
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys///printk.h>
#include <sys/__assert.h>
#include <string.h>
#include <timing/timing.h>
#include <stdlib.h>
#include <stdio.h>
#include <drivers/adc.h>
#include <drivers/pwm.h>

//----------------------------------- CONFIG ----------------------------------------
/* This is the actual nRF ANx input to use. Note that a channel can be assigned to any ANx. In fact a channel can */
/*    be assigned to two ANx, when differential reading is set (one ANx for the positive signal and the other one for the negative signal) */  
/* Note also that the configuration of differnt channels is completely independent (gain, resolution, ref voltage, ...) */
#define ADC_CHANNEL_INPUT NRF_SAADC_INPUT_AIN1 

#define BUFFER_SIZE 1

#define ADC_NODE DT_NODELABEL(adc)  
const struct device *adc_dev = NULL;

#define SAMP_PERIOD_MS 100

 /* Interval between ADC samples */

static struct gpio_callback button_cb_data;

/*ADC definitions and includes*/
#include <hal/nrf_saadc.h>
#define ADC_RESOLUTION 10
#define ADC_GAIN ADC_GAIN_1_4
#define ADC_REFERENCE ADC_REF_VDD_1_4
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40)
#define ADC_CHANNEL_ID 1

/* Size of stack area used by each thread (can be thread specific, if necessary)*/
#define STACK_SIZE 1024

static uint16_t adc_sample_buffer[BUFFER_SIZE];

/* ADC channel configuration */
static const struct adc_channel_cfg my_channel_cfg = {
	.gain = ADC_GAIN,
	.reference = ADC_REFERENCE,
	.acquisition_time = ADC_ACQUISITION_TIME,
	.channel_id = ADC_CHANNEL_ID,
	.input_positive = ADC_CHANNEL_INPUT
};


// ----------------------------------------------------

/* Thread scheduling priority */
#define thread_SENSOR_prio 90
#define thread_PROCESSING_prio 70
#define thread_LED_MANAGER_prio 50
#define thread_RESET_prio 90

/* Create thread stack space */
K_THREAD_STACK_DEFINE(thread_SENSOR_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_PROCESSING_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_LED_MANAGER_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_RESET_stack, STACK_SIZE);
  
/* Create variables for thread data */
struct k_thread thread_SENSOR_data;
struct k_thread thread_PROCESSING_data;
struct k_thread thread_LED_MANAGER_data;
struct k_thread thread_RESET_data;

/* Create task IDs */
k_tid_t thread_SENSOR_tid;
k_tid_t thread_PROCESSING_tid;
k_tid_t thread_LED_MANAGER_tid;
k_tid_t thread_RESET_tid;

/* Global vars (shared memory between tasks A/B and B/C, resp) */
int sensor_value=0; // check if is float
int average_value=0; // meters in int

/* Semaphores for task synch */
struct k_sem sem_sv; // semaphore for sensor_value
struct k_sem sem_av; // semaphore for average_value

/* Thread code prototypes */
void thread_SENSOR_code(void *argA, void *argB, void *argC);
void thread_PROCESSING_code(void *argA, void *argB, void *argC);
void thread_LED_MANAGER_code(void *argA, void *argB, void *argC);
void thread_RESET_code(void *argA, void *argB, void *argC);
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

/* define led 0 1 2 3 */
#define LED0_NODE DT_NODELABEL(led0)
#define LED1_NODE DT_NODELABEL(led1)
#define LED2_NODE DT_NODELABEL(led2)
#define LED3_NODE DT_NODELABEL(led3)
#define SW0_NODE DT_NODELABEL(button0)

/* Get the device pointer, pin number, and configuration flags. A build error on this line means your board is unsupported. */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

/* Takes one sample */
static int adc_sample(void)
{
	int ret;
	const struct adc_sequence sequence = {
		.channels = BIT(ADC_CHANNEL_ID),
		.buffer = adc_sample_buffer,
		.buffer_size = sizeof(adc_sample_buffer),
		.resolution = ADC_RESOLUTION,
	};

	if (adc_dev == NULL) {
            //printk("adc_sample(): error, must bind to adc first \n\r");
            return -1;
	}

	ret = adc_read(adc_dev, &sequence);
	if (ret) {
            //printk("adc_read() failed with code %d\n", ret);
	}	

	return ret;
}

/* main */
void main(void) {
      
    // /* Create and init semaphores */
    k_sem_init(&sem_sv, 0, 1); // init and create semaphore sensor_value
    k_sem_init(&sem_av, 0, 1); // init and create semaphore average_value
    
    /* Create tasks */
    thread_SENSOR_tid = k_thread_create(&thread_SENSOR_data, thread_SENSOR_stack,
        K_THREAD_STACK_SIZEOF(thread_SENSOR_stack), thread_SENSOR_code,
        NULL, NULL, NULL, thread_SENSOR_prio, 0, K_NO_WAIT);

    thread_PROCESSING_tid = k_thread_create(&thread_PROCESSING_data, thread_PROCESSING_stack,
        K_THREAD_STACK_SIZEOF(thread_PROCESSING_stack), thread_PROCESSING_code,
        NULL, NULL, NULL, thread_PROCESSING_prio, 0, K_NO_WAIT);

	thread_LED_MANAGER_tid = k_thread_create(&thread_LED_MANAGER_data, thread_LED_MANAGER_stack,
        K_THREAD_STACK_SIZEOF(thread_LED_MANAGER_stack), thread_LED_MANAGER_code,
        NULL, NULL, NULL, thread_LED_MANAGER_prio, 0, K_NO_WAIT);

	thread_RESET_tid = k_thread_create(&thread_RESET_data, thread_RESET_stack,
        K_THREAD_STACK_SIZEOF(thread_RESET_stack), thread_RESET_code,
        NULL, NULL, NULL, thread_RESET_prio, 0, K_NO_WAIT);

	if (!device_is_ready(button.port)) {
		//printk("Error: button device %s is not ready\n", button.port->name);
		return;
	}

	// BUTTON CONFIG
	int ret;
	ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0) {
		//printk("Error: gpio_pin_configure_dt failed for button, error:%d", ret);
		return;
	}

	/* Configure the interrupt on the button's pin */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE );
	if (ret < 0) {
		//printk("Error: gpio_pin_interrupt_configure_dt failed for button, error:%d", ret);
		return;
	}
	/* Initialize the static struct gpio_callback variable   */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin)); 	
	
	/* Add the callback function by calling gpio_add_callback()   */
	gpio_add_callback(button.port, &button_cb_data);

    return;

} 

/* button pressed*/
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
    k_thread_suspend(thread_LED_MANAGER_tid);
    k_thread_suspend(thread_SENSOR_tid);
    k_thread_suspend(thread_PROCESSING_tid);
	k_thread_resume(thread_RESET_tid);
}

/* a */
void thread_SENSOR_code(void *argA, void *argB, void *argC){
	int err=0;
	/* ADC setup: bind and initialize */
    adc_dev = device_get_binding(DT_LABEL(ADC_NODE));
	if (!adc_dev) {
        printk("ADC device_get_binding() failed\n");
    } 
    err = adc_channel_setup(adc_dev, &my_channel_cfg);
    if (err) {
        printk("adc_channel_setup() failed with error code %d\n", err);
    }
    
    /* It is recommended to calibrate the SAADC at least once before use*/
    NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;

 	//The main loop
	while (1) {
		int64_t start_time = k_uptime_get();
		err=adc_sample();
        if(err) {
            printk("adc_sample() failed with error code %d\n\r",err);
        }
        else {
            if(adc_sample_buffer[0] > 1023) {
                printk("adc reading out of range\n\r");
            }
            else {
                /* ADC is set to use gain of 1/4 and reference VDD/4, so input range is 0...VDD (3 V), with 10 bit resolution */
                sensor_value = (int) (10*adc_sample_buffer[0]*((float)3/1023));
            }
        }
		//  ---
		printk("--------------------------------------\nTHREAD SENSOR:\n");
		
		printk("	ADC read: %d\n", sensor_value);
		int64_t task_exec = k_uptime_get()-start_time; // tempo de execucao
		// printk("THREAD SENSOR\t\tEXEC TIME: %lldms\n",task_exec);
		k_sem_give(&sem_sv);
		/* Wait for next release instant */ 
		k_msleep(SAMP_PERIOD_MS-task_exec);
	}
}

/* b */
void thread_PROCESSING_code(void *argA, void *argB, void *argC)
{
	int filter_size = 10; // MUST BE 10
	int stored_values [filter_size];
	int pox  = 0;
	int full = 0;
 
 	//The main loop
	while (1) {
		k_sem_take(&sem_sv,  K_FOREVER);
		int64_t start_time = k_uptime_get();
		printk("THREAD PROCESSING:\n");
		stored_values[pox++] = sensor_value;
		sensor_value = 0;	//reset sensor_value to 0
		if(pox==filter_size){
			pox = 0;
			full = 1;
		}
		
		if(full){
			//printk("	Values :");
			int tmp = 0;
			for(uint16_t i = 0; i<filter_size; i++){
				tmp += stored_values[i];
				// printk(" %d",stored_values[i]);
			}
			//printk("\n");
			average_value = tmp/filter_size;
			printk("	MED: %d\n",average_value);
			int64_t task_exec = k_uptime_get()-start_time; // tempo de execucao
			// printk("THREAD PROCESSING\tEXEC TIME: %lldms\n",task_exec);
			k_sem_give(&sem_av);
		}
	}
}

/* c */
void thread_LED_MANAGER_code(void *argA, void *argB, void *argC)
{
	int ret;
	/* Check if device is ready */
	if (!device_is_ready(led0.port)) {
		return;
	}

	/* Configure the GPIO pin for output */
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}


	/* Check if device is ready */
	if (!device_is_ready(led1.port)) {
		return;
	}

	/* Configure the GPIO pin for output */
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

	/* Check if device is ready */
	if (!device_is_ready(led2.port)) {
		return;
	}

	/* Configure the GPIO pin for output */
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}

		/* Check if device is ready */
	if (!device_is_ready(led3.port)) {
		return;
	}

	/* Configure the GPIO pin for output */
	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return;
	}
 
 	//The main loop
	while (1) {
		k_sem_take(&sem_av, K_FOREVER);
		int64_t start_time = k_uptime_get();
		printk("THREAD LED_MANAGER:\n");

		if(average_value<10){
			//acender 1, 2, 3, 4
			printk("	LED 1 2 3 4\n");
			ret = gpio_pin_set_dt(&led0, 1);
			ret = gpio_pin_set_dt(&led1, 1) | ret ;
			ret = gpio_pin_set_dt(&led2, 1) | ret ;
			ret = gpio_pin_set_dt(&led3, 1) | ret ;
			if (ret < 0) { return; }
		}
		else if(average_value<20){
			//acender 1, 2, 3
			printk("	LED 1 2 3\n");
			ret = gpio_pin_set_dt(&led0, 1);
			ret = gpio_pin_set_dt(&led1, 1) | ret;
			ret = gpio_pin_set_dt(&led2, 1) | ret;
			//desligar 4if (ret < 0) { return; }
			ret = gpio_pin_set_dt(&led3, 0) | ret;
			if (ret < 0) { return; }
		}
		else if(average_value<30){
			//acender 1, 2
			printk("	LED 1 2\n");
			ret = gpio_pin_set_dt(&led0, 1);
			ret = gpio_pin_set_dt(&led1, 1) | ret;
			//desligar 3, 4
			ret = gpio_pin_set_dt(&led2, 0) | ret;
			ret = gpio_pin_set_dt(&led3, 0) | ret;
			if (ret < 0) { return; }
		}else{
			//acender 1
			printk("	LED 1\n");
			ret = gpio_pin_set_dt(&led0, 1);
			//desligar 2, 3, 4
			ret = gpio_pin_set_dt(&led1, 0) | ret;
			ret = gpio_pin_set_dt(&led2, 0) | ret;
			ret = gpio_pin_set_dt(&led3, 0) | ret;
			if (ret < 0) { return; }
		}
		int64_t task_exec = k_uptime_get()-start_time; // tempo de execucao
		// printk("THREAD LED\t\tEXEC TIME: %lldms\n",task_exec);
	}
}

/* d */
void thread_RESET_code(void *argA, void *argB, void *argC){
	int i,ret;
	while(1){
		//printk("THREAD RESET : SUSPEND\n");
        k_thread_suspend(thread_RESET_tid);
        //printk("THREAD RESET\nBLINKING . . \n");
		i = 0;
		
		ret = gpio_pin_set_dt(&led0, 0);
		ret = gpio_pin_set_dt(&led1, 0) | ret;
		ret = gpio_pin_set_dt(&led2, 0) | ret;
		ret = gpio_pin_set_dt(&led3, 0) | ret;
		if (ret<0){return;}

        while(i<(5*2*5)){
            k_msleep(1000/(2*5));
            gpio_pin_toggle_dt(&led0);
            gpio_pin_toggle_dt(&led1);
            gpio_pin_toggle_dt(&led2);
            gpio_pin_toggle_dt(&led3);
            i++;
        }

        k_thread_resume(thread_SENSOR_tid);
        k_thread_resume(thread_PROCESSING_tid);
        k_thread_resume(thread_LED_MANAGER_tid);
	}
}