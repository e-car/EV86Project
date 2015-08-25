// 変換 [string->float]
float string2Float(String str) {
  int index = str.indexOf('.');
  char charArray[str.length() + 1];
  str.toCharArray(charArray, (str.length() + 1));
  float result = 0;
  
  for (int i = 0; i < index ; i++) {
    result += (int)(charArray[i] - '0') * (int)pow(10, (index - 1) - i);
  }
  
  for (int i = (index + 1); i < str.length(); i++) {
    result += (int)(charArray[i] - '0') * pow(0.1, (i - index));
  }
  return result;
}
