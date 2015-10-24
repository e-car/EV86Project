//       uno  mega
//digital 10   53  :SS
//digital 11   51  :MOSI
//digital 12   50  :MISO
//digital 13   52  :SCK


// 熱電対式温度センサ取得関数
// 引数: 
//    int ss_pin : スレーブ選択用のデジタルピン番号
// 戻り値:
//    String : 計測した温度を文字列型で返す
String getTemp(int ss_pin)
{
  byte spiData[2];            // 熱電対とのspiでは4byte(32bit)分のシリアルデータが来ます
  int thermocouple = 0;
  bool sign = false;
  
  
  digitalWrite(ss_pin, LOW);          //  Enable the chip
  delay(1);
  for(int i = 0; i < 2; i++){         //4byteのデータをスレーブから受け取る
    /* メモ */
    /* 回路の構成上，ビット反転(NOT : ~)をつけて取得すること */
    spiData[i] = ~SPI.transfer(0);     //0x00をスレーブに送りつける
                                          // ->これで返信だけもらう(通称ダミーデータという)
  }
  digitalWrite(ss_pin, HIGH);          //  Disable the chip


  /* 上位2byte(spiData[0]とspiData[1])は熱電対先の温度情報をもつ */
  /* 下位2byte(spiData[2]とspiData[3])は内部基準点温度情報をもつ */
  /* なので2byteごとに分割します */
  thermocouple  = ((spiData[0] << 8) | spiData[1]);   //  Read thermocouple data
  
  
  if((thermocouple & 0x0001) == 0x0001) {  //エラー処理
    Serial.print("ERROR: ");
    Serial.println();
  }
  else {  //エラーが発生していない場合thermocoupleを温度値に変換していく
    
    thermocouple = thermocouple >> 2;  //"RES", "Fault"ビットの破棄  
  
  
    /* step.1 符号ビットを確認*/
    if((thermocouple & 0x2000) == 0x2000){     // 0°未満の場合
      sign = true;                             // 負数フラグを立てる
      thermocouple = ~thermocouple + 0x0001;   // 正数表現に変えておく
      thermocouple &= 0x3FFF;                  // 数値部分抽出
    }
    else{
      sign = false;   //正数(0°以上です)
    }
    
    
    /* step.2 戻り値Stringを作っていく */
    
    int Integer = thermocouple >> 2;    //整数部(上位11bit)
    int Decimal = thermocouple & 0x03;  //小数部(下位2ビット)
    
    
    if(sign){ //負数のとき
      String str = String(Integer, DEC);
      str += ".";
      str += String(Decimal*25);
      return str;
    }
    else{     //正数のとき
      String str = String(Integer, DEC);
      str += ".";
      str += String(Decimal*25);
      return str;
    }
  }
  
}
