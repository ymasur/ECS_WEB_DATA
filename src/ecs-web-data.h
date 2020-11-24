/*
    ecs-web-data.h
    --------------
    11.11.2020 - ymasur@microclub.ch
*/
#define __PROG__ "ECS-WEB-data" // project name
#define VERSION "6.01" // program version

#include <Arduino.h>
#ifndef ECS_WEB_DATA
#define ECS_WEB_DATA

#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <FileIO.h>
#include <avr/wdt.h>
#include <string.h>
#include <Wire.h>
#include <OneWire.h>
#include <time.h>
#include <RTClib.h>
#include <jm_Scheduler.h>
#include <jm_LCM2004A_I2C.h>

#include "e_menu.h"

// Storage class of common variables, always in main module
#ifdef MAIN
  #define CLASS
#else
  #define CLASS extern
#endif

// Const #defined here

#define LED13 13  // on board LED
// SW on Didel LearnCbot
#define SW1 2   //SW1 « P1 » port 2 / actif LOW
#define SW2 3   //SW2 « P2 » port 3 / actif LOW
#define SW_NB 2 // only 2 switches to scan

// LEDs on Didel LearnCbot
#define LED1 4  //LED1 VERTE « L1 » port 4 / actif HIGH
#define LED2 5  //LED2 ROUGE « L2 » port 5 / actif HIGH
#define LED3 6  //LED3 ROUGE « L3 » port 6 / actif HIGH
#define LED4 7  //LED4 VERTE « L4 » port 7 / actif HIGH

// time given by RTC, or Unix cmd              1         2
//                           012345678901234567890
// asci format, as          "2019-08-29 22:10:42"
#define DT_LENGHT 20
#define TIME_S 18 // offset where the second digit is, in string dateTimeStr
CLASS char dateTimeStr[DT_LENGHT+1]; 

// LCD
CLASS jm_LCM2004A_I2C * lcd;

// RTC
CLASS RTC_DS3231 rtc;
CLASS DateTime myTime; // used to maintain time operations
CLASS DateTime bootTime; // simple copy to know it

// freemem
#define LOW_SRAM_ALERT 200  // Normal use : 910..965 left

// global vars are defined here

#define NAME_LENGHT 32  // sufficient for path/name
//                          1         2         3
//                0123456789012345678901234567890
CLASS char fname[NAME_LENGHT+1]
#ifdef MAIN
//                          1         2         3
//                012345678901234567890123456789012
               = "/mnt/sd/arduino/www/tempdata.txt";
//conservative filename 8.3 format
//the four chars 'temp' are replaced by year and month, as 1409
#else
;
#endif
// the first four chars in fname 'temp' are replaced by year and month, 
// as februar 2019 gives : 1902data.txt
#define OFFSET_YYMM 20 // offset used to modify the filename: temp -> YYMM

CLASS bool stored;
CLASS bool err_file;
CLASS bool err_probe;
CLASS byte err_probe_n;
CLASS byte ser_copy;
#define LN0 0x01
#define LN1 0x02
#define LN2 0x04
#define LN3 0x08
#define LN_all 0x0F

CLASS byte tempo_msg;
#define TEMPO_MSG_VAL 10 // in seconds
CLASS bool err_act;
CLASS char err_msg[21];
CLASS DateTime data_time;
CLASS char datas_values[21];

// OneWire DS18B20 Temperature probe
// 3 probes on inputs 8 - 10 - 12
#define NB_PROBES 3
#ifdef MAIN
// One hardware connexion for each probe
CLASS OneWire ds1(8), ds2(10), ds3(12);
#endif
// DS18S20 Temperature onewire probes array,
// usefull for loop operation
CLASS OneWire *ds[NB_PROBES] 
#ifdef MAIN
= {&ds1, &ds2, &ds3}; // Set the probes array
#endif
;
// Probes temperatures values in float, accuracy 0.1 °C
CLASS float t_val[NB_PROBES];

#define PUMP 9 // used pin for measure of pump activity

int getTimeStamp(char *p);
void read_probes();

// prototypes (needed for VSCode/PlateformIO)
void setup();
int freeMemory();
void poll_loop_1_s();
void poll_loop_X_ms();
bool IsSyncTime_10_minutes();
int pump_ratio();

void display_menu();
void menu_select();
//void display_info(String);
void display_info(String info, u8 val=TEMPO_MSG_VAL);
void display_at_ln(String info, u8 ln=1); // Line 0 is alway used for date/time
void display_clock();
void display_datas();
void display_last_rec(String rec);
void display_log(String log);
void display_vers(String vers);
CLASS String last_rec, last_log, last_msg;

void log_err(String msg); // with flag err_act set, onto SD, display and serial
void log_msg(String msg); // onto SD, display and serial
void log_info(String msg); // onto display for short time and serial

// e_DS18B20_Temp_3_probes.cpp
byte start_temperature_conversion(byte n);
float get_temperature(byte n) ;
int read_1_wire(int *val, byte max_scan);
void convert_datas_values( char *dv); // for diplay only

void store_datas(char *fname, DateTime dt);
void log_msg_SD(String msg);

// e_time prototype
void timeSyncInit();
void timeSyncStart();
void timeSync();
void tm_to_ascii(DateTime *dt, char *dateTimeStr);
bool IsSyncTime_10_minutes();
bool IsSyncTime_03h00();

#endif // ECS_WEB_DATA
