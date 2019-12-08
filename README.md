# FreeRtos-using-stm32-to-program-external-spi-flash
if your system need to program an external spi flash for storing data such as audio data, osd or user data configuration, then this demo will show you how to open a binary file from window and send it to stm32 to program the spi flash.

This demo code in window has four tasks (freertos):
  
    1. The two Led task send command to stm32 to turn the led on/off
    2. The third tash is the download task. It's open a binary file (led.bin (9kb))
       and send a buffer of 256 bytes + header to stm32 until end of file.
    3. The four task is checking the data from stm32 over serial port and display it on window.
    
How to run it:
  
    1. Use stm32 st-link utility to program the stm32f407-disco board with stm32f407disco_vcp_flash.hex.
    2. From window, run Serialflash.exe. 

You need to select the comport for your stm32f407-disco board in my case it's COM72.

    list of stm32 comports:
    0: STMicroelectronics STLink Virtual COM Port (COM44)
    1: STMicroelectronics Virtual COM Port (COM72)
    Please select comport: 1
    Serial port open successfully
    
    
        addr 0 len: 532
        addr 100 len: 532
        addr 200 len: 532
        …….
        addr 1f00 len: 532
        addr 2000 len: 172

        addr 0 len: 532
        addr 100 len: 532
        addr 200 len: 532
        ….
        addr 1f00 len: 532
        addr 2000 len: 172

On Stm32:

    The idle task (freertos) on stm32 firmware will blink the green led every 200ms.

On Window:
    The serialflash has four tasks:
      
      1. LedTask1 - every 200ms send a command to stm32 to turn on/off the led.
      2. LedTask2 - every 200ms send a command to stm32 to turn on/off the led.
      
This is the vLedTask1 and vLedTask2 is same except for the led number buf[2] = 0x02.

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
  
The vDownloadTask call get_bin_file which uses to open the led.bin file and send 256 bytes each times to stm32. 

        static void vDownloadTask(void *parg) {
          parg = parg;
          while (true) {
            get_bin_file();
          }
        }

        static void get_bin_file(void) {
          FILE *fptr;
          uint8_t buf[256];

          uint32_t addr = 0;
          uint32_t size=256;
          
          //open the binary file
          if ((fptr = fopen(pfile, "rb")) == NULL) { 
            printf("found not found"); 
            return; 
          };

          //read until end of file
          while (!feof(fptr)) {
            xSemaphoreTake(xMutex, portMAX_DELAY);
            
            //read 256 bytes each time
            //fread return number of succecfull bytes read
            //size will have either 256 bytes or the remaining bytes.
            size = fread(buf, sizeof(buf[0]), size, fptr);
            //send it to stm32
            adapter.flash(&adapter, buf, size, addr);
            //next address in the flash to be program
            addr += size;   
            
            xSemaphoreGive(xMutex);
            vTaskDelay(1);
            taskYIELD();
          }
          fclose(fptr);
        }


   
The packet sending from window to stm32 as below.
     
              [ascii]
    
      1. '[' indicates start of packet.
      2. ']' indicates end of packet.
      3. First 2 bytes is opcode.
      4. follow bytes are data. 
      5. All the data bytes and including the opcode will be convert to ascii except the '[' and ']'
      6. Every hex byte now become two ascii bytes.


This code does not have any flow control. it could cause the data overwritten if the sending is faster than the receving.
Every packet send should have a response packet. That will be next my goal to implement this feature.


