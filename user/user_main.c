#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

static volatile os_timer_t timer;
static int channel;
int beacon_packet(uint8_t *buf,char *ssid,uint8_t len,uint8_t chan){
	buf[0] = 0x80;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	int i;
	uint8_t mac[]={0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	for (i=0; i<6; i++) buf[i+4] = 0xff;
	for (i=0; i<6; i++) buf[i+10] = mac[i];
	for (i=0; i<6; i++) buf[i+16] = mac[i];
	buf[22] = 0xe0;
	buf[23] = 0x09;
	uint8_t timestamp[]={0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00};
	for (i=0; i<8; i++) buf[i+24] = timestamp[i];
	buf[30]=0x64;
	buf[31]=0x00;
	buf[32]=0x01;
	buf[33]=0x04;
	buf[34]=0x00;
	buf[35]=len;
	for (i=0; i<len; i++) buf[i+36] = (uint8_t)ssid[i];
	uint8_t suffix[]={0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03, 0x01};
	for (i=0; i<12; i++) buf[i+36+len] = suffix[i];
	buf[len+48]=chan;
	return len+49;
}

void change_channel(){
	uint8_t buf[255];
	int size=beacon_packet(buf,"imtoy",5,channel);
	wifi_send_pkt_freedom(buf, size, 0);
	wifi_promiscuous_enable(0);
	switch(channel){
		case 1:
			channel = 14;
			break;
		case 2:
		case 3:
		case 4:
		  channel++;
		  break;
		case 5:
		  channel = 7;
		  break;
		case 6:
		  channel = 1;
		  break;
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		  channel++;
		  break;
		case 13:
		  channel = 6;
		  break;
		case 14:
		  channel = 2;
		  break;
		default:
		  channel = 6;
		  break;
	}
	wifi_set_channel(channel);
	wifi_promiscuous_enable(1);
}


void ICACHE_FLASH_ATTR user_init(){
	os_printf("\n\nSDK version:%s\n", system_get_sdk_version());
	wifi_set_opmode(STATION_MODE);
	
	// Set timer for deauth
	os_timer_disarm(&timer);
	os_timer_setfn(&timer, (os_timer_func_t *)change_channel, NULL);
	os_timer_arm(&timer, 200, 1);
}
