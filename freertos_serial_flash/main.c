#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <strsafe.h>
#include <stdbool.h>
#include <sys/stat.h>


/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "serial.h"
#include "semphr.h"

/* Demo includes. */
#include "supporting_functions.h"
#define MAX_SERIAL_PORT		10

char *serial_list[MAX_SERIAL_PORT] = { NULL };
struct AdapterTypedef_t adapter = { 0 };

char *pfile = "led.bin";

static void get_bin_file(void);
SemaphoreHandle_t xMutex;

/*
* monitor serial data come from stm32 vcp
*/
static void vSerialTask(void *pvParameters) {
	pvParameters = pvParameters;
	while (true) {
		if (adapter.Handle) {
			xSemaphoreTake(xMutex, portMAX_DELAY);
			if (adapter.is_port_open(&adapter)) {		
				adapter.ser_mon(&adapter);
			}
			xSemaphoreGive(xMutex);
		}
		vTaskDelay(2);
		taskYIELD();
	}
}

/*
* Send command to stm32 to blink led1 every 200ms
*/
static void vLedTask1(void *parg) {
	uint8_t buf[12];
	bool led=false;
	parg = parg;
	buf[0] = SERIAL_PACKET__LED;	//opcode low byte
	buf[1] = 0x00;					//opcode high byte
	buf[2] = 0x01;					//led number
	parg = parg;
	while (true) {
		buf[3] = led ? 0x01 : 0x0;		//led on/off
		led = !led;
		xSemaphoreTake(xMutex, portMAX_DELAY);
		adapter.write(&adapter, buf, 4);
		xSemaphoreGive(xMutex);
		vTaskDelay(200);
		taskYIELD();
	}
}

/*
* Send command to stm32 to blink led2 every 200ms
*/
static void vLedTask2(void *parg) {
	uint8_t buf[12];
	bool led = false;
	parg = parg;
	buf[0] = SERIAL_PACKET__LED;	//opcode low byte
	buf[1] = 0x00;					//opcode high byte
	buf[2] = 0x02;					//led number
	while (true) {
		buf[3] = led ? 0x01 : 0x0;		//led on/off
		led = !led;
		xSemaphoreTake(xMutex, portMAX_DELAY);
		adapter.write(&adapter, buf, 4);
		xSemaphoreGive(xMutex);
		//printf("led2: %s\n", led ? "on" : "off");
		vTaskDelay(200);
		taskYIELD();
	}
}

/*
* Send command to stm32 to blink led3 every 200ms
*/
static void vLedTask3(void *parg) {
	uint8_t buf[12];
	bool led = false;
	parg = parg;
	buf[0] = SERIAL_PACKET__LED;	//opcode low byte
	buf[1] = 0x00;					//opcode high byte
	buf[2] = 0x03;					//led number
	while (true) {
		buf[3] = led ? 0x01 : 0x0;		//led on/off
		led = !led;
		xSemaphoreTake(xMutex, portMAX_DELAY);
		adapter.write(&adapter, buf, 4);
		xSemaphoreGive(xMutex);
		//printf("led3: %s\n", led ? "on" : "off");
		vTaskDelay(200);
		taskYIELD();
	}
}

static void vDownloadTask(void *parg) {
	parg = parg;
	while (true) {
		get_bin_file();
	}
}

/*
* convert from ascii to hex
*/
static uint8_t ascii2hex(uint8_t data) {
	if (data >= 0 && data <= 9) {
		return data + '0';
	}
	else if (data >= 0xa && data <= 0x0f) {
		return data - 10 + 'a' ;
	}
	return 0;
}

/*
* get the stm32 binary from user.
*/
static void get_bin_file(void) {
	uint8_t buf[256];
	FILE *fptr;
	uint32_t addr = 0;
	uint32_t size=256;

	if ((fptr = fopen(pfile, "rb")) == NULL) { 
		printf("found not found"); 
		return; 
	};

	while (!feof(fptr)) {
		xSemaphoreTake(xMutex, portMAX_DELAY);
		size = fread(buf, sizeof(buf[0]), size, fptr);
		adapter.flash(&adapter, buf, size, addr);
		addr += size;
		xSemaphoreGive(xMutex);
		vTaskDelay(1);
		taskYIELD();
	}
	fclose(fptr);
}

/*
* create a serial connection.
*/
void serial_create(void) {
	char **p_ptr;

	//check if stm32 comport in the window
	if (serial_devices_mon()) {
		int id = 0;
		//get the stm32 comport list
		serial_devices_get(&serial_list);
		p_ptr = serial_list;
		if (*p_ptr != NULL) {
			printf("\nlist of stm32 comports:\n");
			while (*p_ptr) {
				printf("%d: %s\n", id++, *p_ptr++);
			}
			printf("Please select comport: ");
			scanf("%d", &id);
			//create a serial port instance
			serial_devices_create(&adapter, NULL, serial_list[id]);
			//now open it.
			if (serial_devices_open(&adapter)) {
				printf("Serial port open successfully\n");
			}
		}
	}
}

int main(int argc, char *argv[])
{	
	//get_bin_file(pfile);

	if (argc >= 2) {
		pfile = argv[1];
	} 
	//look for the stm32 comport.
	serial_create();
	xMutex = xSemaphoreCreateMutex();

	//task to monitor the serial port add or remove from window
	xTaskCreate(vLedTask1, "led task1", 1024, NULL, 1, NULL);
	xTaskCreate(vLedTask2, "led task2", 1024, NULL, 1, NULL);
	//xTaskCreate(vLedTask3, "led task3", 1024, NULL, 1, NULL);
	//
	xTaskCreate(vDownloadTask, "dowload task", 1024, NULL, 1, NULL);
	//task to read the serial data
	xTaskCreate(vSerialTask, "read data", 1024, NULL, 1, NULL);

	vTaskStartScheduler();
	for (;; );
}
