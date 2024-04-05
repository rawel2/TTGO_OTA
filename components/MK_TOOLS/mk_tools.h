/*
 * mk_tools.h
 *
 *  Created on: 5 kwi 2022
 *      Author: Miros�aw Karda�
 */

#ifndef COMPONENTS_MK_TOOLS_MK_TOOLS_H_
#define COMPONENTS_MK_TOOLS_MK_TOOLS_H_


/*...... kody steruj�ce do ATB ESP-TERMINAL .........................*/
#define RED		"\x1b""[0;31m"		// Red
#define LIM		"\x1b""[0;32m"		// Lime
#define YEL		"\x1b""[0;33m"		// Yellow
#define BLU		"\x1b""[0;34m"		// Blue
#define MAG		"\x1b""[0;35m"		// Magenta (Fuchsia)
#define AQU		"\x1b""[0;36m"		// Aqua

#define GRE		"\x1b""[0;37m"		// Green
#define BRE		"\x1b""[0;38m"		// Bright Red
#define BBL		"\x1b""[0;39m"		// Bright Blue
#define ORA		"\x1b""[0;40m"		// Orange

#define TCLS	"\x1b""[2J"			// CLS Terminal

#define LLN		"[9;LL]"		// Display in last line
/*...... kody steruj�ce do ATB ESP-TERMINAL - koniec .................*/




/* ustaw czas systemowy w RTOS z okre�leniem czasu letniego lub zimowego isDST=0 (zimowy), isDST=1 (letni) */
extern void mk_set_system_time( int YY, int MM, int DD, int hh, int mm, int ss, int isDST );

/* podaj czas alarmu w postaci time_t z uwzgl�dnieniem bie��cej strefy czasowej */
extern time_t mk_get_alarm_time( int YY, int MM, int DD, int hh, int mm, int ss );


/* sprawd� czy zmieni� si� czas w stosunku do poprzedniego wywo�ania funkcji - wykrywana r�nica 1 sekundy */
extern uint8_t is_time_changed( uint8_t );

/* podaj czas lub dat� na podstawie argumentu now, wg formatu zgodnego z tabel� w pliku PNG */
extern char * mk_get_formatted_datetime( time_t now, char * buf_fmt );

/* podaj czas lub dat� systemow� wg formatu zgodnego z tabel� w pliku PNG */
extern char * mk_get_formatted_system_datetime( char * buf_fmt );



extern char * mk_upper( char * s );
extern char * mk_lower( char * s );




#endif /* COMPONENTS_MK_TOOLS_MK_TOOLS_H_ */
