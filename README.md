# ESP32 LIBRARY FOR ICM20948 WITH ISP DMA

this library for 9-axis ccelerometer, gyroscope and magnetometer ICM20948,
it utilizing fast direct memory access (DMA) library provided by [hideakitai_DMA](https://github.com/hideakitai/ESP32DMASPI)
ensuring fast and non blocking cpu and RTOS friendly. good for drone or realtime proccesing system
 > [!CAUTION]
 > This code still prototype and have know issue that it still dont have reacycle feature
 > This feature will be implement for future update

 ### Supported ESP32 Version

| IDE         | ESP32 Board Version |
| ----------- | ------------------- |
| Arduino IDE | `>= 2.0.11`         |
| PlatformIO  | `>= 5.0.0`          |

 
 ## API
 ### SENSOR SETUP

 #### interupt 
 ```C++
//api to enable data ready interupt ( default is true)
  enable_interupt(true);
 ```
 ##### set polarity
<div align="center">
  <img src="https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/ACT%20HIGH.png" alt="Set_polarity_high" width="300">
</div>

 ```C++
   set_polarity_int(ICM20948_ACT_HIGH);
 ```
 <div align="center">
  <img src="https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/ACT%20LOW.png" alt="Set_polarity_low" width="300">
</div>

 ```C++
   set_polarity_int(ICM20948_ACT_LOW);
 ```


 #### accelerometer setup
 ##### set acc range

| Range setting                    | G |
|----------------------            | - |
| `ICM20948_ACC_RANGE_2G (default)`| 2 |
| `ICM20948_ACC_RANGE_4G`          | 4 |
| `ICM20948_ACC_RANGE_8G`          | 8 |
 ``` C++
   set_acc_range(ICM20948_ACC_RANGE_2G);

 ```
##### set acc dlpf
| DLPF SETTING | 3dB Bandwidth hz | Output Rate Hz |
|--------------|------------------|----------------|
| 0            |  246.0           | 1125/(1+ASRD)  |
| 1            |  246.0           | 1125/(1+ASRD)  |
| 2            |  111.4           | 1125/(1+ASRD)  |
| 3            |   50.4           | 1125/(1+ASRD)  |
| 4            |   23.9           | 1125/(1+ASRD)  |
| 5            |   11.5           | 1125/(1+ASRD)  |
| 6            |    5.7           | 1125/(1+ASRD)  |
| 7  (default) |  473.0           | 1125/(1+ASRD)  |
| OFF          | 1209.0           | 4500           |

```C++
set_acc_dlpf(ICM20948_DLPF_7);

```
> [!TIP]
> the data divider will be lock to disable if you turn off the DLPF
##### set data rate divider

![alt text](https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/quicklatex.com-30bd3464d130620ec59757d10947b7e5_l3-1.svg)

 ``` C++
  set_acc_data_divider(0);
 ```

 #### gyro setup
 ##### set gyr range

 | Range setting                       |  degrees per second  |
 |---------------------------          |----------------------|
 | `ICM20948_GYRO_RANGE_250 (default)` | 250                  |
 | `ICM20948_GYRO_RANGE_500`           | 500                  |
 | `ICM20948_GYRO_RANGE_1000`          | 1000                 |
 | `ICM20948_GYRO_RANGE_2000`          | 2000                 |

 ```C++ 
 set_gyr_range(ICM20948_GYRO_RANGE_250);
 ```
 ##### set gyr dlpf 
| DLPF        | 3dB Bandwidth hz   | Output Rate hz   |
|-------------|--------------------|------------------|
| 0 (default) |   196.6            | 1125/(1+GSRD)    |
| 1           |   151.8            | 1125/(1+GSRD)    |
| 2           |   119.5            | 1125/(1+GSRD)    |
| 3           |    51.2            | 1125/(1+GSRD)    |
| 4           |    23.9            | 1125/(1+GSRD)    |
| 5           |    11.6            | 1125/(1+GSRD)    |
| 6           |     5.7            | 1125/(1+GSRD)    |
| 7           |   361.4            | 1125/(1+GSRD)    |
| OFF         | 12106.0            | 9000             |

```C++
set_gyr_dlpf(ICM20948_DLPF_0);
```
##### set data rate divider
> [!NOTE]
> it behave just like the accelerometer
```C++
set_gyr_data_divider(0);
```

 #### magnetometer setup
| Mode                      | Description                                                                                          |
|---------------------------|------------------------------------------------------------------------------------------------------|
| `AK09916_PWR_DOWN`        | Power down to save energy                                                                            |
| `AK09916_TRIGGER_MODE`    | Measurements on request, a measurement is triggered by calling `setMagOpMode(AK09916_TRIGGER_MODE)`  |
| `AK09916_CONT_MODE_10HZ`  | Continuous measurements, 10 Hz rate                                                                  |
| `AK09916_CONT_MODE_20HZ`  | Continuous measurements, 20 Hz rate                                                                  |
| `AK09916_CONT_MODE_50HZ`  | Continuous measurements, 50 Hz rate                                                                  |
| `AK09916_CONT_MODE_100HZ` | Continuous measurements, 100 Hz rate (default)                                                       |
