/*
*XBee APIフレーム用送受信プログラム
*随時更新していきます。
*現在できている項目
*  1: 宛先Addressの設定
*  2: 通常データの送受信
*  3: ATコマンドによるホストXBeeへの要求と応答の受け取り
*
*/

//#define MEGA
#define EDISON

//#define ROUTER1
//#define ROUTER2
#define ROUTER3

// ヘッダーのインクルード
#ifdef MEGA
#include <SoftwareSerial.h>
#endif
#include "ev86XBee.h"   

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

// 水温センサパラメータ
float B = 3435;
float T0 = 298.15;
float R0 = 10.0;
float R1 = 10.0;
int n1,n2;
float rr1,rr2,t1,t2,temp1,temp2;


void setup() {
  Serial.begin(9600);                          // Arduino-PC間の通信
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

void loop() {
  Serial.println("-----------------------------");
  // 受信データの初期化
  router.clearData();
  
  //温度センサ読み取り処理
  n1 = analogRead(1);
  n2 = analogRead(2);
  
  rr1 = R1 * n1 / (1024.0 - n1);
  rr2 = R1 * n2 / (1024.0 - n2);
  
  t1 = 1 / (log(rr1 / R0) / B + (1 / T0));
  t2 = 1 / (log(rr2 / R0) / B + (1 / T0));
  
  temp1 = t1 - 273.15;
  temp2 = t2 - 273.15;
  
  Serial.print("rr1:");
  Serial.print(rr1);
  Serial.print(" Temp1:");
  Serial.println(temp1);
  
  Serial.print("rr2:");
  Serial.print(rr2);
  Serial.print(" Temp2:");
  Serial.println(temp2);
  
  // 送信用データに変換
  senData = float2String(temp1);
  
  delay(1000);
  
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

// 変換 [float->string] 
String float2String(float value) {
  String result;
  result = (String)((int)value);
  result += ".";
  value -= (int)value;
  result += (int)(value * 10);
  return result;
}
