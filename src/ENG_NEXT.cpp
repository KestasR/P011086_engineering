#include <Arduino.h>
#include <mcp2515.h>
#include <DHT.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "Adafruit_Sensor.h"
#include "Lcd_service.h"
#include "Timer.h"
#include <thermistor.h>
#include "next_receive.h"

#define _CAN_CS 10
#define _CAN_INT A0

#define pin_tmp_1 4
#define typ_tmp_1 DHT22
#define pin_tmp_2 5
#define typ_tmp_2 DHT22
#define pin_tmp_3 7
#define typ_tmp_3 DHT22
#define pin_tmp_4 6
#define typ_tmp_4 DHT22

#define NTC_PIN A1
#define RDC 19
#define PE 17
#define POWER_LOSS 18
#define MUTE_IN 16

#define MUTE_LED_OUT 8
#define Buzzer_OUT 9

int rdc_value;
int PE_value;
int POWER_LOSS_value;
int MUTE_IN_value;
int rdc_state;
int PE_state;
int POWER_LOSS_state;
int MUTE_IN_state;
int prev_MUTE_IN_STATE=0;
int MUTE_IN_STATE_COUNTER=0;

THERMISTOR thermistor(NTC_PIN, // Analog pin
                      47000,   // Nominal resistance at 25 ºC
                      4450,    // thermistor's beta coefficient
                      47000);  // Value of the series resistor
uint16_t ntc_temp;

MyNextion nextionas;
SoftwareSerial softSerial(3, 2);
Timer temp_timer;
Timer can_tx_timer;
Timer can_rx_timer;
Timer myTimer1;
Timer myTimer2;
Timer timer_ntc;
Timer timer_from_nextion;
Timer timer_IN;
Timer timer_to_nextion;
Timer timer_debounce;

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
String from_disp;
String dfd;

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  softSerial.begin(9600);
  nextionas.setSerial(&softSerial);

  canTx99.can_id = 99; // Can ID
  canTx99.can_dlc = 8; // Can message lenght

  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();

  pinMode(pin_tmp_1, INPUT);
  pinMode(pin_tmp_2, INPUT);
  pinMode(pin_tmp_3, INPUT);
  pinMode(pin_tmp_4, INPUT);

  pinMode(RDC, INPUT_PULLUP);
  pinMode(PE, INPUT_PULLUP);
  pinMode(POWER_LOSS, INPUT_PULLUP);
  pinMode(MUTE_IN, INPUT_PULLUP);
  pinMode(MUTE_LED_OUT, OUTPUT);
  pinMode(Buzzer_OUT, OUTPUT);

  myTimer1.setDuration(200);
  myTimer2.setDuration(200);
  temp_timer.setDuration(5000);
  timer_ntc.setDuration(5000);
  can_tx_timer.setDuration(300);
  can_rx_timer.setDuration(200);
  timer_from_nextion.setDuration(200);
  timer_IN.setDuration(1000);
  timer_to_nextion.setDuration(200);
  timer_debounce.setDuration(100);

  temp_sensor_1.begin();
  temp_sensor_2.begin();
  temp_sensor_3.begin();
  temp_sensor_4.begin();
}

void loop()
{
  // nextionas.fromNextion(dfd);

  if (timer_from_nextion.hasElapsed())
  {
    timer_from_nextion.restart();
  }

  if (softSerial.available())
  {
    dfd += char(softSerial.read());

    if (dfd.length() > 3 && dfd.substring(0, 3) != "TVC")
    {
      dfd = "";
      Serial.println("error");
    }
    else
    {
      if (dfd.substring((dfd.length() - 1), dfd.length()) == "?")
      {

        String command = dfd.substring(3, 6);
        String value = dfd.substring(6, dfd.length() - 1);
        Serial.println(command + " : " + value);
        dfd = "";
      }
    }
  }

  if (temp_timer.hasElapsed())
  {
    temp_timer.restart();
    temp[0] = (temp_sensor_1.readTemperature() + 100) * 10;
    temp[1] = (temp_sensor_2.readTemperature() + 100) * 10;
    temp[2] = (temp_sensor_3.readTemperature() + 100) * 10;
    temp[3] = (temp_sensor_4.readTemperature() + 100) * 10;

    nextionas.toNextion("gl.t_rck", temp[0]);
    nextionas.toNextion("gl.t_eng", temp[1]);
    nextionas.toNextion("gl.t_trf", (ntc_temp + 100) * 10);
    nextionas.toNextion("gl.t_out", temp[2]);
    nextionas.toNextion("gl.t_std", temp[3]);
  }

  if (can_tx_timer.hasElapsed())
  {
    canTx99.data[0] = temp[0];
    canTx99.data[1] = temp[1];
    canTx99.data[2] = temp[2];
    canTx99.data[3] = temp[3];
    canTx99.data[4] = ntc_temp;
    mcp2515.sendMessage(&canTx99);
    can_tx_timer.restart();
  }

  if ((mcp2515.readMessage(&canRx533) == MCP2515::ERROR_OK) && can_rx_timer.hasElapsed())
  {
    if (canRx533.can_id == 533)
    {
      can_rx_timer.restart();
      CAN_rx_data[0] = canRx533.data[0];
      CAN_rx_data[1] = canRx533.data[1];
      CAN_rx_data[2] = canRx533.data[2];
      CAN_rx_data[3] = canRx533.data[3];
      CAN_rx_data[4] = canRx533.data[4];
      CAN_rx_data[5] = canRx533.data[5];
    }
  }

  if (timer_ntc.hasElapsed())
  {
    timer_ntc.restart();
    ntc_temp = thermistor.read() / 10;
  }

  if (timer_IN.hasElapsed())
  {
    timer_IN.restart();
  }

  if (timer_to_nextion.hasElapsed())
  {
    timer_to_nextion.restart();
    rdc_value = analogRead(RDC);
    if (rdc_value<500)
    {
      rdc_state=1;
    }
    else
    {
      rdc_state=0;
    }
    PE_value = analogRead(PE);
    if (PE_value<500)
    {
      PE_state=1;
    }
    else
    {
      PE_state=0;
    }
    POWER_LOSS_value = analogRead(POWER_LOSS);
    if (POWER_LOSS_value<500)
    {
      POWER_LOSS_state=1;
    }
    else
    {
      POWER_LOSS_state=0;
    }
    
    MUTE_IN_value = analogRead(MUTE_IN);
    if (MUTE_IN_value<500)
    {
      MUTE_IN_state=1;
    }
    else
    {
      MUTE_IN_state=0;
    }
//MUTE mygtuko uzfiksavimas ir jo pozicijos siuntimas
    if (MUTE_IN_state!=prev_MUTE_IN_STATE&&timer_debounce.hasElapsed())
    {
      timer_debounce.restart();
      if(MUTE_IN_state == HIGH)
      {
        MUTE_IN_STATE_COUNTER++;
      }
    
    }
    if(MUTE_IN_STATE_COUNTER>=2) MUTE_IN_STATE_COUNTER=0;
    prev_MUTE_IN_STATE = MUTE_IN_state;

    nextionas.toNextion("gl.RDC", rdc_state);
    nextionas.toNextion("gl.PE", PE_state);
    nextionas.toNextion("gl.POWER", POWER_LOSS_state);
    nextionas.toNextion("gl.MUTE", MUTE_IN_STATE_COUNTER);
    Serial.println(MUTE_IN_STATE_COUNTER);
  }
}
