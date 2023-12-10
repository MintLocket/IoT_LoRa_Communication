#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "SNIPE.h"

#define GPS_TX 4
#define GPS_RX 3

#define ATSerial Serial

String lora_app_key = "10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 00";  //16byte hex key

SNIPE SNIPE(ATSerial);
TinyGPS gps;
SoftwareSerial ss(GPS_RX, GPS_TX);

void setup() {
  ATSerial.begin(115200);

  // put your setup code here, to run once:
  while (ATSerial.read() >= 0) {}
  while (!ATSerial);

  ss.begin(9600);

  if (!SNIPE.lora_init()) {
    Serial.println("SNIPE LoRa Initialization Fail!");
    while (1);
  }

}

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    char gps_buf[32];
    char gps_lat[12];
    char gps_lon[12];
    String gps_value;
    memset(gps_buf, 0x0, sizeof(gps_buf));

    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
 
    Serial.println(gps_value);

    if (flat != TinyGPS::GPS_INVALID_F_ANGLE &&
      flon != TinyGPS::GPS_INVALID_F_ANGLE){
        dtostrf(flat, 9, 6, gps_lat);
        dtostrf(flon, 10, 6, gps_lon);
        sprintf(gps_buf, "%s %s", gps_lat, gps_lon);
        gps_value = (String)gps_buf;
        
        Serial.println("-----gps_send_value-----");
        Serial.println(gps_value);

        //LoRa Send : GPS value
        if (SNIPE.lora_send(gps_value))
        {
          Serial.println("send success");
        }
        else
        {
          Serial.println("send fail");
        }
    } 
  }

  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");
  //SNIPE.lora_send((String)chars);
}
