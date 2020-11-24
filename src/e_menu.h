/*
  e_menu.h
  --------
  11.11.2020 - ymasur@microclub.ch
 */
#include <Arduino.h>
#ifndef _MENU_HPP
#define _MENU_HPP

#define LN_LCD_LENGHT 20

//#define B_ACT 0   // define logical definition
#define B_PLUS 0
#define B_MINUS 1
#define B_NUMBER 2  // we have only 2 buttons

#define NB_MENUS 4  // Menus are defined as:
// 0) normal
// 1) last record
// 2) last log
// 3) software version
// 4) last boot

#define B_SCAN_PERIOD 23 // in millisecond, buttons scan

// compute the repetition delay for a button continuously pressed
// ex. 4x/sec: 1000/(10*4) = 25 ; gives -> 250 ms
#define B_REP(n) (1000/((B_SCAN_PERIOD) * n))

// compute a waiting time in sec. of a pressed button
#define B_WAIT(n_sec) ((n_sec)*(1000/(B_SCAN_PERIOD)))

#ifdef MAIN
  #define CLASS
#else
  #define CLASS extern
#endif

CLASS char menu;
CLASS char menu_old;
CLASS bool menu_changed;

// fct prototypes
/* moved in ecs-web-data.h */

/*
  class Sw
  --------
  Follow the switch sate.
  Called in a periode of 100 ms, compute the time of actived/desactived switch.
  The hardware connection must be contact closed to 0V -> On state.
*/
class Sw
{
  private:
  volatile unsigned short timer;  //0..30000 * 10 ms, 0..300 s
  volatile bool state; //true: active; false: inact.
  String name;
  u8 pin;
  
  public:
  Sw(u8 pin_in, const String pin_name)
  {
    pin = pin_in;
    pinMode(pin, INPUT_PULLUP);   // useful: no external resistor needed
    digitalWrite(pin, HIGH);      // ensure the level high trough pull-up
    timer = 0;
    state = false;                // true: switch is On
    name = pin_name;
  }

/*  scan()
    ------
    Must be called each 10..50 ms, depend of B_SCAN_PERIOD
 */
  void scan()
  {
    if (timer<30000) timer++;
    bool in = !digitalRead(pin);  // get the reversed value
    if (in != state)    // Q: pin state changed?
    {                   // A: yes,
      state = in;       // Store value & reset counter
      timer = 0;
    }
  }

  public:
  inline bool getSt(){ return state; }
  inline char getStChr(){ return (state ? '1':'0'); }
  inline short getTm(){ return timer; }
  inline String getName(){ return name; }
  inline bool getPressed(){ if (state==true && timer>=1) return true; return false; }
  inline bool getRepeted(){ if (state==true && timer%B_REP(4)==0) return true; return false; }
  inline bool getChanged(){ if (timer<=1) return true; return false; }
  inline bool getActivated(){ if (getChanged() && getPressed()) return true; return false; }

};

CLASS Sw* sw[B_NUMBER];  // switchs, they are needed (instancied with 'new')
#endif