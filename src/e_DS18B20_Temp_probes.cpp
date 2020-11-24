/*  e_DS18B20_Temp_probes.cpp
    -------------------------
    11.11.2020 ymasur@microclub.ch

    Start temperature conversion and read results.
    Hardware used: only probe DS18B20, with supply
    Timing: conversion take 27 ms
    OneWire instances are stored in an array: OneWire *ds[NB_PROBES]
*/
#include <Arduino.h>
#include <OneWire.h>
#include "ecs-web-data.h"

#define LARGE 0 // 1 = full messages (bytes, CRC)

/* start_temperature_conversion(byte n)
   ------------------------------------
   Initialize the conversion on a pin 'n'
   Return value:
   - false if all is OK
   - true if any error encountered
*/
byte start_temperature_conversion(byte n)
{
  byte addr[8];

  ds[n]->reset_search();
  if (ds[n]->search(addr)==0)
    return true;


#if LARGE  
  Serial.print("ROM =");
  for(byte i = 0; i < 8; i++) 
  {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }
#endif

  if (OneWire::crc8(addr, 7) != addr[7]) 
  {
#if LARGE      
      Serial.println("CRC is not valid!");
#endif      
      return true;
  }   
  
  if (ds[n]->reset()==0)
    return true;
  ds[n]->select(addr);
  ds[n]->write(0x44, 0);        // start conversion
  return false;
}

/* float get_temperature(byte n)
   -----------------------------
   Read the data bytes of oneWire probe.
   Return temperature of the probe 'n'
*/
float get_temperature(byte n) 
{
  byte i;
  byte data[12];
  byte addr[8];
  float celsius;
  char msg[21];

  digitalWrite(LED1+n, HIGH);

  ds[n]->reset_search();  
  ds[n]->search(addr);
  ds[n]->reset();
  ds[n]->select(addr);    
  ds[n]->write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) // we need 9 bytes
  {           
    data[i] = ds[n]->read();
  }
  digitalWrite(LED1+n, LOW);

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];

  if (raw == 0 && err_probe == false)
  {
      err_probe_n = n+1;
      err_probe = true;
      sprintf(msg, "Sonde %d KO !! ", err_probe_n);
      log_err(msg);
  }
  else if (raw != 0 && err_probe == true && err_probe_n == n+1)
  {
      sprintf(msg, "Sonde %d OK !! ", err_probe_n);
      log_err(msg); 
      err_probe_n = 0;
      err_probe = false;
  }
  
  // sprintf(msg, "raw:%d ", raw); Serial.println(msg);
  celsius = (float)raw / 16.0 + 0.05;

  return celsius;
}
