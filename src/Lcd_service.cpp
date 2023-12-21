#include "Lcd_service.h"

// Constructor
MyNextion::MyNextion() {
  softwareSerial = nullptr;//nulinis pointeris (pasirinkimas į kurį seriala nori rašyt)
}

void MyNextion::setSerial(SoftwareSerial* serial) {
  softwareSerial = serial;
}

void MyNextion::sendString(String str)
{
  if (softwareSerial != nullptr)
  {
    softwareSerial->print(str);
    softwareSerial->write(0xFF);
    softwareSerial->write(0xFF);
    softwareSerial->write(0xFF);
  }
  else
  {
    Serial.print(str);
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
  }
}
// Function to perform concatenation and send to Serial
void MyNextion::toNextion(String var_name, int value)
{
  var_name = var_name + ".val=";
  String comand = var_name + String(value);
  sendString(comand);
}

void MyNextion::fromNextion()
{
  tmr_1 = millis();

  if (softwareSerial->available() > 2)
  {
    char start_char = softwareSerial->read();
    if (start_char == '#')
    {
      uint8_t lenght = softwareSerial->read();
      //unsigned long tmr_1 = millis();
      boolean cmd_found = true;

      while (softwareSerial->available() < lenght)
      {
        if ((millis()-tmr_1)>100)
        {
          cmd_found = false;
          break;
        }
      }

      if (cmd_found == true)
      {
        uint8_t cmd = softwareSerial->read();
        switch (cmd)
        {
          case 'P':
          Serial.println ("turim puslapį");
          break;
        }
      }
    }
    else if (start_char=='8')
    {
      Serial.println ("isejau miegot");
    }
    else if (start_char=='7')
    {
      Serial.println ("atsibudau");
    }
    else if (start_char=='6')
    {
      Serial.println ("mane liecia");
    }
    
  }
}