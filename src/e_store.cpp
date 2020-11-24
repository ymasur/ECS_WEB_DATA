/*
 e_store.cpp
 -----------
 06.11.2020 - ymasur@microclub.ch

 Module store can write datas onto SD/USB storage file
*/
#include <Arduino.h>
#include <FileIO.h>
#include <string.h>
#include "ecs-web-data.h"


#define RETRY 3 //number of retry in write to file (0: mean 1)
#define RETRY_DELAY 5 // nb of millisecond between write try

// organisation of datas :
// "Pump PAN  SOL  ECS"
// "100 67.2 45.0 55.3 "

void convert_datas_to_store(char *dv)
{
  int n, i[NB_PROBES], d[NB_PROBES];  

  sprintf(dv, "%03d",  pump_ratio());

  for (n = 0; n < NB_PROBES; n++)
  {  
    i[n] = (int) t_val[n]; // take integer part
    d[n] = 10 * t_val[n];  // take the decimal part
    d[n] = d[n] % 10;
    sprintf(dv+3+n*5, "\t%02d.%1d", i[n], d[n] );
  }
}

/*  store_datas(char *fname, DateTime dt)
    --------------------------------------------------
    Store the actual datass in a file on SD card
    Global vars used:
    - char[] fname, contain the full path
    - DateTime dt: structure of actual date

    Modified global vars:
    - fname, the filename in a 8.3 format. 4 first chars are modified
    - errFile: false if OK; then true if an error occures
*/

void store_datas(char *fname, DateTime dt)
{
  short i = 0;
  char dateTimeStr[21];
  char datas_str[41];
                                        //   "20/11/11	08:40:00
  snprintf(dateTimeStr, sizeof(dateTimeStr), "%02d/%02d/%02d\t%02d:%02d:%02d\t",
          dt.year()-2000,
          dt.month(),
          dt.day(),
          dt.hour(),
          dt.minute(),
          dt.second()
          );
  // open the file.
  // The FileSystem card is mounted at the following "/mnt/SD" and
  // create the name with year and month on 4 digits
  //  012345678901234567
  // "20/11/11	08:40:00
  fname[OFFSET_YYMM + 0] = dateTimeStr[0];
  fname[OFFSET_YYMM + 1] = dateTimeStr[1];
  fname[OFFSET_YYMM + 2] = dateTimeStr[3];
  fname[OFFSET_YYMM + 3] = dateTimeStr[4];

  fname[OFFSET_YYMM + 4] = 'd';
  fname[OFFSET_YYMM + 5] = 'a';
  fname[OFFSET_YYMM + 6] = 't';
  fname[OFFSET_YYMM + 7] = 'a';   

  convert_datas_to_store(datas_str);

  // open the 'fname' file, 3 try...
  do 
  {
    err_file = true;
    File filept = FileSystem.open(fname, FILE_APPEND);
    if (filept)  // if the file is available, write to it:
    {
      filept.print(dateTimeStr); 
      //filept.print("\t");
      filept.println(datas_str);
      filept.close();
      err_file = false;
    } // file pt OK
    else // A: file not availble, retry
    {
      delay(RETRY_DELAY);
    }
  } while(++i < RETRY && err_file == true);

  if (err_file == true) // Q: is the file isn't open?
  {                    // A: yes, pop up an error:
    String err_msg("Error writing file ");  //$$$

    Serial.print(dateTimeStr);
    Serial.print(err_msg);
    Serial.println(fname);
    err_file = true;   // mark it
    err_msg = err_msg + String(fname);
    log_msg(err_msg); // try to log (can be in error, too)
  }
}

/*  log_msg_SD()
    ------------
    Store the message in a logfile on SD card
    The format is: time + TAB + String given + CRLF
*/
void log_msg_SD(String msg)
{
  const char *flog = "/mnt/sd/arduino/www/ecs.log";
  // open the file, 3 try...
  short i = 0;

  //display_info(msg);
  //dateTime_up_ascii();

  do 
  {
    err_file = true;
    File filept = FileSystem.open(flog, FILE_APPEND);
    if (filept)  // if the file is available, write to it:
    {
      err_file = false;      
      filept.print(dateTimeStr);
      filept.print("\t");
      filept.println(msg);
      filept.close();
    }
    else
    {
      delay(RETRY_DELAY);
    }
  } while(++i < RETRY && err_file == true);

} // log_msg_SD()
