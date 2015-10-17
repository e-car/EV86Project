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
  byte spiData[4];            // 熱電対とのspiでは4byte(32bit)分のシリアルデータが来ます
  unsigned int thermocouple;  // 熱電対先の温度を格納する2byteのデータ型
  unsigned int internal;      // 内部基準温度を格納する2byteのデータ型
  bool sign = false;          // 負数フラグ
  
  unsigned int temp_integral;   //整数部格納用
  unsigned int temp_decimal;    //小数部格納用
  
  char str[16];                  //返り値用String
  
  digitalWrite(ss_pin, LOW);          //  Enable the chip
  delay(10);
  for(int i = 0; i < 4; i++){         //4byteのデータをスレーブから受け取る
    /* メモ */
    /* 回路の構成上，ビット反転(NOT : ~)をつけて取得すること */
    spiData[i] = ~SPI.transfer(0x00);     //0x00をスレーブに送りつける
                                          // ->これで返信だけもらう(通称ダミーデータという)
  }
  digitalWrite(ss_pin, HIGH);          //  Disable the chip
  
  //生データを見ておく
  for(int i = 0; i < 4; i++){
    Serial.print(spiData[i], HEX);
  }
  Serial.print(" ");
  
  /* 上位2byte(spiData[0]とspiData[1])は熱電対先の温度情報をもつ */
  /* 下位2byte(spiData[2]とspiData[3])は内部基準点温度情報をもつ */
  /* なので2byteごとに分割します */
  thermocouple  = ((unsigned int)spiData[0] << 8) | (unsigned int)spiData[1];   //  Read thermocouple data
  internal = ((unsigned int)spiData[2] << 8) | (unsigned int)spiData[3];        //  Read internal data
  
  
  if((thermocouple & 0x0001) == 0x0001) {  //エラー処理
    Serial.print("ERROR: ");
    if ((internal & 0x0004) !=0)  Serial.print("Short to Vcc, ");
    if ((internal & 0x0002) !=0)  Serial.print("Short to GND, ");
    if ((internal & 0x0001) !=0)  Serial.print("Open Circuit, ");
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
    
    
    /* step.2 戻り値Stringを作っていく                            */
    /* ArduinoではfloatからStringに直でキャストできない           */
    /* そのため,一度正数部と小数部に分割してsprintfする手段をとる */
    
    temp_integral = thermocouple >> 2;     //下位2bitを切り捨てれば整数部(上位11bit)のみ抽出
    temp_decimal = (thermocouple & 0x03);  //下位2bitを抽出すれば小数部のみ抽出
    temp_decimal = temp_decimal*25;        //25=0.25*100　小数部を整数値にしておく
    
    
    if(sign){ //負数のとき
      sprintf(str, "-%d.%-2", temp_integral, temp_decimal);
    }
    else{     //整数のとき
      sprintf(str, "%d.%-2", temp_integral, temp_decimal);
    }
    
    
    return str;
  }
  
}
