

// Wifiセットアップ用関数
int setWiFi() {
  /*  WiFi setup */
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("Setting WiFi");
  
  String fv = WiFi.firmwareVersion();
  if( fv != "1.1.0" ) {
    Serial.println("Please upgrade the firmware");
    lcd.clear();
    lcd.setCursor(0, 0); // (0列, 0行)
    lcd.print("Please upgrade");
    lcd.setCursor(0, 1); // (0列, 1行)
    lcd.print("WiFi firmware");
  }
  else {
    Serial.print("Your WiFi Version OK :");
    Serial.println(fv);
  }
  
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("Trying to ");
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print("connect to WiFi");
  delay(1500);
  
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("SSID:");
  lcd.print(ssid);
  lcd.setCursor(0, 1); // (0列, 1行)
  int timeOutCount_WiFi = 0;
  // attempt to connect to WiFi network:
  while(status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID : ");
    Serial.println(ssid);
      
    // connect to WPA/WPA2 network. change this line if using open or WEP network;
    status = WiFi.begin(ssid, pass);
    // wait 5 seconds for connection
    delay(5000);
    
    lcd.setCursor(timeOutCount_WiFi, 1);
    lcd.print("#");
    
    if(timeOutCount_WiFi > 15) {
      Serial.println("Couldn't connect to WiFi network. Please try again to implement your Application Progaram");
      lcd.clear();
      lcd.setCursor(0, 0); // (0列, 0行)
      lcd.print("No connect WiFi");
      lcd.setCursor(0, 1); // (0列, 1行)
      lcd.print("Restart Arduino"); 
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
  
  // LCDにSSIDとIPアドレスを表示
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print("SSID:");
  lcd.print(WiFi.SSID());
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(ip);
}
