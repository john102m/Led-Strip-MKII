
/*
   MKV  has no webserver on it    11/12/2020

*/

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <WebSocketsServer.h>

//#include <ESP8266HTTPClient.h>

#include "LedWrite.h"
#define USE_SERIAL Serial

const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
NTPClient timeClient(ntpUDP);


const int LED_DATA_PIN = 2;
WebSocketsServer webSocket = WebSocketsServer(81);
LedWrite leds = LedWrite(LED_DATA_PIN);
int COLOR_COUNT = leds.colorCount;

IPAddress ip(192, 168, 1, 220); // where xx is the desired IP Address
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask
IPAddress dns(1, 1, 1, 1);

const char *ssid = "BT-6XCPXM";
const char *password = "e4nYhm4u9ecU4v";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int prevMillis; 
int timeMillis; 
int DELAY_MILLISECONDS = 400;
int MODE_SET_COUNTER = 0;
bool MODE_CHANGE = false; 
bool SHUFFLE_MODE = false;
bool RANDOM_MODE = false;

byte clientCount = 0;
bool CN_Flag = false;  // this flag is set on new websocket connection
int SELECTED_MODE; //  selection of current sequence  (start on  white colour changing )

/*
   randColor1  and  randColor2  these globals store 2 colours
   they are used for the twinkle , colour phasing , swipe and LED chase effect methods of the LedWrite class
*/
byte randColor1, randColor2;
int chase_dir = 1;

bool PLAY_FLAG = true;

// setFlag is used by any part of the program when a physical LED refresh is required
bool SET_FLAG = false;

// it is possible to access the LED data array
//RGB* led = leds.led;

//this is a pointer to an array
RGB* color = leds.getColor();

struct EventType {
  char *name;
  char *time;
};

EventType Event[9];  


//////////////////////////////////////////////////////////////////////////////////
//  this method is called when there is text from the web socket connection   ////
//////////////////////////////////////////////////////////////////////////////////
void getTextData(String text)
{
  String msg = text.substring(0, 1);
  RANDOM_MODE = false; SHUFFLE_MODE = false;
  USE_SERIAL.print("Data: " + text.substring(3) + "\n");

  if (msg == "B") {       //if the message had the word 'Button'   etc.......

    MODE_CHANGE = true;

    int btnNumber = text.substring(1).toInt();            // button number pressed

    //USE_SERIAL.print("Data: " + text + "\n");
    
    switch (btnNumber)
    {
      case 7:        
          SELECTED_MODE -= 1;
          if (SELECTED_MODE < 0) SELECTED_MODE = 18;
          break;
        
      case 8:       
          SELECTED_MODE += 1;
          if (SELECTED_MODE > 18) SELECTED_MODE = 0;
          break;
        
      case 11:
          SHUFFLE_MODE = true;
          break;
        
      case 12:
          RANDOM_MODE = true;
          break;
        
      case 13:
          DELAY_MILLISECONDS += 50;
          if (DELAY_MILLISECONDS > 10000)
          {
            DELAY_MILLISECONDS = 10000;
          }
          break;
          
      case 14:
          DELAY_MILLISECONDS -= 50;
          if (DELAY_MILLISECONDS < 10)
          {
            DELAY_MILLISECONDS = 10;
          }
          break;
        
      default:       
          SELECTED_MODE = btnNumber;

    }
    String modeString = "Mode: " + String(SELECTED_MODE);
    if (RANDOM_MODE) modeString = "Random";
    if (SHUFFLE_MODE) modeString = "Shuffle";
    webSocket.broadcastTXT(modeString);
  }
}
//////////////////////////////////////////////////////////////////////////
//       this method occurs when there is a web socket connection     ////
//////////////////////////////////////////////////////////////////////////
void doConnectedThings(uint8_t num)
{
  IPAddress ip = webSocket.remoteIP(num);
  //PLAY_FLAG = true;

  USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d \n", num, ip[0], ip[1], ip[2], ip[3]);
  CN_Flag = true;
  clientCount = num;

  //leds.setColor(color[indigo]);
  //SELECTED_MODE = 9;
  SET_FLAG = true;
  
}
//==========================================================================================================================================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type)
  {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      CN_Flag = false;
      break;

    case WStype_CONNECTED:
      doConnectedThings(num);
      break;

    case WStype_TEXT:
      getTextData((char *)&payload[0]);
      break;

    case WStype_BIN:
      hexdump(payload, length);
      break;
  }
}

//**************************************************************************************************************************************************************
void doSequence()    //  using choice GLOBAL variable  this updates or advances the current sequence
{

  switch (SELECTED_MODE) 
  {
    case 0: 
      leds.colorChange(color[white]);
      break;
    case 1: 
      leds.colorChange(color[red]);
      break;
    case 2: 
      leds.colorChange(color[green]);
      break;
    case 3: 
      leds.colorChange(color[blue]);
      break;
    case 4: 
      leds.colorChange(color[yellow]);
      break;
    case 5:   
      leds.colorChange(color[indigo]);
      break;
    case 6: 
      leds.colorChange(color[violet]);
      break;
    case 7:    
        leds.colorChange(color[white]);
        leds.chaseEffect(chase_dir, randColor1);    
      break;
    case 8 ... 11: 
      leds.shift(chase_dir);    
      break;
    case 12: 
      leds.twinkle(color[yellow], color[blue]);         //yellow on blue
      break;
    case 13: 
      leds.twinkle(color[randColor1], color[randColor2]);
      break;
    case 14: 
      leds.shift(chase_dir);     //  rainbow
      break;
    case 15:     
      leds.twinkle(color[random(13)], color[none]);//   super strobe   ****************
      break;
    case 16: 
      leds.phasing(color[randColor1], color[randColor2]);
      break;
    case 17: 
      leds.snowfall(4);
      break;
    case 18: 
      leds.swipe(color[randColor1], color[randColor2]);
      break;
      
    case 19: 
    default:
      leds.setColor(color[none]);
        
  }
  SET_FLAG = true;
}

//////////////////////////////////////////////////////////
//          Mode eight is two coloured chaser          //
////////////////////////////////////////////////////////
void modeEight()
{
  // pick two random colours
  randColor1 = random(7); randColor2 = random(7);

  // set one of them white if they are the same
  if (randColor1 == randColor2) randColor1 = white;

  //  fill LED array with two alternate colors ,  random spacing
  leds.fillAlternate(random(4, 8), color[randColor1], color[randColor2]);       //  0 to 7 are rainbow  colours   32     params: mod, color 1, color 2

}
//////////////////////////////////////////////////////////
//          Mode nine is sine wave effect         //
////////////////////////////////////////////////////////
void modeNine()
{
  randColor1 = random(7);
  // sine wave effect is actually modified to resemble a softened triangle using the 3rd harmonic of a cosine wave
  leds.wave(randColor1);
}

//////////////////////////////////////////////////////////////////////////////
//          every now and then a sequence will be selected                  //
//    when a sequence is selected the LED data array is filled with         //
//           data to determine  the color and brightness of each LED        //
//////////////////////////////////////////////////////////////////////////////
void setSequence()   //  uses the above modes
{
  chase_dir = random(2);
  switch (SELECTED_MODE) {
    case 7:
      randColor1 = random(7);
      break;

    case 8:
      modeEight(); //Mode eight is a two coloured chaser // fill alternate
      break;

    case 9:
      modeNine();// sine wave effect     actually modified to resemble a softened triangle using the 3rd harmonic of a cosine wave
      break;

    case 10 ... 11:
      leds.fillRandom();
      break;
      
    case 12:
    case 15:
      leds.spread = random(0, 2);  //twinkle   //yellow on blue
      break;

    case 13:      // twinkle random     
      randColor1 = random(7); randColor2 = random(7);
      leds.spread = random(0, 2);
      break;
      
    case 14:
      leds.fillRainbow();
      break;

    case 17:    //snow
      leds.setColor(color[none]);
      break;
      
    case 16:
    case 18:  //swipe
      randColor1 = random(7); randColor2 = random(7);
      break;
    case 19:
      leds.setColor(color[none]);
      break;
      
  }
}

//**************************************************************************************************************************************************************
void PINsetup()
{
  pinMode(LED_DATA_PIN, OUTPUT);
  digitalWrite(LED_DATA_PIN, HIGH);    // that's the RX pin  pinMode(LED_PIN, OUTPUT);  //gpio 2

}

//**************************************************************************************************************************************************************
void setup() 
{
    USE_SERIAL.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
    PINsetup();
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) 
    {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    //WiFi.config(ip, gateway, subnet, dns);
    WiFi.begin(ssid, password);
    //WiFi.config(ip);
    //WiFiMulti.addAP(ssid, password);

    //while(WiFiMulti.run() != WL_CONNECTED) 
    while ( WiFi.status() != WL_CONNECTED ) 
    {
        delay(100);
        Serial.print(".");
    }  

    USE_SERIAL.println("");
    USE_SERIAL.print("Connected to ");
    USE_SERIAL.println(ssid);
    USE_SERIAL.print("IP address: ");
    USE_SERIAL.println(WiFi.localIP());
  
    // Start TCP (WS) server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
  
    clientCount = 0;
    leds.setColor(color[none]);
  
    SET_FLAG = true;
    leds.dimColors();
  
    prevMillis = millis();  
    timeMillis = millis();

    timeClient.begin();

    setEvents();
 
}

void setEvents()
{
    Event[0].name = "off";
    Event[0].time = "00:00";

    Event[1].name = "mode 0";
    Event[1].time = "08:00";

    Event[2].name = "mode 9";
    Event[2].time = "17:00";

    Event[3].name = "off";
    Event[3].time = "17:05";

    Event[4].name = "mode 0";
    Event[4].time = "17:10";
    
    ////////////  test events
    Event[5].name = "mode 11";
    Event[5].time = "21:30";
    
    Event[6].name = "mode 9";
    Event[6].time = "21:25";
    
    Event[7].name = "off";
    Event[7].time = "21:40";
    
    Event[8].name = "mode 0";
    Event[8].time = "21:55";
}

//******************************************************************************************************************************************************************8
void doTime()
{
  timeClient.update();
  String timeStr = timeClient.getFormattedTime();
  Serial.println(timeStr.substring(0, 5)); 
  webSocket.broadcastTXT("Connected. Mode: " + String(SELECTED_MODE)); 
  
  // check for events every minute
  for (int i = 0; i < 9; i++)
  {   
    // event time matches current time
    if (timeStr.substring(0, 5) == Event[i].time)
    {
      Serial.print("Activating event: "), 
      Serial.print(Event[i].name);
      Serial.print(" at ");
      Serial.print(Event[i].time);  

      // perform event task
      if (Event[i].name == "off")
      {
        lightsOff(timeStr.substring(0, 5));  // pass in hours and minutes
      }    
      else
      {
        String temp = Event[i].name;
        setMode(temp.substring(5), timeStr.substring(0, 5));
                    
      }                 
    }     
  }
}

//////////////////////////////////////////////////////////////////////
void lightsOff(String timeStr)
{
  Serial.println(" Lights Off"); 
  webSocket.broadcastTXT("[Event] Lights off at: " + timeStr);
  PLAY_FLAG = false;
  leds.setColor(color[none]);
  leds.refresh();
}
//////////////////////////////////////////////////////////////////////////
void setMode(String mode, String timeStr)
{
  Serial.println(" MODE to set is: " + mode);
  getTextData("B" + mode);
  webSocket.broadcastTXT("[Event] Mode " + mode + " set at: " + timeStr); 
  PLAY_FLAG = true; 
}

/////////////////////////////////////////////////////////////////////////
void loop() 
{
  webSocket.loop();// service the websocket

  if (millis() > timeMillis + 60000)   // 1000 = one second 60000 = 1 minute
  {
    timeMillis = millis();
    doTime();
  }
  
  // one tenth of a second events - stuff happens with a kind of semaphore system - this isn't semaphore 
  if (millis() > prevMillis + DELAY_MILLISECONDS)   // 100 = 0.1 second
  {
    prevMillis = millis();
    if (RANDOM_MODE || SHUFFLE_MODE) // if random mode is selected then increment the counter
    { 
      MODE_SET_COUNTER++;
      
      // if the counter times out then a MODE CHANGE will be triggered and the counter will be reset
      if (MODE_SET_COUNTER > 50)  // 50 gives 5 seconds so the counter times out and a MODE change will happen
      {
        MODE_SET_COUNTER = 0;
        if (!SHUFFLE_MODE) SELECTED_MODE = random(7, 18);  //  MODE change may only shuffle the mode with new random values
        MODE_CHANGE = true;
      }
    }

    delay(1);
    if (MODE_CHANGE) 
    {
      MODE_CHANGE = false;
      setSequence();
    } 
    else if (PLAY_FLAG) 
    {
      doSequence();
    }

    delay(1);
    if (SET_FLAG) 
    {  // SET_FLAG can be used by any part of the program when a physical LED refresh is required
      SET_FLAG = false;
      leds.refresh();
    }
  }//end of one nth of a second events

}
