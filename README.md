# ECS_WEB_DATA
Arduino part of ECS monitoring

Based on a Arduino Yun, the system get each 10 minutes the temperatures of 3 probes:
* solar water, incoming
* water storage 1 (in use, warmed by gaz in cas of insufficient sun)
* water storage 2 (warmed by sun) The hardware contents must be:
* Power supply 5V
* Arduino Yun
* Electronic relay 230V-5V
* RTC DS3231
* LCD LCM2004A Display 4 lines 20 chrs
* 3 x temperatures probes DS18B20 

This WEB Data project is given in two parts : 1) Getting the datas locally, and 2) Showing the data graphically with a Internet WEB server. This project take the measure and record part of temperatures DATA for solar hot water monitoring. Using an Arduino Yun, the Unix server push the data files with ftp on a Internet WEB server.
A web server make the page, created by PHP and JAVASCRIPT, with functions drawing a day (24 hours) of datas. Sample results can be seen here: https://yvesmasur.ch/ecs/ Organisation of the datas: each month, a new file is created by Arduino Yun, named so: yymmdata.txt, yy= year with 2 digits (eg. 2017 -> 17) mm= Month with 2 digits (eg. april -> 04)
Content: sample each 10 minutes, written on a plain text line in this order: {date} \t {hour:minutes:seconds} \t {percent of pump usage} \t {temp1} \t {temp2}\t {temp3} \n

Details of fields, separated by TAB: date format: yy/mm/dd with yy= year; mm= month; dd= day; (eg. 17/11/20 is the november 11, 2017) time format: hh:mm:ss with hh=hour; mm= minute; ss= second (eg. 05:30:00)
Note: because the sample are made each 10 minutes, mm is alway 00.

Pump usage : 0 to 100 percent of use (eg. if the pump usage is 5 minutes at full time, gives 50 % -> 50)
temp 1 : temperature in degrees of solar cicuit
temp 2 : temperature of solar water volume
temp 3 : temperature of hot water volume
\n : end of line, also CRLF
More details (but in french) on http://microclub.ch/2014/10/11/yun-monitoring-de-leau-solaire/
