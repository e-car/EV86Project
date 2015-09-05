//       uno  mega
//digital 10   53  :SS
//digital 11   51  :MOSI
//digital 12   50  :MISO
//digital 13   52  :SCK

#define MEGA
//#define EDISON

//#define ROUTER1
//#define ROUTER2
#define ROUTER3

// ヘッダーのインクルード
#ifdef MEGA
#include <SoftwareSerial.h>
#endif
#include "ev86XBee.h"   
#include <SPI.h>


#ifdef MEGA
int rx = 10;
int tx = 11;
SoftwareSerial myserial = SoftwareSerial(rx, tx);
#endif

// Coordinatorの情報
typedef struct {
  uint32_t h64Add;
  uint32_t l64Add;
  String nodeName;
  String startReq;
  String startAck;
  boolean transmit;
} XBeeNode;

// Coordinator情報の初期化
XBeeNode coor = { 0x0013A200, 0x40E756D1, "Coordinator", "startReq", "startAck", false };  

// XBeeをRouterとして起動
EV86XBeeR router = EV86XBeeR();


// プロトタイプ宣言
void tmp(int x);
String float2String(float value);

String request = "request";    // コールバック用変数

#ifdef ROUTER1
String startAck = "startAck1"; // コネクション許可応答
String senData = "ROUTER1sensor"; // センサー値(仮)
#endif

#ifdef ROUTER2
String startAck = "startAck2"; // コネクション許可応答
String senData = "ROUTER2sensor"; // センサー値(仮)
#endif

#ifdef ROUTER3
String startAck = "startAck3"; 
String senData ="ROUTER3sensor";
#endif


// SPI & 熱電対式温度センサーパラメータ
#define SLAVE1 53   //uno 10 mega 53
#define SLAVE2 49   //uno  9 mega 49
unsigned int thermocouple;  // 14-Bit Thermocouple Temperature Data + 2-Bit
unsigned int internal;      // 12-Bit Internal Temperature Data + 4-Bit
float tempTime, tempTime2;
float disp, disp2;                 // display value
char s[10], s2[10]; // 時間
char c[10], c2[10];// 温度


void setup(){
  
  // SPI用ピン設定
  pinMode(SLAVE1, OUTPUT);
  digitalWrite(SLAVE1, HIGH);
  pinMode(SLAVE2, OUTPUT);
  digitalWrite(SLAVE2, HIGH);
  
  // SPIプロトコル設定
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
  SPI.setDataMode(SPI_MODE0);
  
  // PC−Arduino間Serial
  Serial.begin(9600);
  
  // Arduino-XBee間Serial
#ifdef MEGA
  myserial.begin(9600);
  myserial.flush();
  router.begin(myserial);
#endif

#ifdef EDISON
  Serial1.begin(9600);                         // Arduino-XBee間の通信
  Serial1.flush();
  router.begin(Serial1);
#endif
  delay(5000);
 
  // ホストXBeeの設定確認用メソッド
  router.hsXBeeStatus();                     
  delay(2000);
  
  // リモートXBeeの情報を確認
  router.setDstAdd64(coor.h64Add, coor.l64Add);
  router.rmXBeeStatus();
  Serial.println("Finish checking destination xbee node parameters");
  delay(2000); 
  
}

void loop(){
  Serial.println("-----------------------------");
  // 受信データの初期化
  router.clearData();
  
  // 温度取得
  tmp(SLAVE1);
  tmp(SLAVE2);
  
  // 送信用データに変換 (Temp1のみ)
  senData = float2String(disp);
  
  // 受信パケットの確認
  Serial.println("[get Packet]");
  router.getPacket();
   
  // 受信データが接続要求だった場合 
  if (router.checkData(coor.startReq)) {
   Serial.println("get [startReq]");
   
   // 接続応答を送信
   Serial.println("send startAck");
   router.sendData(startAck);
   
   // コーディネータからの接続応答を確認
   Serial.println("[get Packet]");
   for (int apiID, i = 0; ((apiID = router.getPacket()) != ZB_RX_RESPONSE) && !router.checkData(coor.startAck); i++) {
     Serial.print("timeout : ");
     Serial.println(i);
     
    // 受信データの初期化
    router.clearData();
    delay(100); 
   }
   coor.transmit = true;
   Serial.println("Connected with coordinator"); 
  } // 受信データがリクエストだった場合 
  else if (router.checkData(request)) {
    // センサーデータを送信する
    Serial.print("send : ");
    Serial.println(senData);
    router.sendData(senData);
    
    while(router.getPacket() != ZB_TX_STATUS_RESPONSE) {
      delay(20);
    }; 
    coor.transmit = true;
  } // データが来ていない場合
  else {
    Serial.println("Not send");
    coor.transmit = false;
  }
 
  // 接続状態の確認
  Serial.print("Transmit Status : ");
  Serial.println(coor.transmit);
  
  // 受信データの初期化
  router.clearData();
  delay(300);
}

// 熱電対式温度センサ取得関数
void tmp(int x){
  digitalWrite(x,LOW);                                      //  Enable the chip
  thermocouple  = (unsigned int)SPI.transfer(0x00) << 8;   //  Read high byte thermocouple
  thermocouple |= (unsigned int)SPI.transfer(0x00);        //  Read low byte thermocouple 
  internal  = (unsigned int)SPI.transfer(0x00) << 8;       //  Read high byte internal
  internal |= (unsigned int)SPI.transfer(0x00);            //  Read low byte internal 
  digitalWrite(x, HIGH);                                   //  Disable the chip
  
  if((thermocouple & 0x0001) != 0) {
    Serial.print("ERROR: ");
    if ((internal & 0x0004) !=0)  Serial.print("Short to Vcc, ");
    if ((internal & 0x0002) !=0)  Serial.print("Short to GND, ");
    if ((internal & 0x0001) !=0)  Serial.print("Open Circuit, ");
    Serial.println();
  }
  else {
    if((thermocouple & 0x8000) == 0){ // 0℃以上   above 0 Degrees Celsius 
      
      if (x == SLAVE1) {
        disp = (thermocouple >> 2) * 0.25;
      } else if (x == SLAVE2) {
         disp2 = (thermocouple >> 2) * 0.25;
      }
      
    }
    else {                            // 0℃未満   below zero
      if (x == SLAVE1) {
        disp = (0x3fff - (thermocouple >> 2) + 1)  * -0.25;
      } else if (x == SLAVE2) {
        disp2 = (0x3fff - (thermocouple >> 2) + 1)  * -0.25;
      }
      
    }
    
    if (x == SLAVE1) {
      tempTime = millis();
      tempTime /= 1000;
      dtostrf(tempTime, 6, 3, s);  //時間
      dtostrf(disp, 6, 2, c);  //温度
      Serial.print(s);
      Serial.print(";TMP1;");
      Serial.print(c);
      Serial.print(";");
      Serial.println();    
    } else if (x == SLAVE2) {
      tempTime2 = millis();
      tempTime2 /= 1000;
      dtostrf(tempTime2, 6, 3, s);  //時間
      dtostrf(disp2, 6, 2, c);  //温度
      Serial.print(s2);
      Serial.print(";TMP2;");
      Serial.print(c2);
      Serial.print(";");
      Serial.println();  
    }
  }
  delay(500);
}

// 変換 [float->string] 
String float2String(float value) {
  String result;
  result = (String)((int)value);
  result += ".";
  value -= (int)value;
  result += (int)(value * 10);
  return result;
}
