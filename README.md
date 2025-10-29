# ESP32 LIBRARY FOR ICM20948 WITH ISP DMA

this library for 9-axis ccelerometer, gyroscope and magnetometer ICM20948,
it utilizing fast direct memory access (DMA) library provided by [hideakitai_DMA](https://github.com/hideakitai/ESP32DMASPI)
ensuring fast and non blocking cpu and RTOS friendly. good for drone or realtime proccesing system
 > [!CAUTION]
 > This code still prototype and have know issue that it still dont have reacycle feature
 > This feature will be implement for future update
 
 ## API
 ### SENSOR SETUP

 #### interupt 
 ```C++
//api to enable data ready interupt ( default is true)
  enable_interupt(true);
 ```
 ##### set polarity
<div align="center">
  <img src="https://cdn.discordapp.com/attachments/1195732459577417790/1432944490821324872/ACT_HIGH.png?ex=6902e50d&is=6901938d&hm=de68ad532ec34620467dd627b5ae6d210b6e05b535eec3bdf1c29efadaad1d16&" alt="Set_polarity_high" width="300">
</div>

 ```C++
   set_polarity_int(ICM20948_ACT_HIGH);
 ```
 #### accelerometer setup

 ``` C++
 //======================================= SETTING ACCELEROMETER =======================================
  /*  ICM20948_ACC_RANGE_2G      2 g   (default)
   *  ICM20948_ACC_RANGE_4G      4 g
   *  ICM20948_ACC_RANGE_8G      8 g   
   *  ICM20948_ACC_RANGE_16G    16 g
   */

  set_acc_range(ICM20948_ACC_RANGE_2G);

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
   *    DATA DIVIDER WILL DISABLE WHEN YOU TURN OFF THIS FEATURE
   */
  set_acc_dlpf(ICM20948_DLPF_7);

  /*  Acceleration sample rate divider divides the output rate of the accelerometer.
   *  Sample rate = Basic sample rate / (1 + divider) 
   *  It can only be applied if the corresponding DLPF is not off!
   *  Divider is a number 0...4095 (different range compared to gyroscope)
   *  If sample rates are set for the accelerometer and the gyroscope, the gyroscope
   *  sample rate has priority.
   * default is 0
   */
  // disable_divider_acc(true); to disable the divider
  set_acc_data_divider(0);

 ```
 > [!TIP]
 > the data divider will be lock to disable if you turn off the DLPF