/*
* XBee-WiFiシステムのプログラム
*/

// ヘッダーのインクルード
#include <signal.h> // you must write down this line to resolve problem between WiFiSocket and Serial communication
#include <Wire.h>
#include <WiFi.h>
#include "ev86XBee.h"
#include "rgb_lcd.h"

// コーディネータが通信するXBeeの数
#define XBEE_NODE1
#define XBEE_NODE2
//#define XBEE_NODE3

/* -------------------------------- Wifi Parameters  -------------------------------- */
IPAddress ip;
char ssid[] = "iPhone_shinichi"; // your network SSID (name), nakayama:506A BUFFALO-4C7A25 304HWa-84F1A0
char pass[] = "252554123sin"; // your network password (use for WPA, or use as key for WEP), nakayama:12345678 iebiu6ichxufg   11237204a
int keyIndex = 0; // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
int serverPort = 9090;
WiFiServer server(serverPort); // 9090番ポートを指定
WiFiClient client;
int socketTimeCount = 0;
const int socketTimeOut = 20; //10 20 30
boolean connectStatus;
String sendWiFiData = "";
/* -------------------------------- Wifi Parameters  -------------------------------- */


/* -------------------------------- XBee Parameters  -------------------------------- */
// 他のNode情報
typedef struct {
  uint32_t h64Add;   // XBeeデバイス識別用64bitアドレスの上位32bit
  uint32_t l64Add;   // XBeeデバイス識別用64bitアドレスの下位32bit
  String nodeName;   // Node名
  String startAck;   // request識別子
  String sensorData; // 取得したセンサーデータ
  boolean transmit;  // 接続状況確認フラグ
} XBeeNode;

// ルーター情報の設定
#ifdef XBEE_NODE1
XBeeNode xbeeNode1 = { 0x0013A200, 0x40E756D4, "RMXBee_XBEE_NODE1", "startAck1", "None", false };
#endif
#ifdef XBEE_NODE2
XBeeNode xbeeNode2 = { 0x0013A200, 0x40E756D3, "RMXBee_XBEE_NODE2", "startAck2", "None", false };
#endif
#ifdef XBEE_NODE3
XBeeNode xbeeNode3 = { 0x0013A200, 0x40993791, "RMXBee_XBEE_NODE3", "startAck3", "None", false };
#endif

// 接続パラメータ
String hostXBee = "HOST_XBee";
String startReq = "startReq";
String startAck = "startAck";
String request =  "request";

// コーディネーター用のインスタンスを生成
EV86XBeeC coor = EV86XBeeC();
/* -------------------------------- XBee Parameters  -------------------------------- */


/* -------------------------------- LCD Parameters  -------------------------------- */
// LCD液晶ディスプレイインスタンスを生成
rgb_lcd lcd;
int clientCheckCount = 0;
/* -------------------------------- LCD Parameters  -------------------------------- */


/* -------------------------------- Prottype declares -------------------------------- */
boolean connectProcess(XBeeNode& router);
void gettingData(XBeeNode& router);
/* -------------------------------- Prottype declares -------------------------------- */


void setup() {  
  /* you must write down a following line */
  signal(SIGPIPE, SIG_IGN); // caution !! Please don't erase this line
  /*
   * serial port document
   * Serial : /dev/ttyGS0 in Edison(Linux OS), thus your arduino IDE serial monitor
   * Serial1 : /dev/ttyMFD1 in Edison(Linux OS), thus Pin 0(Rx) and 1(Tx) in your arduino breakout board for Edison
   * Serial2 : /dev/ttyMFD2 in Edison(Linux OS), thus a terminal when you login to J3 in arduino breakout board for Edison with microUSB-B   
  */
  
  // LCD液晶ディスプレイの初期化 (16列, 2行)
  lcd.begin(16, 2);                            
  displayMsg("Setting Serial", "Ports: PC & XBee");
  
  Serial.begin(9600);                          // Arduino-PC間の通信
  Serial1.begin(9600);                         // Arduino-XBee間の通信
  coor.begin(Serial1);                         // XBeeにSerialの情報を渡す
  delay(5000);                                 // XBeeの初期化のために5秒間待機
     
  // HOST, REMOTE XBeeの設定を確認  
  /***********************************************************************************/ 
  displayMsg("Trying to check", "XBees...");  
  delay(500);
  
  // ホストXBee  
  coor.hsXBeeStatus();
  displayMsg("Checked STATUS", hostXBee); 
  delay(500);
  
  // リモートXBee
#ifdef XBEE_NODE1  
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(xbeeNode1.h64Add, xbeeNode1.l64Add);
  coor.rmXBeeStatus();
  displayMsg("Checked STATUS", xbeeNode1.nodeName); 
  delay(500);
#endif
  
#ifdef XBEE_NODE2
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(xbeeNode2.h64Add, xbeeNode2.l64Add);
  coor.rmXBeeStatus();
  displayMsg("Checked STATUS", xbeeNode2.nodeName); 
  delay(500);
#endif
  
#ifdef XBEE_NODE3  
  // リモートXBeeのアドレス指定と設定情報の取得
  coor.setDstAdd64(xbeeNode3.h64Add, xbeeNode3.l64Add);
  coor.rmXBeeStatus();
  displayMsg("Checked STATUS", xbeeNode3.nodeName); 
  delay(500);
#endif
    
  // REMOTE XBeeとのファーストコネクションでセンサデータを通信できるか確認する
  /***********************************************************************************/  
#ifdef XBEE_NODE1  
  // LCDにXBeeコネクション状況を知らせる
  displayMsg("Connecting to", xbeeNode1.nodeName); 
  
  //コネクション確認のためのセッション 
  if (connectProcess(xbeeNode1)) {
    displayMsg("Connected to", xbeeNode1.nodeName);
  } else {
    displayMsg("Disconnected to", xbeeNode1.nodeName);
  } 
  delay(500); 
#endif

#ifdef XBEE_NODE2
  // LCDにXBeeコネクション状況を知らせる
  displayMsg("Connecting to", xbeeNode2.nodeName); 
  
  //コネクション確認のためのセッション 
  if (connectProcess(xbeeNode2)) {
    displayMsg("Connected to", xbeeNode2.nodeName);
  } else {
    displayMsg("Disconnected to", xbeeNode2.nodeName);
  } 
  delay(500); 
#endif
  
#ifdef XBEE_NODE3
  // LCDにXBeeコネクション状況を知らせる
  displayMsg("Connecting to", xbeeNode3.nodeName); 
  
  //コネクション確認のためのセッション 
  if (connectProcess(xbeeNode3)) {
    displayMsg("Connected to", xbeeNode3.nodeName);
  } else {
    displayMsg("Disconnected to", xbeeNode3.nodeName);
  } 
  delay(500); 
#endif

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
      Serial.print("Socket TimeCount : ");
      Serial.println(socketTimeCount);
      Serial.print("Connect Status : ");
      Serial.println(connectStatus);
      sendWiFiData = "";
      
      displayMsg("Connecting to", "WiFi Client");
      Serial.println("CHECK!!!");
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
#ifdef XBEE_NODE1
            gettingData(xbeeNode1);
            Serial.println("*******************************************"); 
#endif

#ifdef XBEE_NODE2
            gettingData(xbeeNode2);  
            Serial.println("*******************************************"); 
#endif

#ifdef XBEE_NODE3
            gettingData(xbeeNode3);
            Serial.println("*******************************************"); 
#endif      
            /*****************************************************************/

#ifdef XBEE_NODE1            
            // send xbeeNode1 data to client by wifi
            if (xbeeNode1.transmit) {
              Serial.println(xbeeNode1.sensorData);
            } else {
              Serial.println("Couldn't get xbeeNode1.sensorData");
              xbeeNode1.sensorData = "NA,NA";
            }
#endif

#ifdef XBEE_NODE2
            // send xbeeNode2 data to client by wifi
            if (xbeeNode2.transmit) {
              Serial.println(xbeeNode2.sensorData);
            } else {
              Serial.println("Couldn't get xbeeNode1.sensorData");
              xbeeNode2.sensorData = "NA,NA";
            }
#endif

#ifdef XBEE_NODE3
            // send xbeeNode3 data to client by wifi
            if (xbeeNode3.transmit) {
              Serial.println(xbeeNode3.sensorData);
            } else {
              Serial.println("Couldn't get xbeeNode1.sensorData");
              xbeeNode3.sensorData = "NA,NA";
            }
#endif

//#ifdef XBEE_NODE1            
//            sendWiFiData += xbeeNode1.sensorData;
//            sendWiFiData += ",";
//#endif
//
//#ifdef XBEE_NODE2            
//            sendWiFiData += xbeeNode2.sensorData;
//            sendWiFiData += ",";
//#endif
//
//#ifdef XBEE_NODE3
//            sendWiFiData += xbeeNode3.sensorData;
//#endif
//            sendWiFiData += "20.0,20.0";

            sendWiFiData += "20.0,20.0,";
            sendWiFiData += xbeeNode2.sensorData;
            sendWiFiData += ",20.0,20.0";
                 
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
        Serial.println("%%%%%%%%%%%%%%%%%%%Data_Status%%%%%%%%%%%%%%%%%%%%%%%");
#ifdef XBEE_NODE1        
        // router
        Serial.print(xbeeNode1.nodeName);
        Serial.print(" Connect Status : ");
        Serial.println(xbeeNode1.transmit);
#endif

#ifdef XBEE_NODE2
        // xbeeNode2
        Serial.print(xbeeNode2.nodeName);
        Serial.print(" Connect Status : ");
        Serial.println(xbeeNode2.transmit);
#endif

#ifdef XBEE_NODE3
        // xbeeNode3
        Serial.print(xbeeNode3.nodeName);
        Serial.print(" Connect Status : ");
        Serial.println(xbeeNode3.transmit);
#endif
        Serial.println("%%%%%%%%%%%%%%%%%%%Data_Status%%%%%%%%%%%%%%%%%%%%%%%");
      } else {
        // ソケットタイムアウトカウントをインクリメント
        Serial.println("socketTimeCount increase !");
        socketTimeCount++;
        Serial.print("socketTimeCount : ");
        Serial.println(socketTimeCount);
      }
      
       // 一定時間クライアントから応答がなければ、サーバー側からSocketを切る * client.connected()のバグ対策
       if (socketTimeCount > socketTimeOut) {
          displayMsg("Close Socket", " from Server");
          delay(2000);
          lcd.clear();
          socketTimeCount = 0;
          
          Serial.println("Socket TimeOut. close Socket from this server");
          client.flush();
          client.stop();
          break;
       }
       delay(100);
       Serial.println("While END");
       Serial.println("-------------------------------------------------");
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


// 接続確認用パッチ
boolean connectProcess(XBeeNode& router) {
  // XBee_Central →  XBee_Node  section1
  // XBee_Central ← XBee_Node  section2
  // XBee_Central →  XBee_Node  section3
  
  // 今から接続確認をとるXBee_Nodeの名前を表示
  Serial.print("[[[[[ CONNECTION with ");
  Serial.print(router.nodeName);
  Serial.println(" ]]]]]");
  
  // XBee_Nodeへの接続要求送信 (section1)
  coor.setDstAdd64(router.h64Add, router.l64Add);
  coor.sendData(startReq);
  Serial.println("[[[ send startReq ]]]");
  
  
  // XBee_Nodeからの受信パケットの確認
  Serial.print("[get Packet from ");
  Serial.print(router.nodeName);
  Serial.println("]");
    
  // XBee_Nodeから接続応答が来ているかチェック(section2) 
  int stopCount = 0;
  for (int apiID, i = 0; (apiID = coor.getPacket()) != ZB_RX_RESPONSE && !coor.checkData(router.startAck); i++) {  
    
    // 3回再送信をしても応答がなかったら、接続を行わない。
    if (stopCount > 2) {
      Serial.println("Stop requesting and finish trying to connect");
      delay(1000);
      return false;
    }
    
    // タイムアウトの確認
    Serial.print("Time Count : ");
    Serial.println(i);
    
    // timeout(50)を超過したらXBee_Nodeへの接続要求を再送信する
    if (i > 50) {
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
  
  // 接続応答を送信 (section3)
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
    // timeout(50)に設定
    if (count > 50) {
      router.transmit = false;
      coor.bufFlush();
      Serial.print("Couldn't receive sensor data from ");
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
