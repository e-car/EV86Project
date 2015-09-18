/*
*XBee APIフレーム用送受信プログラム
*随時更新していきます。
*現在できている項目
*  1: 宛先Addressの設定
*  2: 通常データの送受信
*  3: ATコマンドによるホストXBeeへの要求と応答の受け取り
*
*/

#define MEGA
//#define EDISON

//#define ROUTER1
#define ROUTER2
//#define ROUTER3

// ヘッダーのインクルード
#ifdef MEGA
#include <SoftwareSerial.h>
#endif
#include "ev86XBee.h"   

#ifdef MEGA
int rx = 10;
int tx = 11;
SoftwareSerial mySerial = SoftwareSerial(rx, tx);
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

// コネクション用パラメータ
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
String senData = "ROUTER3sensor";
#endif

// 電流センサ用変数
int analogPin_Cur = 0;     // ポテンショメータのワイプ(中央の端子)に両端はグランドと+5Vに接続
float val_Cur = 0;           // 読み取った値を格納する変数
float amplitude_Cur = 0;
int threshold_Cur = 512;   // 閾値

// 電圧センサ用プログラム
int analogPin_Vol = 1;
float val_Vol = 0;
float amplitude_Vol = 0;

void setup() {
  // Arduino-PC間の通信
  Serial.begin(9600);                          
#ifdef MEGA
  // Arduino-XBee間の通信
  mySerial.begin(9600);
  // mySerialをXBeeに設定する
  router.begin(mySerial);
#endif
#ifdef EDISON
  // Arduino-XBee間の通信
  Serial1.begin(9600);                         // Arduino-XBee間の通信
  // Serial1をXBeeに設定する
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

void loop() {
  Serial.println("-----------------------------");
  
  // センサデータの取得・送信用データの作成
  /************************************************/
  Serial.println("[Sensor_data]");
  // 電流センサの値を読む
  senData = getCurrent();
  
  senData += ',';
 
  senData += getVoltage();
  
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
  //delay(300);
}


