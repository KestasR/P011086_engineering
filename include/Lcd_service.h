#ifndef Lcd_sercice_h
#define Lcd_sercice_h

#include <Arduino.h>
#include <SoftwareSerial.h>

class MyNextion
{
public:
    // Constructor
    MyNextion();
    void toNextion(String var_name, int value);
    void setSerial(SoftwareSerial* serial);
    void fromNextion (String dfd);
private:
    void sendString(String str);
    SoftwareSerial* softwareSerial;
    unsigned long tmr_1;
};

#endif