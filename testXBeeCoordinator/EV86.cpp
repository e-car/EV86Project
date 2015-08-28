//
//  EV86.cpp
//  
//
//  Created by 井上 真一 on 2015/08/25.
//
//
#include "EV86.h"

// コンストラクタ
EV86::EV86() {
    fieldNum = FIELD_NUM;
    
    bttryMainVolt = 0.0;
    bttryMainCurr = 0.0;
    bttry12Volt = 0.0;
    bttry12Curr = 0.0;
    motorTemp = 0.0;
    mcTemp = 0.0;
}

//// 変換 [float->string] 
//String EV86::float2String(float value) {
//  String result;
//  result = (String)((int)value);
//  result += ".";
//  value -= (int)value;
//  result += (int)(value * 10);
//  return result;
//}

// メインバッテリーの電圧を取得
void EV86::setMainVolt(float vol) {
  bttryMainVolt = vol; 
}

//　メインバッテリーの電流を取得
void EV86::setMainCurr(float curr) { 
  bttryMainCurr = curr; 
}

// 12ｖバッテリーの電圧値を設定
void EV86::set12Volt(float vol) {
  bttry12Volt = vol; 
}

// 12vバッテリーの電流値を設定
void EV86::set12Curr(float curr) {
  bttry12Curr = curr; 
}

// モーター温度値を設定
void EV86::setMotorTemp(float temp) { 
  motorTemp = temp; 
}

// モーターコントローラーの温度値を設定
void EV86::setMCTemp(float temp) {
  mcTemp = temp; 
}

// メインバッテリーの電力を算出
float EV86::getMainPower() {
  return bttryMainVolt * bttryMainCurr;
}

// 12vバッテリーの電力を算出
float EV86::get12vPower() {
  return bttry12Volt * bttry12Curr;
}

//// Androidに転送するためのデータプロトコルを作る
///* making protocol for sending EV86 data from Edison to Application clinet, for example Android or iPhone */
//String EV86::makePackage() {
//  String str = "#"; // set Start Mark up Sign
//  str += (String(getFieldNum()) + " "); // set the number of ev86 data
//  str += (float2String(getMainVolt()) + "  ");
//  str += (float2String(getMainCurr()) + "  ");
//  str += (float2String(get12Volt()) + "  ");
//  str += (float2String(get12Curr()) + "  ");
//  str += (float2String(getMotorTemp()) + "  ");
//  str += (float2String(getMcTemp()) + "  ");
//  return str;
//}

// 定義変数の数を取得
int EV86::getFieldNum() { 
  return fieldNum; 
}

// メインバッテリー電圧を取得
float EV86::getMainVolt() {
  return bttryMainVolt;
}

// メインバッテリー電流を取得
float EV86::getMainCurr() {
  return bttryMainCurr; 
}

// 12vバッテリー電圧を取得
float EV86::get12Volt() { 
  return bttry12Volt; 
}

// 12vバッテリー電流を取得
float EV86::get12Curr() { 
  return bttry12Curr; 
}

// モーター温度を取得
float EV86::getMotorTemp() { 
  return motorTemp;
}

// モーターコントローラ温度を取得
float EV86::getMcTemp() {
  return mcTemp;
}



