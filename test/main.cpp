#include <Arduino.h>
#include <mcp2515.h>
#include <DHT.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "Adafruit_Sensor.h"
#include "Lcd_service.h"
#include "Timer.h"

#define	_CAN_CS		10	
#define	_CAN_INT	A0

#define pin_tmp_1 4
#define typ_tmp_1 DHT22
#define pin_tmp_2 5
#define typ_tmp_2 DHT22
#define pin_tmp_3 6
#define typ_tmp_3 DHT22
#define pin_tmp_4 7
#define typ_tmp_4 DHT22

MyNextion nextionas;
SoftwareSerial softSerial(3, 2);
Timer temp_timer;
Timer can_tx_timer;
Timer can_rx_timer;
Timer myTimer1;
Timer myTimer2;

DHT temp_sensor_1(pin_tmp_1, typ_tmp_1);
DHT temp_sensor_2(pin_tmp_2, typ_tmp_2);
DHT temp_sensor_3(pin_tmp_3, typ_tmp_3);
DHT temp_sensor_4(pin_tmp_4, typ_tmp_4);

MCP2515 mcp2515(10);
struct can_frame canTx99;
struct can_frame canRx533;
uint8_t CAN_rx_data[8];

float temp[4];
float hum[4];


void setup()
{
  Serial.begin(9600);
  SPI.begin();
  softSerial.begin(9600);
  nextionas.setSerial(&softSerial);

  canTx99.can_id = 99; //Can ID
  canTx99.can_dlc = 8; //Can message lenght

  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  pinMode(pin_tmp_1, INPUT);
  pinMode(pin_tmp_2, INPUT);
  
  myTimer1.setDuration(200);
  myTimer2.setDuration(200);
  temp_timer.setDuration(5000);
  can_tx_timer.setDuration(300);
  can_rx_timer.setDuration(200);

  temp_sensor_1.begin();
  temp_sensor_2.begin();
}

void loop()
{

  if (temp_timer.hasElapsed())
  {
    temp_timer.restart();
    temp[0] = temp_sensor_1.readTemperature();
    temp[1] = temp_sensor_2.readTemperature();
    temp[2] = temp_sensor_3.readTemperature();
    temp[3] = temp_sensor_4.readTemperature();

    nextionas.toNextion("t_rck", temp[0]*10);
    nextionas.toNextion("t_eng", temp[1]*10);
    nextionas.toNextion("t_trf", temp[2]*10);
    nextionas.toNextion("t_out", temp[3]*10);
    nextionas.toNextion("t_std", temp[0]*10);

  }

  if (can_tx_timer.hasElapsed())
  {
    canTx99.data[0]=temp[0]*10;
    canTx99.data[1]=temp[1]*10;
    canTx99.data[2]=temp[2]*10;
    canTx99.data[3]=temp[3]*10;
    canTx99.data[4]=temp[0]*10;
    mcp2515.sendMessage(&canTx99);
    can_tx_timer.restart();
  }

  if ((mcp2515.readMessage(&canRx533)==MCP2515::ERROR_OK)&& can_rx_timer.hasElapsed())
  {
    if(canRx533.can_id==533)
    {
      can_rx_timer.restart();
      CAN_rx_data[0]=canRx533.data[0];
      CAN_rx_data[1]=canRx533.data[1];
      CAN_rx_data[2]=canRx533.data[2];
      CAN_rx_data[3]=canRx533.data[3];
      CAN_rx_data[4]=canRx533.data[4];
      CAN_rx_data[5]=canRx533.data[5];      
    }
  }
}
