#include <SoftwareSerial.h>
#include "SNIPE.h"

#define ATSerial Serial

SNIPE SNIPE(ATSerial);

//16byte hex key
String lora_app_key = "10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 00";
int Count = 0;

void setup() {
  ATSerial.begin(115200);

  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);
}

void loop() {
        String ver = SNIPE.lora_recv();
        if (ver == "IN" && ver != "AT_RX_TIMEOUT")
        {
          Count++;
          Serial.println("사람이 승차 하였습니다.");
          Serial.println((String) "현재 탑승 인원: "+ Count);
          SNIPE.lora_send("Ack");
        }
        else if (ver == "OUT" && ver != "AT_RX_TIMEOUT")
        {
          Count--;
          Serial.println("사람이 하차 하였습니다.");
          Serial.println((String) "현재 탑승 인원: "+ Count);
          SNIPE.lora_send("Ack");     
        }
        else if(ver != "IN" && ver != "OUT" && ver != "AT_RX_TIMEOUT"){   
          Serial.println((String) "현재 좌표: "+ ver +", 현재 탑승 인원 " + Count);          
        }
        else{
          Serial.println("Timeout error");
        }

}
