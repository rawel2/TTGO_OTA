/*
 * mk_tools.c
 *
 *  Created on: 5 kwi 2022
 *      Author: Miros�aw Karda�
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>					// dla funkcji typu toupper() itp
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
//#include "esp_spi_flash.h"

//#include "portmacro.h"
#include "FreeRTOSConfig.h"
//#include "..\..\build\include\sdkconfig.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
//#include "nvs_flash.h"
//#include "tcpip_adapter.h"

//#include "esp_http_client.h"

#include "esp_err.h"
#include "esp_log.h"

#include <time.h>
#include <sys/time.h>

//-----------------------------------------


/* ustaw czas systemowy w RTOS z okre�leniem czasu letniego lub zimowego isDST=0 (zimowy), isDST=1 (letni) */
void mk_set_system_time( int YY, int MM, int DD, int hh, int mm, int ss, int isDST ) {

	struct tm tm;

	tm.tm_year 	= YY - 1900;   // Set date
	tm.tm_mon 	= MM - 1;
	tm.tm_mday 	= DD;
	tm.tm_hour 	= hh;      // Set time
	tm.tm_min 	= mm;
	tm.tm_sec 	= ss;
	tm.tm_isdst = isDST; // 1 or 0	1-gdy czas ustawiany w czasie LETNIM, 0-gdy czas ustawiany w czasie ZIMOWYM

	time_t t = mktime(&tm);

	struct timeval now = { .tv_sec = t };
	settimeofday(&now, NULL);
}


/* podaj czas alarmu w postaci time_t z uwzgl�dnieniem bie��cej strefy czasowej */
time_t mk_get_alarm_time( int YY, int MM, int DD, int hh, int mm, int ss ) {

	time_t anow;
	struct tm atm;

	time(&anow);
	localtime_r(&anow, &atm);

	struct tm tm;

	tm.tm_year 	= YY - 1900;   // Set date
	tm.tm_mon 	= MM - 1;
	tm.tm_mday 	= DD;
	tm.tm_hour 	= hh;      // Set time
	tm.tm_min 	= mm;
	tm.tm_sec 	= ss;
	tm.tm_isdst = atm.tm_isdst;// czas letni lub zimowy pobrany z systemu

	time_t t = mktime(&tm);

	struct timeval now = { .tv_sec = t };
	settimeofday(&now, NULL);

	anow = mktime(&tm);

	return anow;
}




/* sprawd� czy zmieni� si� czas w stosunku do poprzedniego wywo�ania funkcji - wykrywana r�nica 1 sekundy */
uint8_t is_time_changed( uint8_t modulo ) {

	time_t now;
	static time_t last;

    time(&now);



    if( last == now ) return 0;

    if(last < 1000) {
    	last = now;
    	return 1;
    }

	last = now;

    if(last % modulo  ) return 0;

    return 1;
}

/* podaj czas lub dat� na podstawie argumentu now, wg formatu zgodnego z tabel� w pliku PNG */
char * mk_get_formatted_datetime( time_t now, char * buf_fmt ) {

	char buf[128];

    struct tm timeinfo;

    localtime_r(&now, &timeinfo);


    if (timeinfo.tm_year < (2016 - 1900)) {
    	buf_fmt[0] = 0;
    	return buf_fmt;
    } else {
    	if( !buf_fmt[0] ) { // je�li pusty string formatowania to wymu� domy�lny format np: 12:34:07 14.05.2022
    		strftime( buf, sizeof(buf), "%X  %d.%m.%Y", localtime(&now) );
    	} else {			// je�li jest string formatuj�cy to go u�yj
    		strftime( buf, sizeof(buf), buf_fmt, localtime(&now) );
    	}
    }

    buf_fmt[0] = 0;				// zeruj string (bufor) formatuj�cy
    strcat( buf_fmt, buf );		// zapis do stringa formatuj�cego dat�, czas wg formatu

    return buf_fmt;				// zwr�� wska�nik na bufor
}


/* podaj czas lub dat� systemow� wg formatu zgodnego z tabel� w pliku PNG */
char * mk_get_formatted_system_datetime( char * buf_fmt ) {

	time_t now;

    time(&now);


    return mk_get_formatted_datetime( now, buf_fmt );

}















char * mk_upper( char * s ) {

	char * res = s;
	do {
		if( *s >= 'a' && *s <= 'z' ) *s &= ~0x20;
	} while( *s++ );
	return res;
}

char * mk_lower( char * s ) {

	char * res = s;
	do {
		if( *s >= 'A' && *s <= 'Z' ) *s |= 0x20;
	} while( *s++ );
	return res;
}
