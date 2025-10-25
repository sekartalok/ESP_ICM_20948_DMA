#include <Arduino.h>
#include "ICM2048DMA.h"


//test pin
#define SCL  12
#define SDA  35
#define NCS  34
#define ADO  36
ICM20948_DMA ICM(SCL, ADO, SDA, NCS);
volatile bool ready = false;



void IRAM_ATTR Testinterupt(){
  ready = true;
}
void sensor_read();
void setup() {
  Serial.begin(115200);
/*
    You must configure all sensor settings before calling begin(),
    as this function only updates the configuration state internally
    and does not immediately apply changes to the sensor hardware.
*/

//======================================= SETTING INTERUPT =======================================
//api to enable data ready interupt ( default is true)
  ICM.enable_interupt(true);
/*
  ICM20948_ACT_HIGH : IF THERE WAS INTERUPT THE INT LINE WILL RISING 
        __
  _____| |_____ 
  
  ICM20948_ACT_HIGH : IF THERE WAS INTERUPT THE INT LINE WILL FALLING

  _____  ______ 
      | |
      -- 


*/
  ICM.set_polarity_int(ICM20948_ACT_HIGH);


//======================================= SETTING ACCELEROMETER =======================================
  /*  ICM20948_ACC_RANGE_2G      2 g   (default)
   *  ICM20948_ACC_RANGE_4G      4 g
   *  ICM20948_ACC_RANGE_8G      8 g   
   *  ICM20948_ACC_RANGE_16G    16 g
   */

  ICM.set_acc_range(ICM20948_ACC_RANGE_2G);

  /*  Choose a level for the Digital Low Pass Filter or switch it off.  
   *  ICM20948_DLPF_0, ICM20948_DLPF_2, ...... ICM20948_DLPF_7, ICM20948_DLPF_OFF 
   *  
   *  IMPORTANT: This needs to be ICM20948_DLPF_7 if DLPF is used in cycle mode!
   *  
   *  DLPF       3dB Bandwidth [Hz]      Output Rate [Hz]
   *    0              246.0               1125/(1+ASRD) 
   *    1              246.0               1125/(1+ASRD)
   *    2              111.4               1125/(1+ASRD)
   *    3               50.4               1125/(1+ASRD)
   *    4               23.9               1125/(1+ASRD)
   *    5               11.5               1125/(1+ASRD)
   *    6                5.7               1125/(1+ASRD) 
   *    7              473.0               1125/(1+ASRD) (default)
   *    OFF           1209.0               4500
   *    
   *    ASRD = Accelerometer Sample Rate Divider (0...4095)
   *    You achieve lowest noise using level 6  
   */
  ICM.set_acc_dlpf(ICM20948_DLPF_7);

  /*  Acceleration sample rate divider divides the output rate of the accelerometer.
   *  Sample rate = Basic sample rate / (1 + divider) 
   *  It can only be applied if the corresponding DLPF is not off!
   *  Divider is a number 0...4095 (different range compared to gyroscope)
   *  If sample rates are set for the accelerometer and the gyroscope, the gyroscope
   *  sample rate has priority.
   * default is 0
   */
  // disable_divider_acc(true); to disable the divider
  ICM.set_acc_data_divider(0);

//======================================= SETTING GYRO =======================================

  /*  ICM20948_GYRO_RANGE_250       250 degrees per second (default)
   *  ICM20948_GYRO_RANGE_500       500 degrees per second
   *  ICM20948_GYRO_RANGE_1000     1000 degrees per second
   *  ICM20948_GYRO_RANGE_2000     2000 degrees per second
   */

  ICM.set_gyr_range(ICM20948_GYRO_RANGE_250);

  /*  Choose a level for the Digital Low Pass Filter or switch it off. 
   *  ICM20948_DLPF_0, ICM20948_DLPF_2, ...... ICM20948_DLPF_7, ICM20948_DLPF_OFF 
   *  
   *  DLPF       3dB Bandwidth [Hz]      Output Rate [Hz]
   *    0              196.6               1125/(1+GSRD)  (default)
   *    1              151.8               1125/(1+GSRD)
   *    2              119.5               1125/(1+GSRD)
   *    3               51.2               1125/(1+GSRD)
   *    4               23.9               1125/(1+GSRD)
   *    5               11.6               1125/(1+GSRD)
   *    6                5.7               1125/(1+GSRD) 
   *    7              361.4               1125/(1+GSRD)
   *    OFF          12106.0               9000
   *    
   *    GSRD = Gyroscope Sample Rate Divider (0...255)
   *    You achieve lowest noise using level 6  
   */
  ICM.set_gyr_dlpf(ICM20948_DLPF_0);

  // set the data rate divider
  // default 0
  // disable_divider_gyr(true); to disable the divider
  ICM.set_gyr_data_divider(0);

   /* You can set the following modes for the magnetometer:
   * AK09916_PWR_DOWN          Power down to save energy
   * AK09916_TRIGGER_MODE      Measurements on request, a measurement is triggered by 
   *                           calling setMagOpMode(AK09916_TRIGGER_MODE)
   * AK09916_CONT_MODE_10HZ    Continuous measurements, 10 Hz rate
   * AK09916_CONT_MODE_20HZ    Continuous measurements, 20 Hz rate
   * AK09916_CONT_MODE_50HZ    Continuous measurements, 50 Hz rate
   * AK09916_CONT_MODE_100HZ   Continuous measurements, 100 Hz rate (default)
   */
  ICM.set_mag_mode(AK09916_CONT_MODE_100HZ);
    /*
        DEBUG OUTPUT CODES:
        * -1: Sensor temperture is to low (less than -30.00)
        *  1: Sensor temperture is to high (more than 80.00)
        *  2: Total sensor failure or ICM initialization failure. Check the sensor connections.
        *  3: AK sensor failed to initialize. Try resetting or cycling the power.
        *  4: HEAP allocation for DMA is fail 
        *  0: Sensor is ready, and the system is automatically completing its configuration.

        NOTES:
        - If initialization fails without blocking, subsequent sensor reads may cause system errors 
            and potentially brick the ESP.

    */
  int test = ICM.begin();
  if(!test==0){
    Serial.print("SENSOR FAIL : ");
    Serial.println(test);
    while(1);
  }

  // this code must run after begin();
  ICM.clear_int();

}

void loop() {

  if(ready){
    ICM.clear_int();
    sensor_read();
    ICM.clear_int();
  }

 

}

void sensor_read(){
  xyzFloat acc;
  xyzFloat gyr;
  xyzFloat mag;
  float temperture;
  ICM.sensor_read();
  ICM.get_acc_raw(&acc);
  ICM.get_gyro_raw(&gyr);
  ICM.get_magneto_raw(&mag);
  temperture = ICM.get_temperature();

  Serial.println("Acceleration raw values :");
  Serial.print(acc.x);
  Serial.print("   ");
  Serial.print(acc.y);
  Serial.print("   ");
  Serial.println(acc.z);

  Serial.println("Gyroscope raw values :");
  Serial.print(gyr.x);
  Serial.print("   ");
  Serial.print(gyr.y);
  Serial.print("   ");
  Serial.println(gyr.z);

  Serial.println("Magneto raw values :");
  Serial.print(mag.x);
  Serial.print("   ");
  Serial.print(mag.y);
  Serial.print("   ");
  Serial.println(mag.z);

  Serial.println("Sensor temperture :");
  Serial.print(temperture);

}

