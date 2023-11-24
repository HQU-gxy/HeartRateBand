#ifndef _PUNCH_H
#define _PUNCH_H
#include "Arduino.h"
#include "HX711.h"
#define buf_size 50
HX711 Hx711;
#define u8 uint8_t
#define u16 uint16_t
class Punch{
  private:
    byte DOUT = 34 ,PD_SCK = 14 ;
  public:
    u8 measure_cnt =0; 
    int per_kg = 25600;
    float punch_power =0;
    float measure_buf[buf_size];
    void punch_init(){
      Hx711.begin(DOUT,PD_SCK);
      Hx711.wait_ready_retry(10, 200);
      if(Hx711.is_ready()){
        Hx711.tare(10);
        Serial.println("bridge is ready and tared");
      }
      else {
        Serial.println("bridge is not ready");
      }
    }
    void measure_once_upper(){
        float temp =  Hx711.get_units(1);
        if(temp > this->per_kg){
            punch_power = temp/this->per_kg;
            measure_buf[measure_cnt] = punch_power;
            Serial.printf("punch : %.2f kg\r\n",measure_buf[measure_cnt]);
            measure_cnt ++;
        }
      if(measure_cnt == buf_size){
        measure_cnt =0;
      }
    }
    void measure_times(u8 times){
      measure_buf[measure_cnt] = Hx711.get_units(3);
      measure_cnt ++;
      if(measure_cnt == buf_size){
        measure_cnt =0;
        this->send_buf(times);
      }
    }
    void send_buf(u8 times){
        float sum_temp =0,average_temp=0;
        for(u8 i =0 ;i<times ;i++){
            Serial.printf("%.3f ", measure_buf[i]);
            sum_temp += measure_buf[i];
        }
        Serial.printf("\r\naverage :%.3f \r\n",sum_temp/times);
    }
    void set_per_kg(int kg){
        if(kg<40000 && kg >10000){
            this->per_kg = kg;
        }
    }
};


#endif