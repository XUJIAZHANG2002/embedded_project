/*
 * Copyright (C) 2009 - 2019 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <stdio.h>

#include "xparameters.h"

#include "netif/xadapter.h"

#include "platform.h"
#include "platform_config.h"
#if defined (__arm__) || defined(__aarch64__)
#include "xil_printf.h"
#endif

#include "lwip/tcp.h"
#include "xil_cache.h"

#if LWIP_IPV6==1
#include "lwip/ip.h"
#else
#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif
#endif

#include "PmodKYPD.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "PmodTMP3.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xparameters.h"
void DemoInitialize();
void DemoRun();
void DemoCleanup();
void DisableCaches();
void EnableCaches();
void DemoSleep(u32 millis);
void display_temp();
PmodKYPD myDevice;
PmodTMP3 myDevice2;
// keytable is determined as follows (indices shown in Keypad position below)
// 12 13 14 15
// 8  9  10 11
// 4  5  6  7
// 0  1  2  3
#define DEFAULT_KEYTABLE "0FED789C456B123A"


/* defined by each RAW mode application */
void print_app_header();
int start_application(int* city, int* day, int num);
int transfer_data();
void tcp_fasttmr(void);
void tcp_slowtmr(void);

/* missing declaration in lwIP */
void lwip_init();

#if LWIP_IPV6==0
#if LWIP_DHCP==1

int city = 0;
int day = 0;
struct info{
	int* cityArr;
	int* dayArr;
	int len;
};
struct info result;
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *echo_netif;

#if LWIP_IPV6==1
void print_ip6(char *msg, ip_addr_t *ip)
{
	print(msg);
	xil_printf(" %x:%x:%x:%x:%x:%x:%x:%x\n\r",
			IP6_ADDR_BLOCK1(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK2(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK3(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK4(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK5(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK6(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK7(&ip->u_addr.ip6),
			IP6_ADDR_BLOCK8(&ip->u_addr.ip6));

}
#else
void
print_ip(char *msg, ip_addr_t *ip)
{
//	print(msg);
//	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
//			ip4_addr3(ip), ip4_addr4(ip));
}

void
print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{

//	print_ip("Board IP: ", ip);
//	print_ip("Netmask : ", mask);
//	print_ip("Gateway : ", gw);
}
#endif

#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif

#ifdef XPS_BOARD_ZCU102
#ifdef XPAR_XIICPS_0_DEVICE_ID
int IicPhyReset(void);
#endif
#endif
void DemoInitialize() {
   EnableCaches();
   KYPD_begin(&myDevice, XPAR_PMODKYPD_0_AXI_LITE_GPIO_BASEADDR);
   KYPD_loadKeyTable(&myDevice, (u8*) DEFAULT_KEYTABLE);


//   xil_printf("\x1B[H");  // Move terminal cursor to top left
//   xil_printf("\x1B[1K"); // Clear terminal
//   xil_printf("Connected to PmodTMP3 Demo over UART\n\r");

   TMP3_begin(&myDevice2, XPAR_PMODTMP3_0_AXI_LITE_IIC_BASEADDR, TMP3_ADDR);
//   xil_printf("Connected to PmodTMP3 over IIC on JB\n\r\n\r");
}
void display_temp(){
	int count = 0;
	  double temp  = 0.0;
		double temp2 = 0.0;
		double temp3 = 0.0;

		while (1) {
		   temp  = TMP3_getTemp(&myDevice2);
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

		   xil_printf("Temperature: %d.%d in Fahrenheit\n", temp2_int, temp2_frac);
		   xil_printf("Temperature: %d.%d in Celsius\n", temp3_int, temp3_frac);
		   print("\n\r");
		   sleep(1); // Delay
		   count ++;
		   if (count == 1){
			   break;
		   }
    }
}
int* createArray(int length) {
    int* array = (int*)malloc(length * sizeof(int));
    if (array == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return array;
}



void DemoRun() {
   u16 keystate;
   XStatus status, last_status = KYPD_NO_KEY;
   u8 key, last_key = 'x';
   // Initial value of last_key cannot be contained in loaded KEYTABLE string

   Xil_Out32(myDevice.GPIO_addr, 0xF);

   xil_printf("Pmod KYPD demo started. Press any key on the Keypad.\r\n");
   int state = 0;
   int count = 0;
   int num = 0;
   int* cityArray;
   int* dayArray;
   while (1) {
      // Capture state of each key
      keystate = KYPD_getKeyStates(&myDevice);

      // Determine which single key is pressed, if any
      status = KYPD_getKeyPressed(&myDevice, keystate, &key);

      // Print key detect if a new key is pressed or if status has changed
      if (status == KYPD_SINGLE_KEY
            && (status != last_status || key != last_key)) {

    	  if (state == 0){

    	  xil_printf("Welcome to use my weather forecast system. Press B to display room temperature. Press C to find out weather forecast of a city in few days. Key Pressed: %c\r\n", (char) key);
    	  if (key =='A'){
    		  xil_printf("Key A is pressed. quitting...");
    		  return;
    	  }

    	  if (key == 'B'){
    		  display_temp();
    		}

    	  if (key == 'C'){
    		  if (state == 0){
    			  xil_printf("How many queries you want? Enter a number.\r\n");
//    			  xil_printf("enter 1 for New York. 2 for Dubai. 3 for Shang Hai");
    			  state = 1;

    		  }

    	  }
    	  }

    	  if(key !='C'&&state == 1){
    		  num = key - '0';
    		  cityArray = createArray(num);
    		  dayArray = createArray(num);
    		  for (int i = 0; i < num; ++i) {
    		         cityArray[i] = 0;
    		     }
    		  for (int i = 0; i < num; ++i) {
    		         dayArray[i] = 0;
    		     }
    		  xil_printf("You have made %d queries Enter D to continue\r\n", num);
//    		  xil_printf("First Enter 'D'. Then enter a date, which means X days from now");
    		  state = 2;

    	  }
    	  if (key == 'D' &&state == 2){
    		  xil_printf("enter 1 for New York. 2 for Dubai. 3 for Shang Hai\r\n");
    		  state = 3;

    	  }
    	  if (key != 'D' && state == 3){
    		  cityArray[count] = key - '0';
//    		  if (count == num){
//    			  return;
//    		  }else{
//    			  count ++;
//    		  }
    		  state = 4;
    		  xil_printf("Enter C to continue\r\n");

    	  }
    	  if (key == 'C' && state == 4){
    		  xil_printf("enter a date, which means X days from now\r\n");
    		  state = 5;
    	  }
    	  if (key != 'C' && state == 5){
    		  dayArray[count] = key - '0';
    		  count ++;
    		  if (count == num){

    			  result.cityArr = cityArray;
    			  result.dayArr = dayArray;
    			  result.len = num;
    			  return;
    		  }else{
    			  state = 2;
    			  xil_printf("Enter D to continue\r\n");
    		  }
    	  }

         last_key = key;
      }
   else if (status == KYPD_MULTI_KEY && status != last_status){
         xil_printf("Error: Multiple keys pressed\r\n");
      }
      last_status = status;

      usleep(1000);
   }
   return;
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
int main()
{

	DemoInitialize();

	  DemoRun();
//	   DemoCleanup();
//	  xil_printf("%d %d", city,day);
#if LWIP_IPV6==0
	ip_addr_t ipaddr, netmask, gw;

#endif
	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif = &server_netif;
#if defined (__arm__) && !defined (ARMR5)
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

/* Define this board specific macro in order perform PHY reset on ZCU102 */
#ifdef XPS_BOARD_ZCU102
	if(IicPhyReset()) {
		xil_printf("Error performing PHY reset \n\r");
		return -1;
	}
#endif

	init_platform();

#if LWIP_IPV6==0
#if LWIP_DHCP==1
    ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#else
	/* initialize IP addresses to be used */
	IP4_ADDR(&ipaddr,  192, 168,   1, 10);
	IP4_ADDR(&netmask, 255, 255, 255,  0);
	IP4_ADDR(&gw,      192, 168,   1,  1);
#endif
#endif
	print_app_header();

	lwip_init();

#if (LWIP_IPV6 == 0)
	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
#else
	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, NULL, NULL, NULL, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	echo_netif->ip6_autoconfig_enabled = 1;

	netif_create_ip6_linklocal_address(echo_netif, 1);
	netif_ip6_addr_set_state(echo_netif, 0, IP6_ADDR_VALID);

	print_ip6("\n\rBoard IPv6 address ", &echo_netif->ip6_addr[0].u_addr.ip6);

#endif
	netif_set_default(echo_netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(echo_netif);

#if (LWIP_IPV6 == 0)
#if (LWIP_DHCP==1)
	/* Create a new DHCP client for this interface.
	 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
	 * the predefined regular intervals after starting the client.
	 */
	dhcp_start(echo_netif);
	dhcp_timoutcntr = 24;

	while(((echo_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		xemacif_input(echo_netif);

	if (dhcp_timoutcntr <= 0) {
		if ((echo_netif->ip_addr.addr) == 0) {
//			xil_printf("DHCP Timeout\r\n");
//			xil_printf("Configuring default IP of 192.168.1.10\r\n");
			IP4_ADDR(&(echo_netif->ip_addr),  192, 168,   1, 10);
			IP4_ADDR(&(echo_netif->netmask), 255, 255, 255,  0);
			IP4_ADDR(&(echo_netif->gw),      192, 168,   1,  1);
		}
	}

	ipaddr.addr = echo_netif->ip_addr.addr;
	gw.addr = echo_netif->gw.addr;
	netmask.addr = echo_netif->netmask.addr;
#endif

	print_ip_settings(&ipaddr, &netmask, &gw);

#endif
	/* start the application (web server, rxtest, txtest, etc..) */
	start_application(result.cityArr, result.dayArr,result.len);

	/* receive and process packets */
	while (1) {
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		xemacif_input(echo_netif);
		transfer_data();
	}

	/* never reached */
	cleanup_platform();

	return 0;
}
