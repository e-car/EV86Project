// Wifiセットアップ用関数
void setWiFi() {
   /*  WiFi setup */
    String fv = WiFi.firmwareVersion();
    if( fv != "1.1.0" ) {
      Serial.println("Please upgrade the firmware");
    }
    else {
      Serial.print("Your WiFi Version OK :");
      Serial.println(fv);
    }
    
    int timeOutCount_WiFi = 0;
    // attempt to connect to WiFi network:
    while(status != WL_CONNECTED) {
      Serial.print("Attempting ot connect to SSID : ");
      Serial.println(ssid);
      // connect to WPA/WPA2 network. change this line if using open or WEP network;
      status = WiFi.begin(ssid, pass);
      // wait 5 seconds for connection
      delay(5000);
      
      if(timeOutCount_WiFi > 20) {
        Serial.println("Couldn't connect to WiFi network. Please try again to implement your Application Progaram");
        while(true); // !!!!!!!!!!!!!
      }
      timeOutCount_WiFi++;
    }
    timeOutCount_WiFi = 0;
    Serial.println("WiFi Connection OK");
    
    /* server setup */
    server.begin(); // open server
    Serial.println("Open Server");
    
    // you're connected now, so print out the status:
    printWifiStatus();
}

/* output WiFi condition  */
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  // print your WiFi IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
