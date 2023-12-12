// Example of the different modes of the X.509 validation options
// in the WiFiClientBearSSL object
//
// Jul 2019 by Taejun Kim at www.kist.ac.kr

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "SNIPE.h"

#define ATSerial Serial

SNIPE SNIPE(ATSerial);

// 16byte hex key
String lora_app_key = "10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 00";
int Count = 0;
String gps = "";


//json을 위한 설정
StaticJsonDocument<200> doc;
DeserializationError error;
JsonObject root;

const char *ssid = "";  // 와이파이 이름
const char *pass = "";      // 와이파이 비밀번호
const char *thingId = "";          // 사물 이름 (thing ID) 
const char *host = ""; // AWS IoT Core 주소
const char* outTopic = "outTopic"; //mqtt test
const char* inTopic = "inTopic"; //mqtt test

long lastMsg = 0;
char msg[50];
int value = 0;
String inputString;
boolean stringComplete = false; 
String sIP; //IP Address
String sChipID; // mac address를 문자로 기기를 구분하는 기호로 사용
char cChipID[40];

String ver; // LoRa 통신 수신값

// 사물 인증서 (파일 이름: xxxxxxxxxx-certificate.pem.crt)
const char cert_str[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
// 사물 인증서 프라이빗 키 (파일 이름: xxxxxxxxxx-private.pem.key)
const char key_str[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----

)EOF";
// Amazon Trust Services(ATS) 엔드포인트 CA 인증서 (서버인증 > "RSA 2048비트 키: Amazon Root CA 1" 다운로드)
const char ca_str[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Serial1.print((char)payload[i]);
  }
  Serial.println();
  
  deserializeJson(doc,payload);
  root = doc.as<JsonObject>();
  int value = root["on"];
  Serial.println(value);
  
}

X509List ca(ca_str);
X509List cert(cert_str);
PrivateKey key(key_str);
WiFiClientSecure wifiClient;
PubSubClient client(host, 8883, callback, wifiClient); //set  MQTT port number to 8883 as per //standard

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(thingId)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "hello world");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      wifiClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void setup() {
  // Serial.begin(9600);
  // Serial1.begin(9600);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();

  ATSerial.begin(115200);

  // put your setup code here, to run once:
  while(ATSerial.read()>= 0) {}
  while(!ATSerial);

  //이름 자동으로 생성
  uint8_t chipid[6]="";
  WiFi.macAddress(chipid);
  sprintf(cChipID,"%02x%02x%02x%02x%02x%02x%c",chipid[5], chipid[4], chipid[3], chipid[2], chipid[1], chipid[0],0);
  sChipID=String(cChipID);
  thingId=cChipID;
  Serial.println(thingId);


  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  wifiClient.setTrustAnchors(&ca);
  wifiClient.setClientRSACert(&cert, &key);
  Serial.println("Certifications and key are set");

  setClock();
  //client.setServer(host, 8883);
  client.setCallback(callback);

  client.subscribe("$aws/things/[사물 이름]/shadow/update/delta"); //사물 디바이스 섀도우의 update/delta 구독
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // long now = millis();
  // if (now - lastMsg > 500) {
  //   serialMqttEvent();
  //   lastMsg = now;
  //   ++value;
  //   snprintf (msg, 75, "hello world #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   client.publish(outTopic, msg);
  //   Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
  // }

  ver = SNIPE.lora_recv(); //LoRa 통신으로 전달 받은 값 대입
  if (ver == "IN" && ver != "AT_RX_TIMEOUT") //승객 승차신호
  {
    Count++;
    Serial.println("사람이 승차 하였습니다.");
    Serial.println((String) "현재 탑승 인원: "+ Count);
    SNIPE.lora_send("Ack");
  }
  else if (ver == "OUT" && ver != "AT_RX_TIMEOUT") //승객 하차신호
  {
    Count--;
    Serial.println("사람이 하차 하였습니다.");
    Serial.println((String) "현재 탑승 인원: "+ Count);
    SNIPE.lora_send("Ack");     
  }
  else if(ver != "IN" && ver != "OUT" && ver != "AT_RX_TIMEOUT"){ //일반적으로 GPS값 위도, 경도 수신  
    Serial.println((String) "현재 좌표: "+ ver +", 현재 탑승 인원 " + Count);
    if(ver!="AT_ERROR"){
      char payload[512];
      getDeviceStatus(payload);
      sendMessage(payload);
    }
  }
  else{
    Serial.println("Timeout error");
  }
}


void serialMqttEvent() {
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    //MQTT가 접속됬으면 메세지를 보낸다.
    if (client.connect(thingId)) {
      inputString.toCharArray(msg, inputString.length());
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(outTopic, msg);
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  if(Serial.available() == false) 
    return;
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
  }
  stringComplete = true;
}

void getDeviceStatus(char* payload) {
  // 디바이스 섀도우 update 토픽에 보낼 페이로드 구성
  sprintf(payload,"{\"state\":{\"reported\":{\"GPS\":\"%s\",\"Count\":\"%d\"}}}",ver.c_str(),Count);
}

void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/[사물 이름]/shadow/update"; //[사물 이름]에 해당하는 사물 디바이스 섀도우에 값을 전달하기 위해 토픽 설정
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  client.publish(TOPIC_NAME, payload);  //사물 디바이스 섀도우 update 토픽에 페이로드 전송
}
