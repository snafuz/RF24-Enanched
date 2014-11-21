# Optimized High Speed Arduino driver for nRF24L01 2.4GHz with SoftSPI

 This projetc merges the Optimized HighSpeed driver for nRF24L01 by TMRh20 (https://github.com/TMRh20/RF24/)
 with SoftSPI injection by shnae (https://github.com/shnae/rf24_plus_softSPI) 
 
  
Design Goals: This library is designed to be...  
  
	* More compliant with the chip's manufacturer specified operation
	* Use full potential hw capabilities via Arduino
	* More reliable and feature rich  
	* Easy for beginners to use, with well documented examples and features 
	* Consumed with a public interface that's similiar to other Arduino standard libraries  
	* Built against the standard SPI and SoftSPI library

  
This drive uses the SPI bus, plus two chip control pins.
If you want to use SoftSPI (for example because there are other SPI device connected) you have to 
define SOFTSPI as compiler flag (-DSOFTSPI).

PIN Configuration

with standard SPI
  nRF24L01      UNO     LEONARDO
  * GND  1        GND     GND
  * VCC  2        3.3v    3.3v
  * CE   3        7       7
  * CSN  4        8       8
  * SCK  5        13      ICSP_3
  * MOSI 6        11      ICSP_4
  * MISO 7        12      ICSP_1

 with SoftSPI
  nRF24L01      UNO     
  * GND  1        GND   
  * VCC  2        3.3v   
  * CE   3        7      
  * CSN  4        8      
  * SCK  5        14 (A0)    
  * MOSI 6        15 (A1)  
  * MISO 7        16 (A2)



IMPORTANT NOTE:
 - Remember that pin 10 must still remain an output or the SPI hardware will go into 'slave' mode.  
 - Insert capacitor 100mF across nRF24L01 pin 1 and 2 if you have connection problem
  
Supported Boards:  
  
	* Uno, Nano, Leonardo etc (328 based boards)  
	* Mega Types (2560, 1280, etc)  
	* ARM (Arduino Due) via extended SPI methods 
	* ATTiny 24/44/84 25/45/85  
	* Raspberry Pi

See the documentation for more info:
	- http://tmrh20.github.io/