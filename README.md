# ESP32 LIBRARY FOR ICM20948 WITH ISP DMA

This library is for the **9-axis accelerometer, gyroscope, and magnetometer (ICM20948)**.  
It utilizes the fast **Direct Memory Access (DMA)** library provided by [hideakitai/ESP32DMASPI](https://github.com/hideakitai/ESP32DMASPI),  
ensuring high-speed, non-blocking CPU performance and RTOS-friendly operation.  

Perfect for **drones** and other **real-time processing systems**.

> [!CAUTION]
> This code is still a prototype and has known issues.  
> The recycle feature is not yet implemented and will be added in a future update.


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
 (void)enable_interupt(true);
 ```
 ##### set polarity
<div align="center">
  <img src="https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/ACT%20HIGH.png" alt="Set_polarity_high" width="300">
</div>

 ```C++
   (void)set_polarity_int(ICM20948_ACT_HIGH);
 ```
 <div align="center">
  <img src="https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/ACT%20LOW.png" alt="Set_polarity_low" width="300">
</div>

 ```C++
   (void)set_polarity_int(ICM20948_ACT_LOW);
 ```


 #### accelerometer setup
 ##### set acc range

| Range setting                    | G |
|----------------------            | - |
| `ICM20948_ACC_RANGE_2G (default)`| 2 |
| `ICM20948_ACC_RANGE_4G`          | 4 |
| `ICM20948_ACC_RANGE_8G`          | 8 |
 ``` C++
   (void)set_acc_range(ICM20948_ACC_RANGE_2G);

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
(void)set_acc_dlpf(ICM20948_DLPF_7);

```
> [!TIP]
> the data divider will be lock to disable if you turn off the DLPF
##### set data rate divider

![alt text](https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/quicklatex.com-30bd3464d130620ec59757d10947b7e5_l3-1.svg)

 ``` C++
(void)set_acc_data_divider(0);
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
(void)set_gyr_range(ICM20948_GYRO_RANGE_250);
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
(void)set_gyr_dlpf(ICM20948_DLPF_0);
```
##### set data rate divider
> [!NOTE]
> it behave just like the accelerometer
```C++
(void)set_gyr_data_divider(0);
```

 #### magnetometer setup
| Mode                      | Description                                                                                          |
|---------------------------|------------------------------------------------------------------------------------------------------|
| `AK09916_PWR_DOWN`        | Power down to save energy                                                                            |
| `AK09916_TRIGGER_MODE`    | Measurements on request, a measurement is triggered by calling setMagOpMode(AK09916_TRIGGER_MODE)`  |
| `AK09916_CONT_MODE_10HZ`  | Continuous measurements, 10 Hz rate                                                                  |
| `AK09916_CONT_MODE_20HZ`  | Continuous measurements, 20 Hz rate                                                                  |
| `AK09916_CONT_MODE_50HZ`  | Continuous measurements, 50 Hz rate                                                                  |
| `AK09916_CONT_MODE_100HZ` | Continuous measurements, 100 Hz rate (default)                                                       |
```C++
(void)set_mag_mode(AK09916_CONT_MODE_100HZ);
```
### SENSOR BEGIN
> [!IMPORTANT]
> make sure you done with setup because any setup function will not work after this

##### ERROR CODE AND WHAT IT MEAN

| Error code  |  explanation                   |
|-------------|------------------------------- |
|  -1         | Sensor is to cold `>= -30`     |
|   1         | Sensor is to hot  `<= 80 `     |
|   2         | Total sensor failure when init |
|   3         | AK09916 / I2C MASTER FAIL INIT |
|   4         | HEAP alloc fail dma is not run |
|   0         | Sensor is good to go           |
> [!WARNING]
> make sure you properly restart or block the code if you ecounter 
> 2, 3, 4 error or your esp will error (this will be fix in future update)

```C++
(int)begin();
```

### SENSOR OPERATION

#### clear interupt
```C++
(void)clear_int();  
```
>[!TIP]
> its recomend to put any update that inside interupt IRAM ATTR before clear_int()

### SENSOR READ
#### to read data from ICM sensor
```C++
(void)sensor_read();
```
#### to get the data from buffer
``` C++
  xyzFloat acc;
  xyzFloat gyr;
  xyzFloat mag;

  (void)get_acc_raw(&acc); // to get acc data
  (void)get_gyro_raw(&gyr); // to get gyr data
  (void)get_magneto_raw(&mag); // to get mag data

```
```C++
acc.x // to get x value
acc.y // to get y value
acc.z // to get z value
```
```C++
(float)get_temperature() // tp get icm temperature sensor
```

## GPIO THAT USE FOR TEST:

 <div align="center">
  <img src="https://github.com/sekartalok/ESP_ICM_20948_DMA/blob/main/resource/ICM%20SENSOR.png" alt="Set_polarity_low" width="300">
</div>

# OTHER DOCUMENT
[ICM20948_data_sheet](https://invensense.tdk.com/wp-content/uploads/2016/06/DS-000189-ICM-20948-v1.3.pdf)
[other_refrence](https://wolles-elektronikkiste.de/en/icm-20948-9-axis-sensor-part-i)