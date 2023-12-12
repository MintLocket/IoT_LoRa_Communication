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

const char *ssid = "wsNetwork";  // 와이파이 이름
const char *pass = "01234567";      // 와이파이 비밀번호
const char *thingId = "ESP8266_D1_R1";          // 사물 이름 (thing ID) 
const char *host = "atk03xzexlrj4-ats.iot.ap-northeast-2.amazonaws.com"; // AWS IoT Core 주소
const char* outTopic = "outTopic"; 
const char* inTopic = "inTopic"; 

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
MIIDWjCCAkKgAwIBAgIVAO9kfC/kbHdd4FAEr99ngGUBZgsFMA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMzExMjgwOTM3
NTBaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDUcZiAOHFu69MSU8Sm
vBfpXbCeHyMD5cLY19Si8wNUCxr8MrEFNATCH9nGfWlGtxzOgAV+CLJYKjPRBhg6
p9dOxBignMx9ODjXS0yGtmGa03fh8gxXEfLN4CVgjld3kieE9cQ1b6nelJvqG5Xy
qkKNEkBftKa/onYtXUIRv5QSBN/PQA4VvpErkKoLk2kFyWldAgmZC6aARJoJJtf4
ncJmjiW5Q9NsPT8/fm+0Uy3VOag0UKT0rU+lxHAW2cQXJgCc4SLs+lX9SWjSjOPN
8c16Q138CcYCLjUCq280QKHbOrJYwtIH3op/C83FiX6U6cN1qYEHpPMIZ4wfoSNf
D4YVAgMBAAGjYDBeMB8GA1UdIwQYMBaAFCdyWaOitqJsCDd9y1Sw0DOaYh/BMB0G
A1UdDgQWBBSmFlK+QyjYVvxOtX2hlJ3yz5Nn2DAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAW/XtucraW72pEVzvzJZkA73X
bgeQzFX97IodINzXGn3SK7VXzrlLTT1XtcjGWUy5tAedqRZKepjPAzKQntk6HXi9
Lfu8NeiOd4GGmPsilPWEX3SGbfrsD3enYvz2GkVFwITOzPT3CQFdQi1xgwW4iYZ8
hjDYu9JWmEPGqD+khr9vFuBbXOgjqeJtTk8FyxTl8agsnFemtizprcHmp2LsfFAY
+3OA70j8KE48n+jDpUIi7dqtOiUtsn4qPVUZlJxJ2/3+NRJ9GTZgnoM+vVS12z+t
7nzN/QGoG4YG8J8slJi9n9XOA82MQeiWiYBgLw7iWcdtNUHr240VEJw7yFgRpQ==
-----END CERTIFICATE-----
)EOF";
// 사물 인증서 프라이빗 키 (파일 이름: xxxxxxxxxx-private.pem.key)
const char key_str[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEA1HGYgDhxbuvTElPEprwX6V2wnh8jA+XC2NfUovMDVAsa/DKx
BTQEwh/Zxn1pRrcczoAFfgiyWCoz0QYYOqfXTsQYoJzMfTg410tMhrZhmtN34fIM
VxHyzeAlYI5Xd5InhPXENW+p3pSb6huV8qpCjRJAX7Smv6J2LV1CEb+UEgTfz0AO
Fb6RK5CqC5NpBclpXQIJmQumgESaCSbX+J3CZo4luUPTbD0/P35vtFMt1TmoNFCk
9K1PpcRwFtnEFyYAnOEi7PpV/Ulo0ozjzfHNekNd/AnGAi41AqtvNECh2zqyWMLS
B96KfwvNxYl+lOnDdamBB6TzCGeMH6EjXw+GFQIDAQABAoIBAQCi0X0DqD6nuG3I
ehMZhsb8wCWAynRXpFw1PRVm3cELRIemCmPJjRiofyi1C6EH5YdabJvJAHlPQOj9
piIjskc9wuTtE79VqSVHPBqmlFkyBCi5Ln1urf7c8pEy82Tt60IGkSWTnsUeJd9p
CUNe/EaNQBO+RgIWlTjlg7gDSkT9mQCg30aY+YffALLLChYyGHr4lAsYHitvV2yH
FiSlFIMzanZorgKfXpTVi7LNQf8AcpiukvrwwVLybKZRSlSfGq/YFRbLUniVRlLS
MwYSSeqTKt+UixDwbu27UIMAOTy5xbHmmNtsNBw6PDB9GLZujLJdSEhjPx/4uJhq
sX0HJDlxAoGBAPeyX8E7qHyBpc8lDma8Rwt5la8HDxwrMj+MHsvcSl/okZus1NJT
x7U+ilLTkFRAk5BZ3qWcnsF4mUYdWTbU/cZUILQuTCqLPHsV5K4mpF7ObSUGTGel
EGeDmDRNpfLIvYErGJcXEkoxuk9+iIK78ABERDlqr1DUDGQd+3XVbqR3AoGBANuQ
sgY63OQJtpLVaQ1Qy05eUkKS6vgRabZrz6h7sLxMIbdFeXIs5rTbS8Z5DFgHa28V
XZURVTfmWJpWi5eBe5h6gv3mh9J6UBVDxd8QJy6MabqlEzsy2tH9kLNSwZkPLIrr
ra+MjqOtRyneVKeSQehT+pszzj/1zkasErWL58jTAoGAPJ2UPU+CLeX0U7r3yUBu
CmrYrJqHohcYFuENcEoakVnyuCwVcNs71ijV5OsNOPEMq5iymci4ORRwPe5UtMxu
xig0UKO70/V6+YwkCFJAi0rs/tCJxhto5kwX5UTTRZTGe2O2gtMMkMeWlnmaopOn
aglKwNpQyPD7j0yfCvkOOIUCgYEA0ZaSyW2g87Dt7cX3jLVmDL3uqgwSbZA5p9/M
DtlbgLLvsEGKR1njNBrtf1yHUvRqMlRjy2sLPNwOEe2xPINmVswDhJhtaJZHIbVb
Ca4iqMVyG6995xGJXdA9M2A424FnlIiaeCuk8Urz1lyb1IXBkiOv4j0WkWlzaXTS
Oyh8re8CgYEAiuH43sNrGWgBvlLQBmumulEch/0K78MO98vQbIJPEXeSNOWvAEHV
GiRncS6AVGil7tn5tQdZu/TdbmoIo7fuqFuaamzEj4FKv1a8a59KUq4KNb+zycUx
yt4qcbBS51J96sdbpmkNLEUAos0yxSYoBgcvAGJFdJFAIPW/4U0k5/U=
-----END RSA PRIVATE KEY-----

)EOF";
// Amazon Trust Services(ATS) 엔드포인트 CA 인증서 (서버인증 > "RSA 2048비트 키: Amazon Root CA 1" 다운로드)
const char ca_str[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
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

  client.subscribe("$aws/things/ESP8266_D1_R1/shadow/update/delta"); //사물 디바이스 섀도우의 update/delta 구독
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

  ver = SNIPE.lora_recv();
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
  char TOPIC_NAME[]= "$aws/things/ESP8266_D1_R1/shadow/update";
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  client.publish(TOPIC_NAME, payload);  //사물 디바이스 섀도우 update 토픽에 페이로드 전송
}
