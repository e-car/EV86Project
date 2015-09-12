/*
* XBee-WiFiシステムのプログラム
*/

// ヘッダーのインクルード
#include <signal.h> // you must write down this line to resolve problem between WiFiSocket and Serial communication
#include <Wire.h>
#include <WiFi.h>
#include "EV86.h"
#include "ev86XBee.h"
#include "rgb_lcd.h"

/* -------------------------------- Wifi Parameters  -------------------------------- */
IPAddress ip;
char ssid[] = "304HWa-84F1A0"; // your network SSID (name), nakayama:506A   BUFFALO-4C7A25 iPhone_shinichi
char pass[] = "11237204a"; // your network password (use for WPA, or use as key for WEP), nakayama:12345678 iebiu6ichxufg 252554123sin
int keyIndex = 0; // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
int serverPort = 9090;
WiFiServer server(serverPort); // 9090番ポートを指定
WiFiClient client;
int socketTimeCount = 0;
const int socketTimeOut = 30; //10 20
boolean connectStatus;
String sendWiFiData = "";
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
XBeeNode router1 = { 0x0013A200, 0x40E756D4, "RMXBee_ROUTER1", "startAck1", "None", 50, false, false };
XBeeNode router2 = { 0x0013A200, 0x40E756D3, "RMXBee_ROUTER2", "startAck2", "None", 50, false, false };
XBeeNode router3 = { 0x0013A200, 0x40993791, "RMXBee_ROUTER3", "startAck3", "None", 50, false, false };

// コーディネーター用のインスタンスを生成
EV86XBeeC coor = EV86XBeeC();

// EV86インスタンス生成
//EV86 ev86 = EV86(); // 今回使わない

// LCD液晶ディスプレイインスタンスを生成
rgb_lcd lcd;
int clientCheckCount = 0;

// プロトタイプ宣言
boolean connectProcess(XBeeNode& router);
void gettingData(XBeeNode& router);

// 接続パラメータ
String hostXBee = "HOST_XBee";
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
  lcd.begin(16, 2);                            // LCD液晶ディスプレイの初期化 (16列, 2行)
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("Setting Serial");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print("ports: PC & XBee");
  
  Serial.begin(9600);                          // Arduino-PC間の通信
  Serial1.begin(9600);                         // Arduino-XBee間の通信
  coor.begin(Serial1);                         // XBeeにSerialの情報を渡す
  delay(5000);                                 // XBeeの初期化のために5秒間待機
   
  lcd.clear(); 
  lcd.setCursor(0, 0); // (0列, 0行)　 
  lcd.print("Trying to check");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print("XBees..."); 
  delay(500); 
    
    
  // HOST, REMOTE XBeeの設定を確認  
  /***********************************************************************************/  
    
  // ホストXBeeの設定確認
  coor.hsXBeeStatus();
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)　 
  lcd.print("Checked STATUS");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(hostXBee);
  delay(1000);
  
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(router1.h64Add, router1.l64Add);
  coor.rmXBeeStatus();
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)　 
  lcd.print("Checked STATUS");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router1.nodeName);
  delay(1000);
  
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(router2.h64Add, router2.l64Add);
  coor.rmXBeeStatus();
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)　 
  lcd.print("Checked STATUS");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router2.nodeName);
  delay(1000);
  
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(router3.h64Add, router3.l64Add);
  coor.rmXBeeStatus();
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)　 
  lcd.print("Checked STATUS");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router3.nodeName);
  delay(1000);
    
  // REMOTE XBeeとのコネクションを張る
  /***********************************************************************************/  
  
  // LCDにXBeeコネクション状況を知らせる
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("Connecting to");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router1.nodeName);
  
  //コネクション確立のためのセッション 
  if (connectProcess(router1)) {
    router1.firstTrans = true;
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)　 
    lcd.print("Connected to");
  } else {
    router1.firstTrans = false;
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)　 
    lcd.print("Disconnected to");
  }
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router1.nodeName); 
   
  delay(500); 
  
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("Connecting to");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router2.nodeName);
   
  //コネクション確立のためのセッション
  if (connectProcess(router2)) {
    router2.firstTrans = true;
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)　 
    lcd.print("Connected to");
  } else {
    router2.firstTrans = false;
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)　 
    lcd.print("Disconnected to");
  }
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router2.nodeName);
  
  delay(500);
  
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("Connecting to");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router3.nodeName);
   
  //コネクション確立のためのセッション
  if (connectProcess(router3)) {
    router3.firstTrans = true;
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)　 
    lcd.print("Connected to");
  } else {
    router3.firstTrans = false;
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)　 
    lcd.print("Disconnected to");
  }
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(router3.nodeName);
  

   // WiFi環境の構築　別途、WiFi.inoプログラムを参照するように
  /***********************************************************************************/
  setWiFi();
  lcd.clear();
}

void loop() {  
  Serial.println("[[[[[[ loop start ]]]]]]");
  Serial.print("Local IP : ");
  Serial.println(ip);
  Serial.print("Port : ");
  Serial.println(serverPort);
  
  //　サーバー(Edison)として指定したポートにクライアント(Android)からのアクセスがあるか確認。あれば、接続する 
  client = server.available();
  Serial.print("Client Status : ");
  Serial.println(client);

  // クライアント(Android)が存在する場合
  if (client) { 
    Serial.println("New Client");
    lcd.clear();
    lcd.print("New Client");
    socketTimeCount = 0;
    // クライアント(Android)とサーバー(Edison)
    // 処理に約5秒かかる
    while ((connectStatus = client.connected())) { // 注意!! client.connected()関数にはバグあり!
      socketTimeCount = 0;
      Serial.print("Socket TimeCount : ");
      Serial.println(socketTimeCount);
      Serial.print("Connect Status : ");
      Serial.println(connectStatus);
      
      lcd.clear();
      lcd.setCursor(0, 0); // (0列, 0行)
      lcd.print("Connected to");
      lcd.setCursor(0, 1); // (0列, 1行)
      lcd.print("WiFi Client");
      
      if (client.available() > 0) {
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
            gettingData(router1);
            Serial.println("*******************************************"); 
            gettingData(router2);  
            Serial.println("*******************************************"); 
            gettingData(router3);
            Serial.println("*******************************************"); 
            /*****************************************************************/
            
            // send router1 data to client by wifi
            if (router1.firstTrans && router1.transmit) {
              Serial.println(router1.sensorData);
            } else {
              Serial.println("Couldn't get router1.sensorData");
              router1.sensorData = "NA,NA";
            }
            
            // send router2 data to client by wifi
            if (router2.firstTrans && router2.transmit) {
              Serial.println(router2.sensorData);
            } else {
              Serial.println("Couldn't get router2.sensorData");
              router2.sensorData = "NA,NA";
            }
            
            // send router3 data to client by wifi
            if (router3.firstTrans && router3.transmit) {
              Serial.println(router3.sensorData);
            } else {
              Serial.println("Couldn't get router3.sensorData");
              router3.sensorData = "NA,NA";
            }
            
            sendWiFiData += router1.sensorData;
            sendWiFiData += ",";
            sendWiFiData += router2.sensorData;
            sendWiFiData += ",";
            sendWiFiData += router3.sensorData;
            
            Serial.print("send to WiFi [ ");
            Serial.print(sendWiFiData);
            Serial.println(" ]");
            client.println(sendWiFiData);
            sendWiFiData = "";
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
            
            lcd.clear();
            lcd.setCursor(0, 0); // (0列, 0行)　
            lcd.print("Close Socket");
            lcd.setCursor(0, 1); // (0列, 1行)
            lcd.print(" by Client Req");
            delay(2000);
            lcd.clear();
            clientCheckCount = 0;
            break;
          
          default:
            Serial.println("Error : Request from Client is invalid");
            client.println("Error : Request from Client is invalid");
            break;
        } // switch
        /***********************************************************************************/
        
        // 接続状態の確認
        // router firstTrans
        Serial.print(router1.nodeName);
        Serial.print(" First Connect Status : ");
        Serial.println(router1.firstTrans);
        // router
        Serial.print(router1.nodeName);
        Serial.print(" Connect Status : ");
        Serial.println(router1.transmit);
        
        // router2 firstTrans
        Serial.print(router2.nodeName);
        Serial.print(" First Connect Status : ");
        Serial.println(router2.firstTrans);
        // router2
        Serial.print(router2.nodeName);
        Serial.print(" Connect Status : ");
        Serial.println(router2.transmit);
        
        // router3 firstTrans
        Serial.print(router3.nodeName);
        Serial.print(" First Connect Status : ");
        Serial.println(router3.firstTrans);
        // router3
        Serial.print(router3.nodeName);
        Serial.print(" Connect Status : ");
        Serial.println(router3.transmit);
        Serial.println();
        Serial.println("-------------------------------------------------");
      } else {
        socketTimeCount++;
      }
      
       // 一定時間クライアントから応答がなければ、サーバー側からSocketを切る * client.connected()のバグ対策
       if (socketTimeCount > socketTimeOut) {
          lcd.clear();
          lcd.setCursor(0, 0); // (0列, 0行)　
          lcd.print("Close Socket");
          lcd.setCursor(0, 1); // (0列, 1行)
          lcd.print(" from Server");
          delay(2000);
          lcd.clear();
          socketTimeCount = 0;
          
          Serial.println("Socket TimeOut. close Socket from this server");
          client.flush();
          client.stop();
       }
    } // while
  } else {
    if (clientCheckCount > 15) {
      lcd.clear();
      clientCheckCount = 0;
    }
    
    // クライアントからの接続がない場合
    lcd.setCursor(0, 0); // (0列, 0行)
    lcd.print("No Client");
    lcd.setCursor(clientCheckCount, 1); // (0列, 1行)
    lcd.print("#");
    clientCheckCount++;
    delay(850);
  }
}


// コネクション確立のためのプロセス
boolean connectProcess(XBeeNode& router) {
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
  int stopCount = 0;
  for (int apiID, i = 0; (apiID = coor.getPacket()) != ZB_RX_RESPONSE && !coor.checkData(router.startAck); i++) {  
    
    // 5回再送信をしても応答がなかったら、接続を行わない。
    if (stopCount > 5) {
      Serial.println("Stop requesting and finish trying to connect");
      delay(1000);
      return false;
    }
    
    // タイムアウトの確認
    Serial.print("Time Count : ");
    Serial.println(i);
    
    // timeoutを超過したらルータへの接続要求を再送信する
    if (i > router.timeout) {
      coor.setDstAdd64(router.h64Add, router.l64Add);
      coor.sendData(startReq);
      i = 0;
      stopCount++;
      Serial.println("[[[ send startReq again ]]]");
    }
     
    // 受信データの初期化
    coor.clearData(); 
    delay(40);
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
         break; 
      }
    }
    delay(40);  
  };
  
  // 受信データの初期化
  coor.clearData();
  Serial.println("-------------------------------------------------------------");
  return true;
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
      Serial.print("Couldn't receive sensor data from");
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
    delay(40);  
  };
  
  // 受信データの初期化
  coor.clearData();
}
