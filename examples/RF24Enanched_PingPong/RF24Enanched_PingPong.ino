/**
  Example of how to use RF24-Enanched libraries with Ethernet Shield

  Upload this sketch to two different nodes and put one them into 'transmit' sending 'T' on serial monitor. 
  The ping node sends the current time to the pong node, which responds by sending the value back. 
  The web server shows the last known exchanged value

  Note:
  - In order to let nRF24 and Ethernet modules work togheter you should define SOFTSPI as compiler flag (-DSOFTSPI)

  Pin configuration:
  IMPORTANT: insert capacitor 100mF across nRF24L01 pin 1 and 2 

  nRF24L01      UNO     LEONARDO
  * GND  1        GND     GND
  * VCC  2        3.3v    3.3v
  * CE   3        7       7
  * CSN  4        8       8
  * SCK  5        13      ICSP_3
  * MOSI 6        11      ICSP_4
  * MISO 7        12      ICSP_1

  If SOFTSPI is defined emulated SPI is used with the following PIN configuration

  nRF24L01      UNO     
  * GND  1        GND   
  * VCC  2        3.3v   
  * CE   3        7      
  * CSN  4        8      
  * SCK  5        14 (A0)    
  * MOSI 6        15 (A1)  
  * MISO 7        16 (A2)


  Libraries RF24-Enanched is required

*/

 

  #include <SPI.h>

  #ifdef SOFTSPI
    #include <DigitalIO.h>
    #include <DigitalPin.h>
    #include <I2cConstants.h>
    #include <PinIO.h>
    #include <SoftI2cMaster.h>
    #include <SoftSPI.h>
  #endif //SOFTSPI

  #include <RF24.h>
  #include <RF24_config.h>
  #include <nRF24L01.h>

  #include <Ethernet.h>

  #include "printf.h"


// ######### Setup nRF24L01 radio ##############
// use SoftSPI bus plus pins 7 & 8 
RF24 radio(7,8);
byte addresses[][6] = {"1Node","2Node"};

// Set up roles to simplify testing 
boolean role;                                    // The main role variable, holds the current role identifier
boolean role_ping_out = 1, role_pong_back = 0;   // The two different roles.


// ########### Setup Ethernet ##################
//set mac address
byte mac[] = { 
  0xDE, 0xAD, 0x00, 0xCC, 0xFE, 0xAA };
//set port
EthernetServer server(80);
// set IP
IPAddress ip(192,168,10,177);

String latestMessage = "N/A";

/**
  SETUP
*/
void setup() {

  Serial.begin(57600);
#if defined(__AVR_ATmega32U4__)
  while(!Serial){}
#endif  
  printf_begin();
#if defined(__AVR_ATmega32U4__)
  printf("LEONARDO")
#endif
  printf("\n\rRF24 Setup/\n\r");
  printf("*** PRESS 'T' to begin transmitting\n\r*** PRESS 'R' to begin receiving\n\r");
  
  // Setup and configure rf radio
  radio.begin();                          // Start up the radio
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  
  radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  Ethernet.begin(mac,ip);
  server.begin();
  Serial.println("\n\rEthernet Setup");
  Serial.print("Server is listeining on ");
  Serial.println(Ethernet.localIP());
  
}

/**
  main loop
*/
void loop(void){
  if (role == role_ping_out)  {
    ping(1000);
  }

  if ( role == role_pong_back )
  {
    pong();
  }


  if ( Serial.available() )
  {
      readCommandFromSerial();
  }
   
  webserverLoop(latestMessage);   
 }

 /**
    Ping Out Role
 */
 void ping(long waitDelay){

    // stop listening to talk
    radio.stopListening();                                     
    
    unsigned long time = micros();
    latestMessage = String(time);          
    printf("Now sending [%lu] \n\r",time);
    // Take the time, and send it.  This will block until complete
    if (!radio.write( &time, sizeof(unsigned long) )){  printf("failed.\n\r");  }
    // restart listening
    radio.startListening();                                    
    
    unsigned long started_waiting_at = micros();               
    boolean timeout = false;                                   
    
    // while nothing is received or timeout (200ms)
    while ( ! radio.available() ){                             
      if (micros() - started_waiting_at > 200000 ){            
        timeout = true;
        break;
      }      
    }

    if ( timeout ){                                             
      printf("Failed, response timed out.\n\r");
      }else{
        // Grab and compare the response 
        unsigned long got_time;                                 
        radio.read( &got_time, sizeof(unsigned long) );

        
        printf("Sent %lu, Got response %lu, round-trip delay: %lu microseconds\n\r",time,got_time,micros()-got_time);
      }

    
    delay(waitDelay);

 }
 
/**
 Pong Back Role
*/
 void pong()
 {
    if( radio.available()){
      unsigned long got_time;                                       
      while (radio.available()) {                                   
        // Get the payload
        radio.read( &got_time, sizeof(unsigned long) );             
      }    
      // stop listening to talk  
      radio.stopListening();                                         
      latestMessage = String(got_time);
      // send back.     
      radio.write( &got_time, sizeof(unsigned long) );               
      radio.startListening();                                       // Now, resume listening so we catch the next packets.     
      printf("Sent response %lu \n\r", got_time);  
    }

 }

 /**
    Change Roles via Serial Commands
 */
 void readCommandFromSerial(){

    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      role = role_ping_out;                  // Become the primary transmitter (ping out)
      radio.openWritingPipe(addresses[0]);
      radio.openReadingPipe(1,addresses[1]);

    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");
      
       role = role_pong_back;                // Become the primary receiver (pong back)
       radio.openWritingPipe(addresses[1]);
       radio.openReadingPipe(1,addresses[0]);
     }
 }

 void webserverLoop(String latestMessage){

    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title>Arduino Web Page</title>");
            client.println("</head>");
            client.println("<body>");
            client.print("<h1>The last message we got was: ");
            client.print(latestMessage);      
            client.println("</h1>");
            client.println("</body>");
            client.println("</html>");
            break;
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } 
          else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
      // give the web browser time to receive the data
      delay(1);
      // close the connection:
      client.stop();
      Serial.println("client disonnected");
    }
  }

