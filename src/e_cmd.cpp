/*
  e_cmd.cpp
  ---------
  11.11.2020 - ymasur@microclub.ch
  
  Main module:
  - setup of hardware I/O
  - setup of soft tasks and objects
 */

#define MAIN
#include <Arduino.h>
#include "ecs-web-data.h"

BridgeServer server;
BridgeClient client;
// Schedulers instances
jm_Scheduler pulse_X_ms;
jm_Scheduler pulse_1_s;
// vars
int t0_mem;   //track last value of t_val[0] if pump is active

/* blink(short n=1, short t=1)
   ---------------------------
   Blink n times, with impulse cycle t (1/10 sec)
   *** Used only in setup operations ***
   I/O used: LED13
   return: -
*/
void blink(short n=1, short t=1)
{
  for (short i=0; i<n; i++)
  {
    digitalWrite(LED13, LOW);
    delay(50 * t);
    digitalWrite(LED13, HIGH);
    delay(50 * t);
  }
}

// to be completed/amplifed

// log the error message 
void log_err(String msg)
{
  err_act = true; // make the error flag active (removed by menu)
  log_msg(msg);
}

// log the message and take the last one for reference
void log_msg(String msg)
{
  last_log = msg;   // copy the message
  display_at_ln(msg);
  msg.toCharArray(err_msg, 20);
  log_msg_SD(msg);
  Serial.println(err_msg);
}

// display an info for a short time, and not memorized
void log_info(String msg)
{
  tempo_msg = 10;
  display_at_ln(msg);
  log_msg_SD(msg);
  Serial.println(msg);
}

/*  Pump object
    -----------
    Activation of the pump activity is maintained by scanning the state
    of the pump. THis can be difficult, with PWM activation. In order to have
    enough precision, we must scan quicly and out of 50Hz (20 ms) frequency.
    Sure, the % of ratio is less precise at the end of the measure period.
*/
class Pump
{
  private:
  float on, off;
  int ratio;

  public:
  Pump()    // constructor, oonly clearing internal datas
  {
    on = 0.0;
    off = 0.0;
    ratio = 0;
  }

  // Predicat to know the active state of the pump
  //inline bool is_on(){ return digitalRead(PUMP); }
  // reversed input version
  inline bool is_on(){ return !digitalRead(PUMP); }
  #define PUMP_CORRECTION (1.06) // due of phase dead time

  void scan() // periodically increment the on or the off counters
  {
    if (is_on())
    {
      on = on + 1.0;
      digitalWrite(LED4, 1);
    }
    else
    {
      off = off + 1.0;
      digitalWrite(LED4, 0);
    }
  }

  int calc_ratio()  // compute ratio, in percent
  {
    if ((on + off) < 1.0) return ratio; // avoid DIV 0!
    ratio = (int) 100.0 * (on / (on + off)) * PUMP_CORRECTION;
    if (ratio > 100)  // due to PUMP_CORRECTION, can be over corrected
      ratio = 100;

    return ratio;
  }

  void clear_ratio()
  {
    on = 0.0;
    off = 0.0;
    ratio = 0;
  }
}; // object Pump defined

Pump pump;  // one instance is made

// only for access out of the module
int pump_ratio()
{
  return pump.calc_ratio();
}

// read temperature of NB_PROBES probes
void read_probes(void)
{
  // char msg[21];
  for (byte n = 0; n<NB_PROBES; n++) 
  {
    start_temperature_conversion(n);
    yield();    
  }
    
  delay(1);  // wait for complete conversion

  for (byte n = 0; n < NB_PROBES; n++)
  {
    t_val[n] = get_temperature(n);
    yield();
  }
}

void setup() 
{
  // initialize the digital pin for LEDs as an output.
  pinMode(LED13, OUTPUT);
  digitalWrite(LED13, LOW);

  // I/O of LearnBot circuit
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(PUMP, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  // init an array of button control
  sw[0] = new Sw(SW1, "[+]");
  sw[1] = new Sw(SW2, "[-]");

  // I2C
  Wire.begin();

  // LCD
  lcd = new jm_LCM2004A_I2C();  // Add. 0X27 standard
  lcd->begin();  lcd->clear_display();
  lcd->print(__PROG__ " " VERSION);  // on line 1 of 4
  display_info(F("LCD init done...")); delay(2000);

  // we use serial for log messages
  Serial.begin(9600);
  display_info(F("Serial started..."));
  blink(2,10); //LED13 blink n pulses, 1 sec

  // RTC, start
  display_info(F("Start RTC, read.... ")); delay(2000);

  if (! rtc.begin()) 
  {
    display_info(F("Couldn't find RTC")); delay(2000);
    blink(30,1);  // pulse LED13 3.0 sec at 0.1 Hz 
  }

#if 0 // not for all clock device
  if (rtc.lostPower()) 
  {             //012345678901234567890
    display_info("RTC lost power!     "); delay(2000);
    blink(10,1);  // pulse LED13 n at 0.1 Hz 
  }
#endif
  
  // global vars initialized
  myTime = rtc.now(); // get the RTC time
  bootTime = myTime;  // record the bootTime
  timeSyncInit();
  timeSyncStart();

  menu = 0; // clear menu setting
  // $add clear errors

                //012345678901234567890
  display_info(F("Start polling loops  "), 2);
  pulse_1_s.start(poll_loop_1_s, 1000L * 1000);
  pulse_X_ms.start(poll_loop_X_ms, 1000L * B_SCAN_PERIOD);

  // Bridge startup
  Bridge.begin();

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();

  FileSystem.begin();
    //init vars
  pump.clear_ratio();
  // first call gives garbage
  read_probes();
                //012345678901234567890
  log_msg(F(__PROG__ " " VERSION ));
  err_act = false;  // no error  
  
  display_info(F("Programme pret.      "), 2);

} // end setup()

/* msg_manage()
  -------------
  Management of messages.
  - Info msg are displayed x seconds
  - Error msg are diplayed continuously and stored
  Var used:
  - tempo_msg;  value of info displayed
  - err_msg[21]; storage of last error message (if active)
*/
void msg_manager()
{
  if (tempo_msg)  // Q: is a waiting message displayed?
  {
    tempo_msg--;
    return;
  }
  
  if (err_act)
  {
    display_at_ln(err_msg);
  }
  else
  {             //   01234567890123456789
    display_at_ln(F("Marche normale      "));
  }
}

/*  void serial_cmd()
    -----------------
    Get key onto serial
    Global var modified:
    - menu
    - ser_copy

    The menu number is modified as the hardware switch[+] or [-].
    Limits and display is managed by e_menu.cpp
    The ser_copy byte use 4 bits as flags to control witch line of display (0..3)
    should be copied onto serial.
*/
void serial_cmd()
{
  int c;
  c = Serial.read();
  if (c == -1)
    return;

  //Serial.println(c);
  menu_changed = true;

  switch(c)
  {
    case '+':
      menu++;
      ser_copy = LN_all;        
    break;

    case '-':
      menu--;
      ser_copy = LN_all;  
    break;

    case '0': // menu base
      menu = 0;
      ser_copy = LN_all;      
    break;

    case '1': // last record
      menu = 1;
      ser_copy = LN1+LN2+LN3;
    break;

    case '2': // last log
      menu = 2;
      ser_copy = LN1+LN2;      
    break;

    case '3': // soft version
      menu = 3;
      ser_copy = LN1+LN2;        
    break;

    case '4':
      ser_copy = LN1+LN2;      
      menu = 4;
    break;

    case ' ':
      ser_copy = LN_all;
    break;

    case '?':
    default:
      ser_copy = 0;
      Serial.println("\ncmd: +, -, menu 1, 2, 3, 4.");
    break;
  }
} // serial_cmd()

/*  poll_loop_1_s()
    ---------------
    Called each 1000 ms by the scheduler
    Main loop of the programm
 */
void poll_loop_1_s()
{
  myTime = rtc.now(); // take the actual time
  tm_to_ascii(&myTime, dateTimeStr);

  if (IsSyncTime_03h00())
    timeSyncStart();
  timeSync();

  read_probes();  // take the actual values of probes
  pump.calc_ratio();
  display_menu(); // rest of display depends of the menu choice
  msg_manager();  // message to be displayed

  if (IsSyncTime_10_minutes()) // Q: is time to store datas?
  {
    data_time = myTime; // copy of the time
    
    store_datas(fname, data_time); // A: yes, do this
    pump.clear_ratio(); // prepare for the next period
  }

  digitalWrite(LED13, !digitalRead(LED13)); // life monitoring - blink LED13
}

/*  poll_loop_X_ms()
    ----------------
    Compute the state of switches
    Modified var: intern of object Sw.
    The polling time must be between 10..50 ms
    Return value: -
*/
void poll_loop_X_ms()
{
  // scan all switches
  for(short i = 0; i < SW_NB; i++)
  {
    sw[i]->scan();
  }
  menu_select(); // follow user choice
  pump.scan();  // scan activity of the pump
  serial_cmd();
}

/*  Arduino loop
    ------------
    All the job is done trough jm_cheduler object
 */
void loop()
{
  jm_Scheduler::cycle();
  yield();
} // end loop()
