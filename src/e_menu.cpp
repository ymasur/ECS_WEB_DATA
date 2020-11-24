/*
  e_menu.cpp
  ---------
  06.11.2020 - ymasur@microclub.ch
*/
#include <Arduino.h>
#include "ecs-web-data.h"

#ifndef _MENU_CPP
#define _MENU_CPP

/*
    menu_select()
    -------------
    Follow the menu number to display informations.
    The menu can be modified by push button or via serial command.
*/
void menu_select()
{

  if (sw[B_PLUS]->getActivated()) 
  {
    menu++; // 0..N_MENUS menus
    if (menu > NB_MENUS) menu = 0;

    menu_changed = true;
  }
   
  if (sw[B_MINUS]->getActivated()) 
  {
    menu--; // 0..N_MENUS menus
    if (menu < 0 ) menu = NB_MENUS;

    menu_changed = true;
  }

  if (menu == 0 && menu_changed)
  {
    tempo_msg = 0;
  }
   
  /*
  if (menu == 1) // --- MENU 1 ---
  {

  };

  if (menu == 2) // --- MENU 2
  {
    
  }; 

  if (menu == 3) // --- MENU 3
  {
    
  };
  */
} // fct menu_select() ;

// --- DISPLAY PART ----------------------------------------------------------

// organisation of display :
//  01234567890123456789
// "Pump PAN  SOL  ECS  "
// "100% 67.2 45.0 55.3 "

void convert_datas_values( char *dv)
{
  int n, i[NB_PROBES], d[NB_PROBES];  

  sprintf(dv, "%3d%% ",  pump_ratio());

  for (n = 0; n < NB_PROBES; n++)
  {  
    i[n] = (int) t_val[n]; // take integer part
    d[n] = 10 * t_val[n];  // take the decimal part
    d[n] = d[n] % 10;
    sprintf(dv+5+n*5, "%02d.%1d ", i[n], d[n] );
  }
}

/*  display_at_ln(String info, u8 ln)
    ---------------------------------
    4 lines (0..3) of the display can show 20 char of information.
    Because the display is not consistent, we must adress lines specifically.
    In order to reproduce the same cotent to the serial, a mecanism is set to avoid
    the refresh onto it.
    Command to the serial set a bit to specify wich line cotent is to be copied.
    After the copy, the bit is reset.
    Parameters:
    - info: strig to be displayed
    - ln: line number of the LCD (0..3)
    Vars used/modified:
    - ser_copy: bit of the line to be copied onto serial
*/
void display_at_ln(String info, u8 ln)
{
  byte ser_copy_bit = 1 << ln;  //get the copy bit flag for Serial

  lcd->set_cursor(0, ln); // column, line
  lcd->print(info);

  if (ser_copy_bit && ser_copy)  //Q: is info to be copied onto serial?
  {
    Serial.print("\n" + info);    //A: yes, print it
    ser_copy &= ~ser_copy_bit;    // remove flag
  }
}

/*  LCD display info (standard: line 1)
    Parameters:
    - info: string to be displayed (max 20 chrs...)
    - tempo: time duration of the info, in seconds

    Global vars used/modified:
    - tempo_msg: set to the duration wanted
*/
void display_info(String info, u8 tempo)
{
  tempo_msg = tempo;
  display_at_ln(info, 1);
}

/*  display_clock(void)
    ------------------
    Write the value of date & time on line 0 of LCD
    Global var modified:
    - bootTime, using to get values of RTC
    - dateTimeStr, char array sets with the new value
*/
void display_clock()
{
  tm_to_ascii(&myTime, dateTimeStr);
  display_at_ln(dateTimeStr, 0); 
}

/*  void display_datas()
    --------------------
    Display the field names on Ln 2 and the datas on Ln 3 of the LCD.
    The refresh of the data is avoided if a tempo_msg is running.
    The float values are separated in int part, and one decimal digit.
    Var used: t_val[NB_PROBES]
*/
void display_datas()
{
  convert_datas_values(datas_values);    
  if (tempo_msg)  // if any message waiting
    return;       // pass

  //               01234567890123456789
  // display_at_ln(F("PAN  ECS  SOL  POMPE"), 2);
  display_at_ln(   F("Pump PAN  SOL  ECS  "), 2);
  display_at_ln(datas_values, 3);
}

/*  Menu display part
    -----------------
    The menus are displayed continously
*/

// menu 1 - last record
void display_last_rec()
{
  char date_time_str[21];

  tempo_msg = 1;
  lcd->clear_display();
  display_clock();

  display_at_ln("Dernier enregistr.", 1);
  tm_to_ascii(&data_time, date_time_str);
  display_at_ln(date_time_str, 2);   
  display_at_ln(datas_values, 3);
}

// menu 2 - last log
void display_log(String log)
{
  tempo_msg = 1;
  
  lcd->clear_display();
  display_clock();
  display_at_ln("Dernier log:", 1);
  display_at_ln(log.substring(0, 19), 2);
  display_at_ln(datas_values, 3);  
  err_act = false;
}

// menu 3 - soft version
void display_vers()
{
  tempo_msg = 1;

  lcd->clear_display();
  display_clock();
  display_at_ln("Version soft:", 1);
  display_at_ln(VERSION, 2);
  display_at_ln(datas_values, 3);
}

/*
  menu 4 -last boot
  Convert and shows the date/time of boot
*/
void display_last_boot()
{
  tempo_msg = 1;
  char bootDateTimeStr[21];

  lcd->clear_display();
  display_clock();
  display_at_ln(F("Dernier boot:"), 1);
  tm_to_ascii(&bootTime, bootDateTimeStr);
  display_at_ln(bootDateTimeStr, 2);
  display_at_ln(datas_values, 3);  
}

/*  display_menu()
    --------------
    Called periodically at 1 sec.
    Display menu part, according to state of menu
    Then, shows satus

    Globals vars used:
    - menu: main menu level
    Vars modified:
    - nothing
    Return: -
 */
void display_menu()
{
  char ln_menu[LN_LCD_LENGHT+1]; // used to prepare a line written to the LCD

  lcd->set_cursor(0, 2); // we use lines 3 and 4
      
  if (menu_changed)
  {
    menu_changed = false;
    err_act = false;  // manually clear the log error flag
    snprintf(ln_menu, sizeof(ln_menu), "\n*** menu:%d ***", menu);
    Serial.println(ln_menu);
  }
  
  switch(menu)
  {
    /* ---------------- BASE MENU -------------- */
    case 0:
    display_clock();  // alway displayed on line 0
    display_datas();
    break; // base menu

    /* -------------- MENU 1 ------------- */
    case 1: // last record
    display_last_rec(); 
    break; // menu 1

    /* -------------- MENU 2 ------------- */
    case 2: // last log
    display_log(last_log);    
    break; // menu 2

    /* -------------- MENU 3 ------------- */
    case 3: //version soft
    display_vers();  
    break; // menu 3

    /* -------------- MENU 4 ------------- */
    case 4: // last boot
    display_last_boot();
    break;  // menu 4

    default:  // nothing to do
    break;
  } // switch(menu)

} // fct display_menu()

#endif