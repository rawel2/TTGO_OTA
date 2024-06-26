/*
 * mk_wifi.c
 *
 *  Created on: 5 kwi 2022
 *      Author: Miros�aw Karda�
 *
 *      ver: 1.01
 *
 *      - 25.04.2022 dodano obs�ug� synchronizacji czasu przez SNTP
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "FreeRTOSConfig.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <errno.h>

#include "lwip/dns.h"

#include "esp_err.h"
#include "esp_log.h"

#include <time.h>
#include "lwip/apps/sntp.h"
#include <sys/time.h>
#include <esp_mac.h>

//-----------------------------------------

#include "mk_wifi.h"


static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

#define PORT_ENTER_CRITICAL 	portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL 		portEXIT_CRITICAL(&mux)


static TSTA_GOT_IP_CB 		sta_got_ip_callback;
static TSTA_DISCONNECTED 	sta_disconnected_callback;
static TAP_JOIN_CB 			ap_join_callback;
static TAP_LEAVE_CB 		ap_leave_callback;


static const char *TAG = "MK WIFI: ";
static EventGroupHandle_t s_wifi_event_group;

esp_netif_t * mk_netif_sta;
esp_netif_t * mk_netif_ap;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT 	BIT0
#define WIFI_FAIL_BIT      	BIT1


static uint8_t my_sta_ssid[32];
static uint8_t my_sta_pass[64];
static uint8_t use_my_sta_ssid;


static uint8_t wifi_connected;


void set_my_sta_ssid( char *assid, char * apass ) {

	memset( my_sta_ssid, 0, sizeof(my_sta_ssid) );
	memset( my_sta_pass, 0, sizeof(my_sta_pass) );

	if( !assid || !apass ) {
		use_my_sta_ssid = 0;
		return;
	}

	sprintf( (char*)my_sta_ssid, "%s", assid );
	sprintf( (char*)my_sta_pass, "%s", apass );
	use_my_sta_ssid = 1;
}






/*  val=1 STA IP ON, val=0 STA IP OFF  */
static void set_sta_ip_state( int8_t val ) {

	PORT_ENTER_CRITICAL;
	wifi_connected = val;
	PORT_EXIT_CRITICAL;
}

uint8_t get_sta_ip_state( void ) {

	uint8_t res = 0;

	PORT_ENTER_CRITICAL;
	res = wifi_connected;
	PORT_EXIT_CRITICAL;

	return res;
}

uint8_t sta_ip_state_changed( void ) {

	static uint8_t state_old = 5;
	uint8_t state ;

	state = get_sta_ip_state();

	if (state != state_old){
		state_old = state;
		return 1;
	}

	return 0;
}

#if USE_SNTP == 1

/* inicjalizacja SNTP ze steref� czasow� dla Polski, i synchronizacja czasu co godzin� */
void mk_sntp_init( char * sntp_srv ) {

	sntp_stop();

    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode( SNTP_OPMODE_POLL );

    if( sntp_srv ) sntp_setservername(0, sntp_srv);
    else sntp_setservername(0, DEFAULT_NTP_SERVER);

    sntp_init();

	setenv("TZ", CENTRAL_EUROPEAN_TIME_ZONE, 1);
	tzset();
}




#endif






static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {

	char buf[128];

	static int s_retry_num;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    	ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED - event" );
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {

    	set_sta_ip_state( 0 );

    	if( !MAXIMUM_RETRY ) {	// INFINITE
    		if( sta_disconnected_callback ) sta_disconnected_callback();
    		esp_wifi_connect();
    		ESP_LOGI(TAG, "retry to connect to the AP");
    	}
    	else if (s_retry_num < MAXIMUM_RETRY) {	// max recconect = MAXIMUM_RETRY

            if( !s_retry_num ) {
            	if( sta_disconnected_callback ) sta_disconnected_callback();
            }

            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            s_retry_num = 0;
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        set_sta_ip_state( 1 );
#if USE_SNTP == 1
        mk_sntp_init(NULL);
#endif
        if( sta_got_ip_callback ) {
        	sprintf( buf, IPSTR, IP2STR(&event->ip_info.ip) );
        	sta_got_ip_callback( buf );
        }
    }
    else if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        if( ap_join_callback ) {
        	sprintf( buf, MACSTR, MAC2STR(event->mac) );
        	ap_join_callback( buf );
        }
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        if( ap_leave_callback ) {
        	sprintf( buf, MACSTR, MAC2STR(event->mac) );
        	ap_leave_callback( buf );
        }
#if USE_SNTP == 1
        sntp_stop();
#endif
    }
}





void mk_wifi_init( wifi_mode_t wifi_mode,
					TSTA_GOT_IP_CB got_ip_cb,
					TSTA_DISCONNECTED sta_discon_cb,
					TAP_JOIN_CB ap_join_cb,
					TAP_LEAVE_CB ap_leave_cb ) {

	sta_got_ip_callback 		= got_ip_cb;
	sta_disconnected_callback	= sta_discon_cb;
	ap_join_callback 			= ap_join_cb;
	ap_leave_callback 			= ap_leave_cb;

    s_wifi_event_group = xEventGroupCreate();

    esp_netif_deinit();
    esp_wifi_stop();
    esp_wifi_deinit();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    esp_wifi_set_storage( WIFI_STORAGE_RAM );


    esp_netif_init();
    esp_event_loop_create_default();


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

ESP_LOGW(TAG, "WIFI 1");

    esp_wifi_set_mode( wifi_mode );

ESP_LOGW(TAG, "WIFI 2");

    esp_event_handler_instance_register( WIFI_EVENT,
                                    ESP_EVENT_ANY_ID,
									&wifi_event_handler,
                                    NULL,
									&instance_any_id
                                    );
ESP_LOGW(TAG, "WIFI 3");
    /******* jesli ma by� STA ? ****************************************************/
    if( wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA ) {

    	mk_netif_sta = esp_netif_create_default_wifi_sta();
ESP_LOGW(TAG, "WIFI 3,1");
    	esp_event_handler_instance_register(IP_EVENT,
									IP_EVENT_STA_GOT_IP,
									&wifi_event_handler,
									NULL,
									&instance_got_ip
									);
ESP_LOGW(TAG, "WIFI 3,2");

    	if( !use_my_sta_ssid ) {
    		set_my_sta_ssid( STA_SSID, STA_PASS );
    	}

ESP_LOGW(TAG, "WIFI 4");
    	wifi_config_t wifi_config = {};
    	memcpy( wifi_config.sta.ssid, my_sta_ssid, sizeof(my_sta_ssid) );
    	memcpy( wifi_config.sta.password, my_sta_pass, sizeof(my_sta_pass) );

//    	ESP_LOGW("AAA", "SSID: %s", wifi_config.sta.ssid);
//    	ESP_LOGW("AAA", "PASS: %s", wifi_config.sta.password);


		esp_wifi_set_config( WIFI_IF_STA, &wifi_config );

ESP_LOGW(TAG, "WIFI 5");


#if USE_STA_STATIC_IP == 1
        /* wy��cz DHCP */
        tcpip_adapter_dhcpc_stop( TCPIP_ADAPTER_IF_STA );
        /* skonfiguruj statyczny IP */
        tcpip_adapter_ip_info_t ip_info;
        ip_info.ip.addr = ipaddr_addr( STA_IP );
        ip_info.netmask.addr = ipaddr_addr( STA_MASK );
        ip_info.gw.addr = ipaddr_addr( STA_GW );
        tcpip_adapter_set_ip_info( TCPIP_ADAPTER_IF_STA, &ip_info );

        tcpip_adapter_dns_info_t dns;
        dns.ip.addr = ipaddr_addr( STA_DNS1 );
        tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_MAIN, &dns);
        dns.ip.addr = ipaddr_addr( STA_DNS2 );
        tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_BACKUP, &dns);

#endif

    }

    /******* jesli ma by� AP ? ****************************************************/
    if( wifi_mode == WIFI_MODE_APSTA || wifi_mode == WIFI_MODE_AP ) {

    	mk_netif_ap = esp_netif_create_default_wifi_ap();

        wifi_config_t wifi_config = {
            .ap = {
                .ssid = AP_SSID,
                .ssid_len = strlen(AP_SSID),
                .password = AP_PASS,
                .max_connection = AP_MAX_STA_CONN,
                .authmode = AP_AUTH
            },
        };

        if( !strlen(AP_PASS) ) {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }
        esp_wifi_set_config( ESP_IF_WIFI_AP, &wifi_config );
    }




#if USE_AP_USER_IP == 1

	/* wy��cz DHCP */
	esp_netif_dhcps_stop(mk_netif_ap);
	/* skonfiguruj statyczny IP */
	esp_netif_ip_info_t ip_info;
	ip_info.ip.addr = ipaddr_addr( AP_IP );
	ip_info.netmask.addr = ipaddr_addr( AP_MASK );
	ip_info.gw.addr = ipaddr_addr( AP_GW );
	esp_netif_set_ip_info(mk_netif_ap, &ip_info);

	/* wy��cz ponownie DHCP */
	esp_netif_dhcps_start(mk_netif_ap);
#endif

ESP_LOGW(TAG, "WIFI 6");
	esp_wifi_start();
ESP_LOGW(TAG, "WIFI 7");
    esp_wifi_connect();
ESP_LOGW(TAG, "WIFI 8");

    if( wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA ) {
    	ESP_LOGI(TAG, "[ STA MODE ] finished.");
    }

    if( wifi_mode == WIFI_MODE_APSTA || wifi_mode == WIFI_MODE_AP ) {
        ESP_LOGI(TAG, "[ AP MODE ] finished. AP SSID: %s password: %s", AP_SSID, AP_PASS );
    }

    if( wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA ) {
		/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
		 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
		EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
				WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
				pdFALSE,
				pdFALSE,
				portMAX_DELAY);

		/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
		 * happened. */
		if (bits & WIFI_CONNECTED_BIT) {
			ESP_LOGI(TAG, "connected to ap SSID: %s password: %s", STA_SSID, STA_PASS);
		} else if (bits & WIFI_FAIL_BIT) {
			ESP_LOGI(TAG, "Failed to connect to SSID: %s, password: %s", STA_SSID, STA_PASS);
		} else {
			ESP_LOGE(TAG, "UNEXPECTED EVENT");
		}
    }
}


/*
 *	Sposoby u�ycia:
 *
 *	1.) Wyszukanie wszystkich dost�pnych AP w otoczeniu WIFI - mk_wifi_scan( NULL );
 *	2.) Wyszukanie/sprawdzenie konkretnego AP czy dost�pny:
 *
 *	uint8_t myssid[] = "MirMUR";
 *	mk_wifi_scan( myssid );
 *
 *	albo
 *
 *	mk_wifi_scan( (uint8_t*)"MirMUR" );
 *
 */
uint8_t mk_wifi_scan( uint8_t * assid ) {


    wifi_scan_config_t scan_config = {
			.ssid = 0,
			.bssid = 0,
			.channel = 0,	/* 0--all channel scan */
			.show_hidden = 1,
			.scan_type = WIFI_SCAN_TYPE_ACTIVE,
			.scan_time.active.min = 120,
			.scan_time.active.max = 150,
    };

	if( assid ) {
		scan_config.ssid = assid;
	}

	esp_wifi_scan_start(&scan_config, true);

	uint16_t ap_num;
	esp_wifi_scan_get_ap_num(&ap_num);


//	wifi_ap_record_t ap_records[20];
//	esp_wifi_scan_get_ap_records(&ap_num, ap_records);
//
//	printf("\n-------------------------------------------------------------------------\n");
//	printf("|               SSID  (%02d)        | Channel | RSSI |       MAC          |\n", ap_num);
//	printf("-------------------------------------------------------------------------\n");
//	for(int i = 0; i < ap_num; i++) {
//		printf("|%32s | %7d | %4d |  %02X:%02X:%02X:%02X:%02X:%02X |   \n", ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi , *ap_records[i].bssid, *(ap_records[i].bssid+1), *(ap_records[i].bssid+2), *(ap_records[i].bssid+3), *(ap_records[i].bssid+4), *(ap_records[i].bssid+5));
//	}
//	printf("-------------------------------------------------------------------------\n");


	return ap_num;
}


























