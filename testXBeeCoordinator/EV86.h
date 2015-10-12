// 現在このヘッダーファイルは使用していません。
// 
//
//  EV86.h
//  
//
//  Created by 井上真一 on 2015/05/30.
//
//

#ifndef _EV86_h
#define _EV86_h
#include <String.h>

class EV86 {
private:
    float bttryMainVolt;
    float bttryMainCurr;
    float bttry12Volt;
    float bttry12Curr;
    float motorTemp;
    float mcTemp;
    
private:
    /* methods */
    //String float2String(float value);
    
public:
    /* constractor and destractor */
    EV86();
    
    /* methods */
    // setter
    void setMainVolt(float vol);
    void setMainCurr(float curr);
    void set12Volt(float vol);
    void set12Curr(float curr);
    void setMotorTemp(float temp);
    void setMCTemp(float temp);
    
    // getter
    //String makePackage();
    float getMainVolt();
    float getMainCurr();
    float get12Volt();
    float get12Curr();
    float getMotorTemp();
    float getMcTemp();
    float getMainPower();
    float get12vPower();
};

#endif
