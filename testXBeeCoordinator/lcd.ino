// LCDの汎用関数をここにまとめてます。

void displayMsg(String msg1, String msg2) {
  lcd.clear();
  lcd.setCursor(0, 0); // (0列, 0行)
  lcd.print(msg1);
  lcd.setCursor(0, 1); // (0列, 1行)
  lcd.print(msg2);
}
  
