#include "ICM2048DMA.h"

ICM20948_DMA::ICM20948_DMA(int scl,int ado,int sda,int cs): scl_pin(scl), ado_pin(ado), sda_pin(sda), cs_pin(cs){
    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH);

    SPI = new SPIClass(1);
}

// Destructor
ICM20948_DMA::~ICM20948_DMA() {

    if(SPI){
        SPI->end();
        delete SPI;
        SPI = nullptr;

    }
    if (DMASPI) {
        DMASPI->end();   
        delete DMASPI;   
        DMASPI = nullptr;
    }
    if (dma_tx_buf) {
        heap_caps_free(dma_tx_buf);
        dma_tx_buf = nullptr;
    }
    if (dma_rx_buf) {
        heap_caps_free(dma_rx_buf);
        dma_rx_buf = nullptr;
    }
}
//switch bank
void ICM20948_DMA::switch_bank(uint8_t bank){
    if(current_bank == bank){ return; }

    current_bank = bank;
    digitalWrite(cs_pin, LOW);
    SPI->beginTransaction(spi_setting);
        

    SPI->transfer(0x7F & 0x7F);
    SPI->transfer((bank & 0x03 ) << 4);

        
    SPI->endTransaction();
    digitalWrite(cs_pin, HIGH);
    
}

//SPI INNIT OPERATION 
void ICM20948_DMA::write8(uint8_t bank, uint8_t reg, uint8_t val){
    switch_bank(bank);
    digitalWrite(cs_pin, LOW);
    SPI->beginTransaction(spi_setting);
    
    SPI->transfer(reg & 0x7F); // write mask
    SPI->transfer(val);

    SPI->endTransaction();
    digitalWrite(cs_pin, HIGH);
}

uint8_t ICM20948_DMA::read8(uint8_t bank, uint8_t reg){
    switch_bank(bank);
    uint8_t Return;
    digitalWrite(cs_pin, LOW);
    SPI->beginTransaction(spi_setting);

    SPI->transfer(reg | 0x80); // read mask
    Return = SPI->transfer(0x00);

    SPI->endTransaction();
    digitalWrite(cs_pin, HIGH);
    return Return;
}
// 16 BIT R/W USING 2X 8BIT DATA SPI
void ICM20948_DMA::write16(uint8_t bank, uint8_t reg,int16_t val){
    switch_bank(bank);
    uint8_t ADDH = ((val >> 8) & 0xFF);
    uint8_t ADDL = val & 0xFF;
    digitalWrite(cs_pin,LOW);
    SPI->beginTransaction(spi_setting);
    
    SPI->transfer(reg & 0x07F); //write masking
    SPI->transfer(ADDH);
    SPI->transfer(ADDL);

    SPI->endTransaction();
    digitalWrite(cs_pin,HIGH);
}

//I2C MASTER R/W
uint8_t ICM20948_DMA::AK09916_read8(uint8_t reg){
    switch_bank(3);
    write8(3,0x13,0x0C | 0x80); // set to read mode
    write8(3,0x14,reg);
    write8(3,0x15,0x80);
    delay(10);

    uint32_t start_millis = millis();
    while(read8(3,0x15 & 0x80)){
        if(millis() - start_millis < 100){
            break;
        }
    }
    return read8(3,0x17);

}

void ICM20948_DMA::AK09916_write8( uint8_t reg, uint8_t val){
     switch_bank(3);
    write8(3,0x13,0x0C); // set to write mode
    write8(3,0x16,val);
    write8(3,0x14,reg);
    write8(3,0x15,0x80);
    delay(10);

    uint32_t start_millis = millis();
    while(read8(3,0x15 & 0x80)){
        if(millis() - start_millis < 100){
            break;
        }
    }

}



uint16_t ICM20948_DMA::read16(uint8_t bank, uint8_t reg){
    uint16_t Returns;
    uint8_t ADDH = 0x00; // return all to 0x00 if fail to read 
    uint8_t ADDL = 0x00;

    digitalWrite(cs_pin,LOW);
    SPI->beginTransaction(spi_setting);
    SPI->transfer(reg | 0x80); //read masking
    ADDH = SPI->transfer(0x00);
    ADDL = SPI->transfer(0x00);
    SPI->endTransaction();
    digitalWrite(cs_pin,HIGH);


    Returns = (ADDH << 8) | ADDL;
    return Returns;

}
//SPI MASTER SENDER

void ICM20948_DMA::spi_dma(size_t size){
    size_t lenght = (size + 3) & ~0x03;// 4byte is missing SPI SLAVE
    DMASPI->queue(dma_tx_buf, dma_rx_buf, lenght);
    DMASPI->trigger();

}
//PWR CTRL
void ICM20948_DMA::sleep(bool con){
    uint8_t temp = read8(0,0x06);
    if(con){
        temp |= 0x40;
    }else{
        temp &= ~0x40;
    }
    write8(0,0x06,temp);
}

    
void ICM20948_DMA::enable_gyr(bool con){
    uint8_t temp = read8(0,0x07);
    if(con){
        temp &= ~ 0x07;
    }else{
        temp |= 0x07;
    }
    write8(0,0x07,temp);

}

void ICM20948_DMA::enable_acc(bool con){
    uint8_t temp = read8(0,0x07);
    if(con){
        temp &= ~ 0x38;
    }else{
        temp |= 0x38;
    }
    write8(0,0x07,temp);
}
//GENERAL SETUP FOR ALL 
/*
  THE MAGNETOR OR AK SENSOR CANT BE DISABLE IN THIS CODE, YOU MUST ADJUST THE ICM2048 BEGINE TO ENABLE SENSOR READING
  IF YOU REMOVE THE MAGNETOR_BEGIN WITHOUT ADJUSTING THE WHOLE SENSOR JUST SENDING 0.00 READING FOR SYSTEM INTEGRATION REASON 
  WE DECIDE TO IMPLEMENT THIS FEATURE
*/

int ICM20948_DMA::begin(){
    SPI->begin(scl_pin, ado_pin, sda_pin, cs_pin);
    spi_setting = SPISettings(1000000, MSBFIRST, SPI_MODE0);
    delay(200);
    //init the ICM
    if(!ICM20948_begin()){
        end();
        return 2;
    }
    if(!magnetor_begin()){
        end();
        return 3;
    }

    //to make sure all primary ICM sensor is enable

    enable_gyr(true);
    enable_acc(true);

    //set range and mode 
    
    acc_range(range_acc_var);
    gyr_range(range_gyr_var);
    mag_mode(mode_mag_var);
    acc_dlpf(dlpf_acc_var);
    gyr_dlpf(dlpf_gyr_var);

    if(!disable_divider_acc_var){
        acc_data_divider(divider_acc_var);
    }

    if(!disable_divider_gyr_var){
        gyr_data_divider(divider_gyr_var);
    }

    //set interupt
    if(interupt_enable){
        pin_polarity(interupt_polarity);
        enable_int_latch(true);
        enable_data_ready();
    }
    
    switch_bank(0);
    delay(50);
    enable_dma();
    delay(20);

    //dma alloc fail
    if(!DMASPI){
        end();
        return 4;
    }
    if(!dma_rx_buf){
        end();
        return 4;
    }
    if(!dma_tx_buf){
        end();
        return 4;
    }

    //check temperture ideal working condition
    float temperture;
    read_all();
    delay(20);
    read_all();
    temperture = get_temperature();
    //temperture is to low 
    if(temperture <= -30.00){return -1;}
    //temperture is to high
    if(temperture >= 80.00){return 1;}


    return 0;
}



void ICM20948_DMA::end(){
    spi_setting = SPISettings();
    if(SPI){
        delay(50);
        SPI->end();
        delete SPI;
        SPI =nullptr;
        delay(10);
    } 
   delay(50);

    if (DMASPI) {
        DMASPI->end();   
        delete DMASPI;   
        DMASPI = nullptr;
    }
    if (dma_tx_buf) {
        heap_caps_free(dma_tx_buf);
        dma_tx_buf = nullptr;
    }
    if (dma_rx_buf) {
        heap_caps_free(dma_rx_buf);
        dma_rx_buf = nullptr;
    }
}
//DMA ENABLE 
void ICM20948_DMA::enable_dma(){
    //disable the SPI

    spi_setting = SPISettings();
    if(SPI){
        delay(50);
        SPI->end();
        delete SPI;
        SPI =nullptr;
        delay(10);
    }

    delay(50);

    
    digitalWrite(cs_pin, HIGH);
    DMASPI = new ESP32DMASPI::Master();
    DMASPI->begin(HSPI, scl_pin, ado_pin, sda_pin, cs_pin);
    dma_tx_buf = DMASPI->allocDMABuffer(400);
    dma_rx_buf = DMASPI->allocDMABuffer(400);

    delay(10);

    DMASPI->setDataMode(SPI_MODE0);
    DMASPI->setFrequency(1000000);  
    DMASPI->setMaxTransferSize(400);
    DMASPI->setQueueSize(1);

    delay(10);

  




}

//ICM2048DMA BEGINE
uint8_t ICM20948_DMA:: whoami_ICM20948(){
    return read8(0,0x00);
}

void ICM20948_DMA::reset_ICM20948(){
    write8(0,0x06,0x80);
    delay(100);
}

bool ICM20948_DMA::ICM20948_begin(){
    reset_ICM20948();
    uint8_t tries = 0;
    while(tries <  10 ){
        reset_ICM20948();
        if (whoami_ICM20948() == 0xEA){return 1;}
        delay(100);
        tries++;
    }
    
    return 0;

}

//AK09916 BEGINE 

void ICM20948_DMA::enable_I2C_master(){
    write8(0,0x03,0x20);
    write8(3,0x01,0x07);
}
void ICM20948_DMA::disable_I2C_master(){
    uint8_t temp = read8(0,0x03);
    temp &= ~0x20;
    write8(0,0x03,temp);
}
void ICM20948_DMA::reset_I2C_master(){
   uint8_t temp = read8(0,0x03) ;
   temp |= 0x02;
   write8(0,0x03,temp);
   delay(100);

}
void ICM20948_DMA::reset_AK09916(){
    AK09916_write8(0x32,0x01);
}

uint16_t ICM20948_DMA::whoami_AK09916(){
    uint8_t ADDH_byte = AK09916_read8(0x00);
    uint8_t ADDL_byte = AK09916_read8(0x01);
    return (ADDH_byte << 8) | ADDL_byte;
}

bool ICM20948_DMA::magnetor_begin(){

    reset_ICM20948();
    delay(100);
    enable_I2C_master();
    delay(10);
    reset_AK09916();
    delay(100);
    sleep(false);
    write8(2,0x09,1);

    uint16_t who = 0;
    uint8_t tries = 0;
    while(tries < 10){
        delay(10);
        enable_I2C_master();
        delay(10);
        who = whoami_AK09916();
        if(who == 0x4809 || who ==  0x0948){
            return 1;
        }
        reset_I2C_master();
    
        tries++;
    }
    return 0;
}


//SENSOR SETUP

//SET RANGE

void ICM20948_DMA::set_acc_range(ICM20948_accRange range){
    range_acc_var = range;
}
void ICM20948_DMA::set_gyr_range(ICM20948_gyroRange range){
    range_gyr_var = range;
}
void ICM20948_DMA::acc_range(uint8_t range){
    uint8_t temp = read8(2,0x14);
    temp &= ~0x06;
    temp |= (range<<1);
    write8(2,0x14,temp);
}

void ICM20948_DMA::gyr_range(uint8_t range){
    uint8_t temp = read8(2,0x01);
    temp &= ~0x06;
    temp |= (range<<1);
    write8(2,0x01,temp);
}

//DLPF
void ICM20948_DMA::disable_divider_acc(bool con){
    disable_divider_acc_var = con;

}
void ICM20948_DMA::disable_divider_gyr(bool con){
    disable_divider_gyr_var = con;

}

void ICM20948_DMA::set_acc_dlpf(ICM20948_dlpf dlpf){
    dlpf_acc_var = dlpf;
}
void ICM20948_DMA::set_gyr_dlpf(ICM20948_dlpf dlpf){
    dlpf_gyr_var = dlpf;
}

void ICM20948_DMA::acc_dlpf(uint8_t dlpf){
    uint temp = read8(2,0x14);
    if(dlpf!=8){
        temp |= 0x01;
        temp &= 0xC7;
        temp |= (dlpf << 3);

    }else{
        disable_divider_acc_var = true;
        temp &= 0xFE;
    }
    write8(2,0x14,temp);

}
void ICM20948_DMA::gyr_dlpf(uint8_t dlpf){

    uint temp = read8(2,0x01);
    if(dlpf!=8){
        temp |= 0x01;
        temp &= 0xC7;
        temp |= (dlpf << 3);

    }else{
        disable_divider_gyr_var = true;
        temp &= 0xFE;
    }
    write8(2,0x01,temp);

}
//mag mode

void ICM20948_DMA::set_mag_mode(uint8_t set){
    mode_mag_var = set;

}

void ICM20948_DMA::mag_mode(uint8_t set){
    AK09916_write8(0x31,set);
    if(set != 0x00){
        write8(3,0x03,0x0C | 0x80);//set to read mode
        write8(3,0x04,0x11); //set HXL
        write8(3,0x05,0x88);
    }
}
//DATA DIVIDER
void ICM20948_DMA::set_acc_data_divider(uint8_t data){
    divider_acc_var = data;
}
void ICM20948_DMA::set_gyr_data_divider(uint8_t data){
    divider_gyr_var = data;
}


void ICM20948_DMA::acc_data_divider(uint8_t data){
    write16(2,0x10,data);
}
void ICM20948_DMA::gyr_data_divider(uint8_t data){
    write8(2,0x00,data);
}



//DATA READ
void ICM20948_DMA::read_all(){


    uint8_t reg = 0x2D | 0x80;
    dma_tx_buf[0] =reg;
    int i = 1;
    while (i <= 20){
        dma_tx_buf[i] = 0x00;
        i++;
    }
    spi_dma(21);

       i = 0;
    while(i<20){
        buffer[i] = dma_rx_buf[i+1];
        i++;
    }

}

void ICM20948_DMA::sensor_read(){
    read_all();
}

void ICM20948_DMA::get_acc_raw(xyzFloat * acc){
    acc->x = static_cast<int16_t>(((buffer[0]) << 8) | buffer[1]);
    acc->y = static_cast<int16_t>(((buffer[2]) << 8) | buffer[3]);
    acc->z = static_cast<int16_t>(((buffer[4]) << 8) | buffer[5]);

}

void ICM20948_DMA::get_gyro_raw(xyzFloat * gyro){
    gyro->x = static_cast<int16_t>(((buffer[6]) << 8) | buffer[7]);
    gyro->y = static_cast<int16_t>(((buffer[8]) << 8) | buffer[9]);
    gyro->z = static_cast<int16_t>(((buffer[10]) << 8) | buffer[11]);

}

float ICM20948_DMA::get_temperature(){
    int16_t rawTemp = static_cast<int16_t>(((buffer[12]) << 8) | buffer[13]);
    return (rawTemp*1.0 - 0.0)/333.87 + 21.0f;

}

void ICM20948_DMA::get_magneto_raw(xyzFloat * magneto){
    magneto->x = static_cast<int16_t>((buffer[15]) << 8) | buffer[14];
    magneto->y = static_cast<int16_t>((buffer[17]) << 8) | buffer[16];
    magneto->z = static_cast<int16_t>((buffer[19]) << 8) | buffer[18];
}

//INTERUPT 

void ICM20948_DMA::enable_interupt(bool en){
    interupt_enable = en;
}
void ICM20948_DMA:: set_polarity_int(ICM20948_intPinPol status){
    interupt_polarity = status;
}
void ICM20948_DMA:: enable_int_latch(bool enable){
    uint8_t temp = read8(0,0x0F);
    if(!enable){
        temp &= ~0x20;

    }else{
        temp |= 0x20;
        
    }
    write8(0,0x0F,temp);
}

void ICM20948_DMA:: pin_polarity(bool polarity){
    uint8_t temp = read8(0,0x0F);
    if(!polarity){
        temp &= ~0x80;

    }else{
        temp |= 0x80;
        
    }
    write8(0,0x0F,temp);

}

void ICM20948_DMA:: enable_data_ready(){
    write8(0,0x11,0x01);
}

//this code is important for rearm the interupt due the icm cant read and clear interupt_1
void ICM20948_DMA:: clear_int(){
    
    size_t aligned_len;
    dma_tx_buf[0] = 0x17 | 0x80;
    dma_tx_buf[1] = 0x00;

    dma_tx_buf[2] =  0x19| 0x80;
    dma_tx_buf[3] = 0x00;


    dma_tx_buf[4] =  0x1A | 0x80;
    dma_tx_buf[5] = 0x00;

    aligned_len = (6 + 3) & ~0x03; // 4byte is missing SPI SLAVE


    DMASPI->queue(dma_tx_buf, NULL , aligned_len);
    DMASPI->trigger();
}








