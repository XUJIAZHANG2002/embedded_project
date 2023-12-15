#include "PmodKYPD.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xparameters.h"
#include <stdio.h>
#include <stdlib.h>
void DemoInitialize();
void DemoRun();
void DemoCleanup();
void DisableCaches();
void EnableCaches();
void DemoSleep(u32 millis);

PmodKYPD myDevice_kypd;
PmodTMP3 myDevice_tmp;

int city = 0;
int main(void) {
   DemoInitialize();
   DemoRun();
   DemoCleanup();
   return 0;
}

// keytable is determined as follows (indices shown in Keypad position below)
// 12 13 14 15
// 8  9  10 11
// 4  5  6  7
// 0  1  2  3
#define DEFAULT_KEYTABLE "0FED789C456B123A"

void DemoInitialize() {
   EnableCaches();
   KYPD_begin(&myDevice_kypd, XPAR_PMODKYPD_0_AXI_LITE_GPIO_BASEADDR);
   KYPD_loadKeyTable(&myDevice_kypd, (u8*) DEFAULT_KEYTABLE);

   xil_printf("\x1B[H");  // Move terminal cursor to top left
   xil_printf("\x1B[1K"); // Clear terminal
   xil_printf("Connected to PmodTMP3 Demo over UART\n\r");

   TMP3_begin(&myDevice_tmp, XPAR_PMODTMP3_0_AXI_LITE_IIC_BASEADDR, TMP3_ADDR);
   xil_printf("Connected to PmodTMP3 over IIC on JB\n\r\n\r");
}

void display_temp() {
   double temp  = 0.0;
   double temp2 = 0.0;
   double temp3 = 0.0;


      temp  = TMP3_getTemp(&myDevice);
      temp2 = TMP3_CtoF(temp);
      temp3 = TMP3_FtoC(temp2);

      int temp2_round = 0;
      int temp2_int   = 0;
      int temp2_frac  = 0;
      // Round to nearest hundredth, multiply by 100
      if (temp2 < 0) {
         temp2_round = (int) (temp2 * 1000 - 5) / 10;
         temp2_frac  = -temp2_round % 100;
      } else {
         temp2_round = (int) (temp2 * 1000 + 5) / 10;
         temp2_frac  = temp2_round % 100;
      }
      temp2_int = temp2_round / 100;

      int temp3_round = 0;
      int temp3_int   = 0;
      int temp3_frac  = 0;
      if (temp3 < 0) {
         temp3_round = (int) (temp3 * 1000 - 5) / 10;
         temp3_frac  = -temp3_round % 100;
      } else {
         temp3_round = (int) (temp3 * 1000 + 5) / 10;
         temp3_frac  = temp3_round % 100;
      }
      temp3_int = temp3_round / 100;

      xil_printf("Temperature: %d.%d in Fahrenheit\n\r", temp2_int, temp2_frac);
      xil_printf("Temperature: %d.%d in Celsius\n\r", temp3_int, temp3_frac);
      xil_print("\n\r");


}

void display_forecast(int my_city, int day){


	char *city = "New York";
	switch (my_city){
		case 1:
			*city = "New York";
			break;
		case 2:
			*city = "Dubai";
			break;
		case 3:
			*city = "Shang Hai";
			break;
	}
	char command[100];
	sprintf(command,"python3 weather.py --city \"%s\" --day %d", city,day);

	char buffer[128];

	// Open a pipe to the command and read its output
	FILE *pipe = popen(command, "r");
	if (pipe == NULL) {
		xil_printf("Error opening pipe.\n");
		return -1;
	}

	// Read and print the output
	while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
		xil_printf("%s", buffer);
	}

	// Close the pipe
	int status = pclose(pipe);

	// Check if the command execution was successful
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
		// Read and print the contents of the text file
		FILE *file = fopen("output.txt", "r");
		if (file != NULL) {
			xil_printf("\%s Weather in %d days\n", city, day);
			while (fgets(buffer, sizeof(buffer), file) != NULL) {
				xil_printf("%s\n", buffer);
			}
			fclose(file);
		} else {
			xil_printf("\nError opening the text file.\n");
		}
	} else {
		xil_printf("\nError executing Python script.\n");
	}

	return 0;

}

int do_state(int state, char key){
	int new_state = 0;


	switch(state){
	case 0:
		if (key == 'A'){
			display_temp();
			new_state = 0;
		}
		if (key == 'B'){

			xil_printf("Enter A city \n 1: New York 2:Dubai 3:Shang Hai\n");
			new_state = 1;
		}
		break;
	case 1:
		city = key - '0';
		xil_printf("Enter a date\n");
		new_state = 2;
	case 2:
		int day = key - '0';

		display_forecast(city, day);
		new_state = 0;
	}
	return new_state;
}

void DemoRun() {
   u16 keystate;
   XStatus status, last_status = KYPD_NO_KEY;
   u8 key, last_key = 'x';
   // Initial value of last_key cannot be contained in loaded KEYTABLE string

   Xil_Out32(myDevice_kypd.GPIO_addr, 0xF);

   xil_printf("Pmod KYPD demo started. Press any key on the Keypad.\r\n");
   int state = 0;

   while (1) {
      // Capture state of each key
      keystate = KYPD_getKeyStates(&myDevice_kypd);

      // Determine which single key is pressed, if any
      status = KYPD_getKeyPressed(&myDevice_kypd, keystate, &key);

      // Print key detect if a new key is pressed or if status has changed
      if (status == KYPD_SINGLE_KEY
            && (status != last_status || key != last_key)) {
         xil_printf("Key Pressed: %c\r\n", (char) key);
         last_key = key;
      if (key == 'A'){
         display_temp();
      }
      } else if (status == KYPD_MULTI_KEY && status != last_status)
         xil_printf("Error: Multiple keys pressed\r\n");

      last_status = status;
      state = do_state(state,key);
      usleep(1000);
   }
}

void DemoCleanup() {
   DisableCaches();
}

void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#endif
}
