#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "ICM20948DMA.h"


//test pin
#define SCL  12
#define SDA  35
#define NCS  34
#define ADO  36
#define INT  48

struct lcd_var {
  unsigned int screen_width = 128;
  unsigned int screen_height = 64;
  int oled_reset = -1;
  unsigned int oled_sda = 18;
  unsigned int oled_scl = 17;
};
static const lcd_var lcd;

TaskHandle_t task1;
TaskHandle_t task2;

static SemaphoreHandle_t mutex;

ICM20948_DMA ICM(SCL, ADO, SDA, NCS);
Adafruit_SSD1306 display(lcd.screen_width, lcd.screen_height, &Wire, lcd.oled_reset);


volatile bool ready = false;

void IRAM_ATTR Testinterupt(){
  xTaskNotifyGive(task1);
}

void sensor_master(){
  
  ICM.clear_int();
  ICM.sensor_read();
  ICM.clear_int();
  
}


int cpu_usage(){return 100 - ulTaskGetIdleRunTimePercent();}


void lcd_write(){
 // xyzFloat acc;
 // xyzFloat gyr;
 // xyzFloat mag;
 // float temperture;

//  ICM.get_acc_raw(&acc);
 // ICM.get_gyro_raw(&gyr);
 // ICM.get_magneto_raw(&mag);
 // temperture = ICM.get_temperature();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("CPU USAGE :");
  display.println(cpu_usage());

  display.display();



}

bool display_status() {
  Wire.begin(lcd.oled_sda, lcd.oled_scl);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }
  return true;
}

void task_1(void *pvParameters){
  while(1){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    sensor_master();
    
  }
}
void task_2(void *pvParameters){
  while(1){
     lcd_write();
     vTaskDelay(500 / portTICK_PERIOD_MS);
  }

}

void setup() {
  Serial.begin(9600);
  if(ICM.begin() != 0){
    Serial.println("SENSOR FAIL TO INIT");
    while(1);
  }
  Serial.println("SENSOR WORKING");
  if(!display_status()){
    Serial.println("display fail");
  }
  mutex = xSemaphoreCreateMutex();

  
  attachInterrupt(digitalPinToInterrupt(INT), Testinterupt, RISING);
  ICM.clear_int();

 xTaskCreatePinnedToCore( task_1,"s_read",2000,NULL,2,&task1,0);
 xTaskCreatePinnedToCore( task_2,"l_write",2000,NULL,1,&task2,0);


}

void loop() {




}
