#include <Arduino.h>
#include "ModbusSlave.h"
#include <EEPROM.h>
#include <esp_task_wdt.h>
#include <vector>
#include "Punch.h"
#define u8 uint8_t
//Pin attatch
#define valve_ad 27
#define valve_de 14
#define punch_key 25
#define LED 2
#define EEP_size 30
using namespace std;
Modbus MBslave;
Punch punch;
std::vector<short *>RS_MAP_DATA;
hw_timer_t *timer1 = NULL;
void update_master_code(short addr,short val);
char punch_step_STA =0 ,save_STA =0; //0:punch_delay  1:punch_out 2:punch_stay 3:poll_back
//-----timer1 callback
void timer1_callback(void){
  timerStop(timer1);
  punch_step_STA ++;
  if(punch_step_STA == 4){
    punch_step_STA = 0;
  }
}

uint8_t readRSDATA(uint8_t fc, uint16_t address, uint16_t length)
{
    // Read the requested registers.
        if (address + length <= RS_MAP_DATA.size())
        {   
            auto arr = std::vector<uint16_t>{};
            for (int i = 0; i < length; i++){
                // Serial.println(*static_cast<uint16_t*>(RS_MAP_DATA[address+i]));
                arr.emplace_back(*RS_MAP_DATA[address+i]);
            }
            MBslave.writeArrayToBuffer(0,arr.data(),arr.size());
        }
        else 
            return STATUS_ILLEGAL_DATA_ADDRESS;
    return STATUS_OK;
}
uint8_t writeRSDATA(uint8_t fc, uint16_t address, uint16_t length)
{
    // Read the requested registers.   size - 4 --> writenable
        if (address + length <= RS_MAP_DATA.size())
        {
            for(uint8_t i =0 ; i< length ; i++){
                uint16_t value = MBslave.readRegisterFromBuffer(i);
                update_master_code(address + i ,value);
            }
            save_STA = 1;
        }
        else {
            return STATUS_ILLEGAL_DATA_ADDRESS;
        }
    return STATUS_OK;
}

class Slave{
public:
    short slave_ID = 1 ;
    int Baudrate = 115200;
    short punch_out_time = 1 ,poll_back_time = 1;
    short punch_delay_ms = 1 ,punch_out_stay = 1;
  void Map_DATA(){
    RS_MAP_DATA.push_back(&this-> slave_ID);       //R/W and save
    RS_MAP_DATA.push_back(&this-> punch_out_time);
    RS_MAP_DATA.push_back(&this-> punch_out_stay);
    RS_MAP_DATA.push_back(&this-> poll_back_time);
    RS_MAP_DATA.push_back(&this-> punch_delay_ms);
  }
  void slave_init(){
    MBslave.cbVector[CB_READ_COILS] = readRSDATA;
    MBslave.cbVector[CB_READ_DISCRETE_INPUTS] = readRSDATA;
    MBslave.cbVector[CB_WRITE_COILS] = writeRSDATA;
    MBslave.cbVector[CB_READ_INPUT_REGISTERS] = readRSDATA;
    MBslave.cbVector[CB_READ_HOLDING_REGISTERS] = readRSDATA;
    MBslave.cbVector[CB_WRITE_HOLDING_REGISTERS] = writeRSDATA;
    MBslave.setUnitAddress(slave_ID);
    MBslave.begin(Baudrate);
    }
  void set_ID(short ID){
      slave_ID = ID;
  }
  void set_punch_out_time(short time){
    if(time <0 || time >3000){
      return;
    }
    this->punch_out_time = time;
  }
  void set_punch_out_stay(short time){
    if(time <0 || time >3000){
      return;
    }
    this->punch_out_stay = time;
  }
  void set_poll_back_time(short time){
    if(time <0 || time >5000){
      return;
    }
    this->poll_back_time = time;
  }
  void set_punch_delay_ms(short time){
    if(time <0 || time >3000){
      return;
    }
    this->punch_delay_ms = time;
  }
  void get_EEPROM_DATA(){
    uint8_t index =0;
    short temp_val=0;
    EEPROM.begin(EEP_size);
    auto a = EEPROM.readShort(0);
    if(a > 200 || a < 0){    //eeprom hasn't been written
        EEPROM.end();
        return;
    }
    else{
        for(auto v : RS_MAP_DATA){
            temp_val = EEPROM.readShort(index);
            if(temp_val < 0 || index > EEP_size ){
                index += 2;
                continue;
            }
            update_master_code(index/2 , temp_val);
            Serial.printf("read E *p=%d \r\n",*RS_MAP_DATA[index/2]);
            index += 2;
            if(index >20)
              break;
        }
    //    save_master_value();
    }
    EEPROM.end();
    }
  void save_master_value(){ 
      uint8_t index=0;
      EEPROM.begin(EEP_size);
      for (auto v : RS_MAP_DATA ){
          EEPROM.writeShort(index , *RS_MAP_DATA[index/2]);
          Serial.printf("write E *p=%d \r\n",*RS_MAP_DATA[index/2]);
          index += 2;
      }
      EEPROM.commit();
      EEPROM.end();
  }
  void print_all_verbs(){
        uint8_t i = 0;
        Serial.println("-----START PRINTING RS_DATA----");
        for(auto verb : RS_MAP_DATA){
            Serial.printf("p=%p; *p=%d \r\n",RS_MAP_DATA[i],*RS_MAP_DATA[i]);
            i++;
        }
    }

  void punch_out(){
    digitalWrite(valve_ad , 1);
    digitalWrite(valve_de , 0);
    timerAlarmWrite( timer1,  this->punch_out_time * 1000,  true);
    timerStart(timer1);
  }
  void poll_back(){
    digitalWrite(valve_ad , 0);
    digitalWrite(valve_de , 1);
    timerAlarmWrite( timer1,  this->poll_back_time * 1000,  true);
    timerStart(timer1);
  }
  void punch_stay(){
    digitalWrite(valve_ad , 0);
    digitalWrite(valve_de , 0);
    timerAlarmWrite( timer1,  this->punch_out_stay * 1000,  true);
    timerStart(timer1);
  }
  void punch_delay(){
    digitalWrite(valve_ad , 0);
    digitalWrite(valve_de , 0);
    timerAlarmWrite( timer1,  this->punch_delay_ms * 1000,  true);
    timerStart(timer1);
  }

};
Slave slave;

void update_master_code(short addr,short val){
  switch (addr){
    case 0:
      slave.set_ID(val);
      break;
    case 1:
      slave.set_punch_out_time(val);
      break;
    case 2:
      slave.set_punch_out_stay(val);
      break;
    case 3:
      slave.set_poll_back_time(val);
      break;
    case 4:
      slave.set_punch_delay_ms(val);
      break;
  }
}
void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(EEP_size);
  pinMode(valve_ad,OUTPUT);//val
  pinMode(valve_de,OUTPUT);
  pinMode(punch_key,INPUT);
  Serial.begin(115200);
  Serial2.begin(115200);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,1);
  slave.Map_DATA();
  slave.slave_init();
  slave.get_EEPROM_DATA();
  timer1 = timerBegin(1,80,true);//timer1 -> valve
  timerAttachInterrupt(timer1, &timer1_callback, true);
  timerAlarmWrite( timer1,  1000,  true);
  timerAlarmEnable(timer1);
  timerStop(timer1);
  timerWrite(timer1, 0);
  delay(1000);
  punch.punch_init();
  Serial.println("****PUNCH CTRL****");
  slave.print_all_verbs();
  esp_task_wdt_init(3,true);
}
int last_feed_time=0 ,key_last =0;
  void loop() {
    if(digitalRead(punch_key) == 1 && timerStarted(timer1)==0)    //valve act 
  {
    switch (punch_step_STA){
      case 0:
        Serial.println("punch_delay");
        slave.punch_delay();
        break;
      case 1:
        Serial.println("punch_out ");
        slave.punch_out();
        break;
      case 2:
      Serial.println("punch_stay ");
        slave.punch_stay();
        break;
      case 3:
      Serial.println("poll_back ");
        slave.poll_back();
    }
  }
  if(digitalRead(punch_key) == 0){

    if(millis() - key_last > 5){
      punch_step_STA = 0;
      timerStop(timer1);
      timerWrite(timer1, 0);
    }
  }
  else{
    key_last = millis();
  }

  if(save_STA){
    save_STA=0;
    slave.save_master_value();
  }

  punch.measure_once_upper();

  MBslave.poll();
  if((millis() - last_feed_time) > 2000){
    esp_task_wdt_add(NULL);
    esp_task_wdt_reset();
    last_feed_time = millis() ;
  }
}



