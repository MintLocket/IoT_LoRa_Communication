# IoT_LoRa_Communication

한성대 IoT 통신시스템 팀 프로젝트 - 버스 위치/혼잡도 기능에 사용된 디바이스간 LoRa 통신 코드입니다. 

해당 프로젝트에 사용된 디바이스 및 센서는 다음과 같습니다.

WeMos D1 R1 Board - 1

Arduino Uno Board - 2

codezoo LoRa Shield - 3

NEO 6M GPS Sensor - 1

RC522 RFID Sensor - 1

해당 코드를 사용하기 위해 SNIPE 라이브러리를 등록해야합니다.

SNIPE: https://github.com/codezoo-ltd/SNIPE

## arduino-aws

센세 디바이스에서 전송한 데이터를 수신받고,

WiFi를 이용해 AWS와 통신, AWS IoT Core에 등록된 사물의 디바이스 섀도우에 데이터를 업데이트 하는 코드입니다.

사용을 위해 pubsubclient 라이브러리가 필요합니다.

pubsubclient: https://github.com/knolleary/pubsubclient

해당 기능을 수행하는 디바이스 모듈은 LOLIN(WeMos) D1 R1 모듈이며, 다음 URL을 추가 보드 관리자 URL에 등록해야합니다.

https://arduino.esp8266.com/stable/package_esp8266com_index.json

### 기능

기본적으로 GPS 정보를 수신받습니다.

RFID 디바이스로부터 승/하차 신호를 수신하면 승객의 카운트를 조절합니다.

수신받은 GPS 정보와 승객 수를 다음 형식의 페이로드로 구성하여 AWS로 전송합니다.

{\"state\":{\"reported\":{\"GPS\":\"%s\","Count\":\"%d\"}}}

## LoRa_GPS_Send

LoRa와 GPS를 이용한 통신 코드입니다.

SNIPE 라이브러리의 LoRa_GPS_Send 예제를 사용하였습니다.

예제 사용을 위해 TinyGPS 라이브러리를 등록해야합니다.

TinyGPS: https://github.com/mikalhart/TinyGPS

### 기능

GPS 센서로 위치 정보를 입력 받아 "위도 경도" 형식의 문자열 형태로 수신 디바이스에 데이터를 전송합니다.

### 회로

센서는 NEO-6M 모듈을 사용하였습니다.

VCC: 5v

RX: pin 4

TX: pin 3

GND: GND

## LoRa_RFID_Send
LoRa와 RFID를 이용한 통신 코드입니다.

해당 프로젝트의 기능으로 버스 내부 혼잡도를 확인하는 것을 구현하기 위해 RFID를 사용하였습니다.

RFID 정보는 연결 리스트를 이용하여 등록, 삭제가 이뤄집니다.

해당 코드는 MFRC522 라이브러리의 ReadNUID 예제를 활용하였습니다. 따라서 MFRC522 라이브러리를 등록해야합니다.

MFRC522: https://github.com/miguelbalboa/rfid

### 기능

RFID 카드로 데이터를 입력받습니다.

입력 받은 데이터가 리스트에 없는 경우(승차) 리스트에 UID를 저장하고, 수신 디바이스에게 승객이 승차했다는 신호를 보냅니다 ("IN")

입력 받은 데이터가 리스트에 있는 경우(하차) 리스트에 UID를 저장하고, 수신 디바이스에게 승객이 하차했다는 신호를 보냅니다 ("OUT")

### 회로

다음 블로그 링크의 내용과 같이 구성했습니다.

https://rasino.tistory.com/221
