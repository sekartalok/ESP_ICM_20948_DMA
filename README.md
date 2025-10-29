# ESP32 LIBRARY FOR ICM20948 WITH ISP DMA

this library for 9-axis ccelerometer, gyroscope and magnetometer ICM20948,
it utilizing fast direct memory access (DMA) library provided by [hideakitai_DMA](https://github.com/hideakitai/ESP32DMASPI)
ensuring fast and non blocking cpu and RTOS friendly. good for drone or realtime proccesing system
 > [!CAUTION]
 > This code still prototype and have know issue that it still dont have reacycle feature
 > This feature will be implement for future update
 
 ## API
 ### SENSOR SETUP

 interupt 
 ```C++
 /======================================= SETTING INTERUPT =======================================
//api to enable data ready interupt ( default is true)
  enable_interupt(true);
/*
  ICM20948_ACT_HIGH : IF THERE WAS INTERUPT THE INT LINE WILL RISING 
        __
  _____| |_____ 
  
  ICM20948_ACT_HIGH : IF THERE WAS INTERUPT THE INT LINE WILL FALLING

  _____  ______ 
      | |
      -- 


*/
  set_polarity_int(ICM20948_ACT_HIGH);

 ```