/*
*XBee APIフレーム用送受信プログラム
*随時更新していきます。
*現在できている項目
*  1: 宛先Addressの設定
*  2: 通常データの送受信
*  3: ATコマンドによるホストXBeeへの要求と応答の受け取り
*
* testXBeeRouterを元に、電流センサの値を読み、送信するプログラムを作成
* Arduino MEGA使用
* アナログ0ピン使用
* センサーへの印加電圧は5V
* 送信データは電流値
* 
*/

#define MEGA
//#define EDISON

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

String request = "request";    // コールバック用変数
String startAck = "startAck2";      // コネクション許可応答
String senData = "Water Temp"; // センサー値(仮)

// Coordinatorの情報
typedef struct {
  uint32_t h64Add;
  uint32_t l64Add;
  String nodeName;
  String startReq;
  String startAck;
  
} XBeeNode;

// Coordinator情報の初期化
XBeeNode coor = { 0x0013A200, 0x40B77090, "Coordinator", "startReq", "startAck" };  

// XBeeをRouterとして起動
EV86XBeeR router = EV86XBeeR();


// 電流センサ用変数
int analogPin = 0;     // ポテンショメータのワイプ(中央の端子)に
                       // 両端はグランドと+5Vに接続
float val = 0;           // 読み取った値を格納する変数
float amplitude = 0;
float amp_buf = 0;
int threshold = 512;   // 閾値

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
  
  // ホストXBeeの内部受信バッファをフラッシュする
  router.bufFlush();                         
  delay(1000);
  
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
  // センサデータの取得・送信用データの作成
  Serial.println("-----------------------------");
  
  // 電流センサの値を読む
  Serial.println("[Sensor_data]");
  // 3回計測してその平均を返す
  val = 0.0;
  for(int i=0;i<3;i++){
    val += (float)analogRead(analogPin);    // アナログピンを読み取る
    delay(10);
  }
  val = val / 3;
  if((val < 511+3) && (val > 511-3))
    val = 511;
//  amplitude = (val - 511.0)*(300.0/1024);
  amplitude = (val - 511) * (300.0 / 511);
  Serial.print("ad_data:");
  Serial.println(val);
  Serial.print("data : ");
  Serial.println(amplitude);            // デバッグ用に送信
  senData = float2String(amplitude);   // 値を文字列に変換
  
  // 受信データの初期化
  router.clearData();
  
  Serial.println("-----------------------------");
  // 受信パケットの確認
  Serial.println("[get Packet]");
  router.getPacket();
  delay(300);
   
  // 受信データが接続要求だった場合 
  if (router.checkData(coor.startReq)) {
   Serial.println("get startReq");
   
   // 接続応答を送信
   router.sendData(startAck);
   
   // 送信状態のチェック
   Serial.println("[get Packet]");
   router.getPacket(); 
  }
  // 受信データが接続応答だった場合 
  else if (router.checkData(coor.startAck)) {
   Serial.println("Connected with coordinator");
   
  }  
  // 受信データがリクエストだった場合
  else if (router.checkData(request)) {
    // センサーデータを送信する
    Serial.print("send :");
    Serial.println(senData);
    router.sendData(senData);
    Serial.println();
    delay(100);
    
    // 送信状態のチェック
    Serial.println("[get Packet]");
    router.getPacket();  
    
  } // データが来ていない場合
  else {
    Serial.println("Not send");
  }
  
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
