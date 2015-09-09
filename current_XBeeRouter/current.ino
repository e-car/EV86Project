// 電流センサの値を読み取る関数
String getCurrent() {
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
  return float2String(amplitude);   // 値を文字列に変換
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
