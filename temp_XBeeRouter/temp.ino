//       uno  mega
//digital 10   53  :SS
//digital 11   51  :MOSI
//digital 12   50  :MISO
//digital 13   52  :SCK


// 熱電対式温度センサ取得関数
String getTemp(int x){

  digitalWrite(x,LOW);                                     //  Enable the chip
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
        disp = (0x3fff - (thermocouple >> 2) + 1)  * -0.25; // 温度値(float) from Temp Sensor 1
      } else if (x == SLAVE2) {
        disp2 = (0x3fff - (thermocouple >> 2) + 1)  * -0.25; // 温度値(float) from Temp Sensor 2
      }
      
    }
    
    if (x == SLAVE1) {
      //tempTime = millis();
      //tempTime /= 1000;
      //dtostrf(tempTime, 6, 3, s);  //時間
      //dtostrf(disp, 6, 2, c);  //温度
      return float2String(disp); // 戻り値
      
    } else if (x == SLAVE2) {
      //tempTime2 = millis();
      //tempTime2 /= 1000;
      //dtostrf(tempTime2, 6, 3, s2);  //時間
      //dtostrf(disp2, 6, 2, c2);  //温度
      return float2String(disp2); // 戻り値
      
    }
  }
  
}

// 変換 [float->string] 
String float2String(float value) {
  String result;
  result = (String)((int)value);
  result += ".";
  value -= (int)value;
  result += (int)(value * 10); // caution! 対応していない部分有り
  return result;
}
