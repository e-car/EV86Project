// 電流センサの値を読み取る関数
String getCurrent() {
  
  // 3回計測してその平均を返す
  val_Cur = 0.0;
  for (int i = 0;i < 3;i++){
    val_Cur += (float)analogRead(analogPin_Cur);    // アナログピンを読み取る
    delay(10);
  }
  val_Cur = val_Cur / 3;
  
  if((val_Cur < 511 + 3) && (val_Cur > 511 - 3))
    val_Cur = 511;
//  amplitude = (val_Cur - 511.0)*(300.0 / 1024);
  amplitude_Cur = (val_Cur- 511) * (300.0 / 511);
  Serial.print("ad_data_Current:");
  Serial.println(val_Cur);
  Serial.print("data_Current : ");
  Serial.println(amplitude_Cur);            // デバッグ用に送信
  Serial.println();
  return float2String(amplitude_Cur);   // 値を文字列に変換
}

// 電圧センサの値を読み取る関数
String getVoltage() {
  
  // 3回計測してその平均を返す
  val_Vol = 0.0;
  for (int i = 0;i < 3;i++){
    val_Vol += (float)analogRead(analogPin_Vol);    // アナログピンを読み取る
    delay(50);
  }
  val_Vol = val_Vol / 3;
  
  amplitude_Vol = val_Vol;
  Serial.print("ad_data_Voltage:");
  Serial.println(val_Vol);
  Serial.print("data_Voltage: ");
  Serial.println(amplitude_Vol);
  return float2String(amplitude_Vol);
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
