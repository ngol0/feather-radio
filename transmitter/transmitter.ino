// Feather9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_GPS.h>

/*for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7


/* for feather m0  
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
/*

/* for shield 
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 7
*/

/* Feather 32u4 w/wing
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     2    // "SDA" (only SDA/SCL/RX/TX have IRQ!)
*/

/* Feather m0 w/wing 
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
*/

#if defined(ESP8266)
  /* for ESP w/featherwing */ 
  #define RFM95_CS  2    // "E"
  #define RFM95_RST 16   // "D"
  #define RFM95_INT 15   // "B"

#elif defined(ESP32)  
  /* ESP32 feather w/wing */
  #define RFM95_RST     27   // "A"
  #define RFM95_CS      33   // "B"
  #define RFM95_INT     12   //  next to A

#elif defined(NRF52)  
  /* nRF52832 feather w/wing */
  #define RFM95_RST     7   // "A"
  #define RFM95_CS      11   // "B"
  #define RFM95_INT     31   // "C"
  
#elif defined(TEENSYDUINO)
  /* Teensy 3.x w/wing */
  #define RFM95_RST     9   // "A"
  #define RFM95_CS      10   // "B"
  #define RFM95_INT     4    // "C"
#endif

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// what's the name of the hardware serial port?
#define GPSSerial Serial1

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

uint32_t timer = millis();

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);

  delay(100);

  Serial.println("Feather LoRa TX Test!");
  Serial.println("Adafruit GPS library basic test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  delay(1000);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);
}

//int16_t packetnum = 0;  // packet counter, we increment per xmission


void loop()
{
  //- FOR GPS READ-OUT - 
  char c = GPS.read();
  if (GPSECHO)
    if (c) Serial.print(c);

 if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    Serial.print("\nTime: ");

    //Change the timezone and date
      //If hour < 6am, change hour to night time and switch back to one day
    if (GPS.hour < 6)
    {
      float ((GPS.hour) = 24 - (6 - GPS.hour));
      float ((GPS.day) = GPS.day - 1);
      //On the 1st day of the month, day is set to 1-1 = 0
      //thus, set back to the last day of previous month
      if (GPS.day == 0)
      {
        //Month that has 31st days
        //If month is Feb -> Jan
        //Apr -> Mar
        //Jun -> May
        //Aug -> Jul
        //Sep -> Aug
        //Nov -> Oct
        if (GPS.month == 2| GPS.month == 4| GPS.month == 6| GPS.month == 8| GPS.month == 9| GPS.month == 11)
        {
          float ((GPS.day) = 31);
          float ((GPS.month) = GPS.month - 1);
        }
        //Jan -> Dec - also has 31st days
        else if (GPS.month == 1)
        {
          float((GPS.day) = 31);
          float((GPS.month) = 12);
        }
        //If month is March, set back to Feb
        //If year is leap year, Feb has 29 days
        //Otherwise, Feb has 28 days
        else if (GPS.month == 3)
        {
          if (GPS.year%4 == 0)
          {
            float ((GPS.day) = 29);
            float ((GPS.month) = GPS.month - 1);
          }
          else
          {
            float ((GPS.day) = 28);
            float ((GPS.month) = GPS.month - 1);
          }
        }
        //Month that has 30th days
        //May -> Apr
        //Jul -> Jun
        //Oct -> Sep
        //Dec -> Nov
        else
        {
          float ((GPS.day) = 30);
          float ((GPS.month) = GPS.month - 1);
        }
      }
    }
    //If hour >= 6am, substract from 6 and date not changed
    else
    {
      float ((GPS.hour) = GPS.hour - 6);
    }
       
    if (GPS.hour < 10) { Serial.print('0'); }
    Serial.print(GPS.hour, DEC); Serial.print(':');
    if (GPS.minute < 10) { Serial.print('0'); }
    Serial.print(GPS.minute, DEC); Serial.print(':');
    if (GPS.seconds < 10) { Serial.print('0'); }
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    if (GPS.milliseconds < 10) {
      Serial.print("00");
    } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
      Serial.print("0");
    }
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);

      //- FOR TRANSMITTING DATA -
      //Convert location float type to string type
      const float lat = GPS.latitude;
      const float lon = GPS.longitude;
      char bufflat[10];
      char bufflon[10];
      dtostrf(lat, 4, 4, bufflat);
      dtostrf(lon, 4, 4, bufflon);

      //Convert date float to string
      const float day = GPS.day;
      const float month = GPS.month;
      char buffday[10];
      char buffmon[10];
      dtostrf(day, 2, 0, buffday);
      dtostrf(month, 2, 0, buffmon);

      //Covert time float to string
      const float hr = GPS.hour;
      const float mins = GPS.minute;
      const float sec = GPS.seconds;
      char buffhr[10];
      char buffmins[10];
      char buffsec[10];
      dtostrf(hr, 2, 0, buffhr);
      dtostrf(mins, 2, 0, buffmins);
      dtostrf(sec, 2, 0, buffsec);

      //Send latitude and longitude
      //Send lat
      char latPack[10] = "Lat N: ";
      char buffer1[256];
      strncpy(buffer1, latPack, sizeof(buffer1));
      strncat(buffer1, bufflat, sizeof(buffer1));
      rf95.send((uint8_t*)buffer1, 20);

      //Send long
      char lonPack[10] = "Long W: ";
      char buffer2[256];
      strncpy(buffer2, lonPack, sizeof(buffer2));
      strncat(buffer2, bufflon, sizeof(buffer2));
      rf95.send((uint8_t*)buffer2, 20);

      //Send date and time
       //Add time
      char buffer3[256];
      strncpy(buffer3, buffhr, sizeof(buffer3));
      strncat(buffer3, ":", sizeof(buffer3));
      strncat(buffer3, buffmins, sizeof(buffer3));
      strncat(buffer3, ":", sizeof(buffer3));
      strncat(buffer3, buffsec, sizeof(buffer3));

      //Add date
      strncat(buffer3, ", ", sizeof(buffer3));
      strncat(buffer3, buffmon, sizeof(buffer3));
      strncat(buffer3, "/", sizeof(buffer3));
      strncat(buffer3, buffday, sizeof(buffer3));

       //Sending
      rf95.send((uint8_t*)buffer3, 20);

      rf95.waitPacketSent();
    }
  }
}
