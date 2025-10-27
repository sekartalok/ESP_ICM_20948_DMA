/* 
------------------------------------------------------------------------------------------------------
LIBRARY FOR 9 AXIS SENSOR ICM 20948 USING DMA (DIRECT MEMORY ACCES)

BY SEKARTALOK

- MAIN COMUNICATION : SPI / FULL DUPLEX 

- USING ICM 20948_WE FOR REFERENCE (https://github.com/wollewald/ICM20948_WE/tree/main)

- USING hideakitai ESPDMASPI LIB WITH SOME ADJUSTMENT FOR THIS CODE (https://github.com/hideakitai/ESP32DMASPI)

- FEEL FREE TO USE IT BUT AT YOUR OWN RISK 

- FIRST MADE 10 / 24 / 2025 DD/MM/YY 

- LAST UPDATE 10 / 25 /2025 DD/MM/YY (REDESIGN BEGIN FOR MORE INTEGRATE SETUP)

- ERROR NOTE:


------------------------------------------------------------------------------------------------------
HAPPY CODING ▼・ᴗ・▼
*/

#ifndef ICM_20948_DMA
#pragma once 

#define ICM_20948_DMA
#include <Arduino.h>  
#include "xyzFloat.h"
#include "ESP32DMASPIMaster.h"

// user interface api
typedef enum AK09916_OP_MODE {
    AK09916_PWR_DOWN           = 0x00,
    AK09916_TRIGGER_MODE       = 0x01,
    AK09916_CONT_MODE_10HZ     = 0x02,
    AK09916_CONT_MODE_20HZ     = 0x04,
    AK09916_CONT_MODE_50HZ     = 0x06,
    AK09916_CONT_MODE_100HZ    = 0x08
} AK09916_opMode;

typedef enum ICM20948_ACC_RANGE {
    ICM20948_ACC_RANGE_2G, ICM20948_ACC_RANGE_4G, ICM20948_ACC_RANGE_8G, ICM20948_ACC_RANGE_16G
} ICM20948_accRange;

typedef enum ICM20948_GYRO_RANGE {
    ICM20948_GYRO_RANGE_250, ICM20948_GYRO_RANGE_500, ICM20948_GYRO_RANGE_1000, ICM20948_GYRO_RANGE_2000
} ICM20948_gyroRange;

typedef enum ICM20948_DLPF {
    ICM20948_DLPF_0, ICM20948_DLPF_1, ICM20948_DLPF_2, ICM20948_DLPF_3, ICM20948_DLPF_4, ICM20948_DLPF_5, 
    ICM20948_DLPF_6, ICM20948_DLPF_7, ICM20948_DLPF_OFF
} ICM20948_dlpf;

typedef enum ICM20948_INT_PIN_POL {
    ICM20948_ACT_HIGH, ICM20948_ACT_LOW
} ICM20948_intPinPol;


class ICM20948_DMA {
private:


    //DMA SPI
    ESP32DMASPI::Master * DMASPI;
    uint8_t *dma_tx_buf{nullptr};
    uint8_t *dma_rx_buf{nullptr};

    //SPI BLOCKING
    SPIClass * SPI;
    SPISettings spi_setting;


    // Pin assignments
    int cs_pin;
    int sda_pin;
    int scl_pin;
    int ado_pin;
    
    // Bank and data buffer
    uint8_t current_bank{0};
    uint8_t buffer[20];

    //sensor setup with default setting 
    uint8_t dlpf_gyr_var{0};
    uint8_t dlpf_acc_var{7};
    uint8_t range_gyr_var{0};
    uint8_t range_acc_var{0};
    uint8_t mode_mag_var{8};
    uint16_t divider_acc_var{0};
    uint8_t divider_gyr_var{0};
    bool disable_divider_acc_var{false};
    bool disable_divider_gyr_var{false};

    //interupt
    uint8_t interupt_polarity{0};
    bool interupt_enable{true};

    //DMA SWITCH
    void enable_dma();
    

    //DMA READ OPERATION ONLY FOR READ AND CLEAR
     
    void spi_dma(size_t size);

    //SPI INNIT OPERATION 
    uint8_t read8(uint8_t bank, uint8_t reg);
    void write8(uint8_t bank, uint8_t reg, uint8_t val);
    uint16_t read16(uint8_t bank, uint8_t reg);
    void write16(uint8_t bank, uint8_t reg,int16_t val);
    //PWR CTRL

    void sleep(bool con);

    //SWITCH BANK
    void switch_bank(uint8_t bank);


    //ICM20948
    bool ICM20948_begin();
    void reset_ICM20948();
    void enable_gyr(bool con);
    void enable_acc(bool con);
    uint8_t whoami_ICM20948();

    //SENSOR SETUP
    void acc_range(uint8_t range);
    void gyr_range(uint8_t renge);
    void mag_mode(uint8_t set);
    void acc_dlpf(uint8_t dlpf);
    void gyr_dlpf(uint8_t dlpf);
    void acc_data_divider(uint8_t data);
    void gyr_data_divider(uint8_t data);
    void disable_divider_acc(bool con);
    void disable_divider_gyr(bool con);







    //AK09916

    //LOW LEVEL DATA TRANSFER I2C MASTER
    uint8_t AK09916_read8(uint8_t reg);
    void AK09916_write8( uint8_t reg, uint8_t val);

    //I2C MASTER CONTROLLER
    void enable_I2C_master();
    void disable_I2C_master();
    void reset_I2C_master();

    //AK09916 MASTER OPERATIONS
    bool magnetor_begin();
    void reset_AK09916();
    uint16_t whoami_AK09916();
    

    //DATA READ
    void read_all();


    //INTERUPT 
    void enable_int_latch(bool enable);
    void pin_polarity(bool polarity);
    void enable_data_ready();
   // void clear_int();


   

    

   

public:
    //class management 
    ICM20948_DMA(int scl,int ado,int sda,int cs);
    ~ICM20948_DMA();

    // sensor control master

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

        ------------------------------------------------------------------------------------------------------------------------

        END COMMAND:
        - Calling the end function is not recommended for advanced mechanisms such as drones or boats,
            as it consumes significant CPU resources and may block the RTOS or other system processes.
        - Use this only when absolutely necessary, such as during a system crash or total failure.
        - Deallocating this class has the same effect as calling end().
    */



    int begin();
    void end();



    //sensor set range
    void set_acc_range(ICM20948_accRange range);
    void set_gyr_range(ICM20948_gyroRange range);

    //sensor set dlpf
    void set_acc_dlpf(ICM20948_dlpf dlpf);
    void set_gyr_dlpf(ICM20948_dlpf dlpf);

    //sensor data divider
    void set_acc_data_divider(uint8_t data);
    void set_gyr_data_divider(uint8_t data);

    //mag mode 
    void set_mag_mode(uint8_t set);

    /*
        - this area is require heap to be allocate or the esp may crash because violate memory acces
        - there is no if checking after this line for fast dma read
        - make sure the begine is end with 0 or -1 and 1 
        -----------------------------------------------------------------------------------------------------    
        NOTE: this area is lock to bank 0 and do not change the sensor bank manualy if you want to run this line
    
    */

    //sensor read and get
    void sensor_read();
    void get_gyro_raw(xyzFloat * gyro);
    void get_acc_raw(xyzFloat * acc);
    void get_magneto_raw(xyzFloat * magneto);
    float get_temperature();

    //interupt set
    void enable_interupt(bool en);
    void set_polarity_int(ICM20948_intPinPol status);
    void clear_int();

};
#endif




