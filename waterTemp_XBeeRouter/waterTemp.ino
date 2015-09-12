// 水温データ取得用関数
String getWaterTemp() {
  
  //温度センサ読み取り処理
  n1 = analogRead(1);
  n2 = analogRead(2);
  
  rr1 = R1 * n1 / (1024.0 - n1);
  rr2 = R1 * n2 / (1024.0 - n2);
  
  t1 = 1 / (log(rr1 / R0) / B + (1 / T0));
  t2 = 1 / (log(rr2 / R0) / B + (1 / T0));
  
  temp1 = t1 - 273.15;
  temp2 = t2 - 273.15;
  
  Serial.print("rr1:");
  Serial.print(rr1);
  Serial.print(" Temp1:");
  Serial.println(temp1);
  
  Serial.print("rr2:");
  Serial.print(rr2);
  Serial.print(" Temp2:");
  Serial.println(temp2);
  
  // 送信用データに変換
  return float2String(temp1) + "," + float2String(temp2);
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

