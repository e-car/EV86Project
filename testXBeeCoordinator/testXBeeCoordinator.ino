/*
* XBee-WiFiシステムのプログラム
*/


// ヘッダーのインクルード
#include <signal.h> // you must write down this line to resolve problem between WiFiSocket and Serial communication
#include <WiFi.h>
#include "EV86.h"
#include "ev86XBee.h"

/* -------------------------------- Wifi Parameters  -------------------------------- */
char ssid[] = "iPhone_shinichi"; // your network SSID (name), nakayama:506A 304HWa-84F1A0 BUFFALO-4C7A25
char pass[] = "252554123sin"; // your network password (use for WPA, or use as key for WEP), nakayama:12345678 11237204a iebiu6ichxufg
int keyIndex = 0; // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(9090); // 9090番ポートを指定
int socketTimeCount = 0;
const int socketTimeOut = 20;
boolean connectStatus;
/* -------------------------------- Wifi Parameters  -------------------------------- */

// Routerの情報
typedef struct {
  uint32_t h64Add;
  uint32_t l64Add;
  String nodeName;
  String startAck;
  String sensorData; // 取得したセンサーデータ
  int timeout;
  boolean transmit;
  boolean firstTrans;
} XBeeNode;

// ルーター情報の設定
XBeeNode router = { 0x0013A200, 0x40993791, "EDISON", "startAck1", "None", 50, false, false };
XBeeNode router2 = { 0x0013A200, 0x40707DF7, "MEGA", "startAck2", "None", 50, false, false };
// コーディネーター用のインスタンスを生成
EV86XBeeC coor = EV86XBeeC();
// EV86インスタンス生成
EV86 ev86 = EV86();

// プロトタイプ宣言
void connectProcess(XBeeNode& router);
void gettingData(XBeeNode& router);

// 接続パラメータ
String startReq = "startReq";
String startAck = "startAck";
String request =  "request";

void setup() {  
  /* you must write down a following line */
  signal(SIGPIPE, SIG_IGN); // caution !! Please don't erase this line
  /*
   * serial port document
   * Serial : /dev/ttyGS0 in Edison(Linux OS), thus your arduino IDE serial monitor
   * Serial1 : /dev/ttyMFD1 in Edison(Linux OS), thus Pin 0(Rx) and 1(Tx) in your arduino breakout board for Edison
   * Serial2 : /dev/ttyMFD2 in Edison(Linux OS), thus a terminal when you login to J3 in arduino breakout board for Edison with microUSB-B   
  */
  Serial.begin(9600);                          // Arduino-PC間の通信
  Serial1.begin(9600);                         // Arduino-XBee間の通信
  Serial1.flush();                             // serial bufferをクリア
  coor.begin(Serial1);                         // XBeeにSerialの情報を渡す
  delay(5000);                                 // XBeeの初期化のために5秒間待機
   
  // ホストXBeeの内部受信バッファをフラッシュする
  coor.bufFlush();
  delay(1000);
  
  // ホストXBeeの設定確認
  coor.hsXBeeStatus();                     
  delay(2000);
  
//  // リモートXBeeのアドレス指定と設定情報の取得
//  coor.setDstAdd64(router.h64Add, router.l64Add);
//  coor.rmXBeeStatus();
//  delay(2000);
  
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(router2.h64Add, router2.l64Add);
  coor.rmXBeeStatus();
  delay(2000);
  
  //コネクション確立のためのセッション
  //connectProcess(router);
  connectProcess(router2);
  
  // set WiFi
  //setWiFi();
  delay(1000);
}

void loop() {  
  Serial.println("[[[[[[ loop start ]]]]]]");
  //　サーバー(Edison)として指定したポートにクライアント(Android)からのアクセスがあるか確認。あれば、接続する 
  WiFiClient client = server.available();
  Serial.print("Client Status : ");
  Serial.println(client);

  // クライアント(Android)が存在する場合
  if (client) { 
    Serial.println("New Client");
    socketTimeCount = 0;
    // クライアント(Android)とサーバー(Edison)
    // 処理に約5秒かかる
    while ((connectStatus = client.connected())) { // 注意!! client.connected()関数にはバグあり!
      Serial.print("Socket TimeCount : ");
      Serial.println(socketTimeCount);
      Serial.print("Connect Status : ");
      Serial.println(connectStatus);
      
      if (client.available() > 0) {
        socketTimeCount = 0;
        char revChar = client.read(); // read from TCP buffer
        Serial.print("Get [");
        Serial.print(revChar);
        Serial.print("] -> Send  : ");
        // クライアントからのリクエストの受け取りとその処理
        /***********************************************************************************/
        switch(revChar) {  
          case 'G':
          // センサーデータ取得 from XBee
            Serial.println("Sensor Data by XBee");
            /*****************************************************************/
            //gettingData(router);
            Serial.println("*******************************************"); 
            gettingData(router2);  
            /*****************************************************************/
            
            // send router1 data to client by wifi
            if (router.firstTrans && router.transmit) {
              Serial.println(router.sensorData);
              client.println(router.sensorData);
            } else {
              Serial.println("Couldn't get router.sensorData");
              client.println("Couldn't get router.sensorData");
            }
            // send router2 data to client by wifi
            if (router2.firstTrans && router2.transmit) {
              Serial.println(router2.sensorData);
              client.println(router2.sensorData);
            } else {
              Serial.println("Couldn't get router2.sensorData");
              client.println("Couldn't get router2.sensorData");
            }
            break;
          
          case 'T':
            Serial.println("test EV86 car data");
            client.println("test EV86 car data");
            break;
            
          case 'S':
            // close the connection:
            Serial.println("Stop Signal & Close Socket");
            client.flush();
            client.stop();
            Serial.println("client disonnected");
            break;
          
          default:
            Serial.println("Error : Request from Client is invalid");
            client.println("Error : Request from Client is invalid");
            break;
        }
        /***********************************************************************************/
      } else {
        socketTimeCount++;
      }
    
      // 接続状態の確認
      // router firstTrans
      Serial.print(router.nodeName);
      Serial.print(" First Connect Status : ");
      Serial.println(router.firstTrans);
      // router
      Serial.print(router.nodeName);
      Serial.print(" Connect Status : ");
      Serial.println(router.transmit);
      
      // router2 firstTrans
      Serial.print(router2.nodeName);
      Serial.print(" First Connect Status : ");
      Serial.println(router2.firstTrans);
      // router2
      Serial.print(router2.nodeName);
      Serial.print(" Connect Status : ");
      Serial.println(router2.transmit);
      Serial.println();
      Serial.println("-------------------------------------------------");
      delay(100);
      
      // 一定時間クライアントから応答がなければ、サーバー側からSocketを切る * client.connected()のバグ対策
      if (socketTimeCount > socketTimeOut) {
        Serial.println("Socket TimeOut. close Socket from this server");
        client.flush();
        client.stop();
        break;
      }
    }
  }
  delay(500);
}

// コネクション確立のためのプロセス
void connectProcess(XBeeNode& router) {
  // coor →  router
  // coor ← router
  // coor →  router
  
  // コネクションを取るXBeeのノード名を確認
  Serial.print("[[[[[ CONNECTION with ");
  Serial.print(router.nodeName);
  Serial.println(" ]]]]]");
  
  // ルーターへの接続要求送信
  coor.setDstAdd64(router.h64Add, router.l64Add);
  coor.sendData(startReq);
  Serial.println("[[[ send startReq ]]]");
  
  
  // 受信パケットの確認
  Serial.print("[get Packet from ");
  Serial.print(router.nodeName);
  Serial.println("]");
    
  // ルーターから接続応答が来ているかチェック  
  for (int apiID, i = 0; (apiID = coor.getPacket()) != ZB_RX_RESPONSE && !coor.checkData(router.startAck); i++) {  
    // タイムアウトの確認
    Serial.print("timecount : ");
    Serial.println(i);
  
    // timeoutを超過したらルータへの接続要求を再送信する
    if (i > router.timeout) {
      coor.setDstAdd64(router.h64Add, router.l64Add);
      coor.sendData(startReq);
      i = 0;
      Serial.println("[[[ send startReq again ]]]");
    }
     
    // 受信データの初期化
    coor.clearData();
    delay(30);
  }
  
  // 接続応答を送信
  coor.setDstAdd64(router.h64Add, router.l64Add);
  coor.sendData(startAck);
  Serial.println("send startAck\n");
  
  // 受信パケットの確認
  Serial.println("[get Packet]");
  int count = 0;
  while (true) {  
    int apiID = coor.getPacket();
      
    if (apiID == ZB_TX_STATUS_RESPONSE) {
      if (coor.getConnectStatus() == SUCCESS) {
         router.firstTrans = true;
         break; 
      }
    }
    delay(30);  
  };
  
  // 受信データの初期化
  coor.clearData();
    
  // コネクションの確立の有無
  if (router.firstTrans == true) {
    Serial.print("[[[ Connected with Router in ");
    Serial.print(router.nodeName);
    Serial.println(" ]]]");
  } else {
    Serial.print("[[[ Disconnected with Router in ");
    Serial.print(router.nodeName);
    Serial.println(" ]]]");
  }
    Serial.println("-------------------------------------------------------------");
  /* ------------------------------------------------- */
}


// ポーリングによる各XBeeへのリクエスト送信とレスポンス受信
void gettingData(XBeeNode& router) {
  // センサーデータ要求
  coor.setDstAdd64(router.h64Add, router.l64Add); // 宛先アドレスの指定
  coor.sendRequest(request);                      // リクエスト送信
  
  // 受信パケットの確認
  Serial.print("[get Packet from ");
  Serial.print(router.nodeName);
  Serial.println("]");
  
  // 受信パケットの確認
  int count = 0;
  while (true) {  
    int apiID = coor.getPacket();
    
    if (count > router.timeout) {
      router.transmit = false;
      coor.bufFlush();
      Serial.print("Couldn't receive seonsor data from");
      Serial.print(router.nodeName);
      Serial.println(" on XBee Network");
      break;
    }
      
    if (apiID == ZB_RX_RESPONSE) {
      router.transmit = true;
      // 受信したセンサーデータ確認
      router.sensorData = coor.getData();
      Serial.print("Sensor Data: "); 
      Serial.println(router.sensorData);
      Serial.println();
      break;
    }  
      
    if (apiID < 0) {
      count++;
    }
    
    delay(30);  
  };
  // 受信データの初期化
  coor.clearData();
}
