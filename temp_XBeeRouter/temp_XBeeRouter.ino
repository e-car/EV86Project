

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

// コネクション用のパラメータ
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

// プロトタイプ宣言
String float2String(float value);
String getTemp(int x);

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
  
  // PC−Arduino間
  Serial.begin(9600);
  
  // Arduino-XBee間Serial
#ifdef MEGA
  // Arduino-XBee間の通信
  myserial.begin(9600);
  router.begin(myserial);
#endif
#ifdef EDISON
  // Arduino-XBee間の通信
  Serial1.begin(9600);                         
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
  // 送信用データの初期化
  senData = "";
  
  // センサデータの取得・送信用データの作成
  /***********************************************/
  // 温度センサの値を読む
  Serial.println("[Sensor_data]");
  
  // 送信用データに変換
  senData = getTemp(SLAVE1);
  senData += ",";
  senData += getTemp(SLAVE2);
  
  // XBeeデータ受信
  /************************************************/
  
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
  delay(30);
}


