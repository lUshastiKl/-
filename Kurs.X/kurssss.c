/**
 * ????????? ??? ????????? ???????? ? ??????????? ? BMP180
 * ?????? ?????? ? ?????????? ??????
 */

#include <xc.h>
#include <stdio.h>
#include <string.h>

#define _XTAL_FREQ 4000000

#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

// ?????? ????????? BMP180
#define BMP180_ADDR 0x77
#define BMP180_REG_AC1 0xAA
#define BMP180_REG_AC2 0xAC
#define BMP180_REG_AC3 0xAE
#define BMP180_REG_AC4 0xB0
#define BMP180_REG_AC5 0xB2
#define BMP180_REG_AC6 0xB4
#define BMP180_REG_B1 0xB6
#define BMP180_REG_B2 0xB8
#define BMP180_REG_MB 0xBA
#define BMP180_REG_MC 0xBC
#define BMP180_REG_MD 0xBE
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_DATA 0xF6

// ??????? BMP180
#define BMP180_CMD_TEMP 0x2E
#define BMP180_CMD_PRESS 0x34

// ????????????? ????????????
short ac1, ac2, ac3;
unsigned short ac4, ac5, ac6;
short b1, b2, mb, mc, md;

char buffer[50];
long b5; // ???????????? ? ????????

// ????????? ???????
void Init(void);
void UART_SendString(char *text);
void I2C_Init(void);
void I2C_Start(void);
void I2C_Restart(void);
void I2C_Stop(void);
void I2C_Write(unsigned char data);
unsigned char I2C_Read(unsigned char ack);
void BMP180_ReadCalibration(void);
unsigned int BMP180_ReadUT(void);
unsigned long BMP180_ReadUP(void);
void BMP180_Calculate(long *temperature, long *pressure);

void Init(void) {
    // ????????? ??????
    TRISCbits.TRISC3 = 1;  // SCL
    TRISCbits.TRISC4 = 1;  // SDA  
    TRISCbits.TRISC6 = 0;  // TX
    TRISCbits.TRISC7 = 1;  // RX
    
    // ????????? UART
    SPBRG = 25;
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    
    // ????????? I2C
    I2C_Init();
    
    __delay_ms(100);
}

void I2C_Init(void) {
    SSPCON1 = 0x28;
    SSPCON2 = 0x00;
    SSPSTAT = 0x00;
    SSPADD = 9; // 100kHz ??? 4MHz
}

void I2C_Start(void) {
    SSPCON2bits.SEN = 1;
    while(SSPCON2bits.SEN);
}

void I2C_Restart(void) {
    SSPCON2bits.RSEN = 1;
    while(SSPCON2bits.RSEN);
}

void I2C_Stop(void) {
    SSPCON2bits.PEN = 1;
    while(SSPCON2bits.PEN);
}

void I2C_Write(unsigned char data) {
    SSPBUF = data;
    while(SSPSTATbits.BF);
    // ???? ?????????? ????????
    while(SSPCON2bits.ACKSTAT); // ???? ACK
}

unsigned char I2C_Read(unsigned char ack) {
    SSPCON2bits.RCEN = 1;
    while(!SSPSTATbits.BF);
    SSPCON2bits.ACKDT = ack;
    SSPCON2bits.ACKEN = 1;
    while(SSPCON2bits.ACKEN);
    return SSPBUF;
}

void UART_SendString(char *text) {
    while(*text) {
        while(!TXSTAbits.TRMT);
        TXREG = *text++;
    }
}

// ?????? ????????????? ??????
void BMP180_ReadCalibration(void) {
    // ?????? AC1
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_AC1);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ac1 = (short)(I2C_Read(0) << 8);
    ac1 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? AC2
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_AC2);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ac2 = (short)(I2C_Read(0) << 8);
    ac2 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? AC3
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_AC3);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ac3 = (short)(I2C_Read(0) << 8);
    ac3 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? AC4
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_AC4);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ac4 = (I2C_Read(0) << 8);
    ac4 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? AC5
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_AC5);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ac5 = (I2C_Read(0) << 8);
    ac5 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? AC6
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_AC6);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ac6 = (I2C_Read(0) << 8);
    ac6 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? B1
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_B1);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    b1 = (short)(I2C_Read(0) << 8);
    b1 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? B2
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_B2);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    b2 = (short)(I2C_Read(0) << 8);
    b2 |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? MB
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_MB);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    mb = (short)(I2C_Read(0) << 8);
    mb |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? MC
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_MC);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    mc = (short)(I2C_Read(0) << 8);
    mc |= I2C_Read(1);
    I2C_Stop();
    
    // ?????? MD
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_MD);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    md = (short)(I2C_Read(0) << 8);
    md |= I2C_Read(1);
    I2C_Stop();
}

// ?????? ???????? ???????????
unsigned int BMP180_ReadUT(void) {
    unsigned int ut;
    
    // ?????? ????????? ???????????
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_CONTROL);
    I2C_Write(BMP180_CMD_TEMP);
    I2C_Stop();
    
    __delay_ms(5); // ???????? ?????????
    
    // ?????? ??????????
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_DATA);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    ut = I2C_Read(0) << 8;
    ut |= I2C_Read(1);
    I2C_Stop();
    
    return ut;
}

// ?????? ????????? ????????
unsigned long BMP180_ReadUP(void) {
    unsigned long up;
    
    // ?????? ????????? ????????
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_CONTROL);
    I2C_Write(BMP180_CMD_PRESS | (3 << 6)); // Ultra high resolution
    I2C_Stop();
    
    __delay_ms(26); // ???????? ??? ultra high resolution
    
    // ?????? ?????????? (3 ?????)
    I2C_Start();
    I2C_Write(BMP180_ADDR << 1);
    I2C_Write(BMP180_REG_DATA);
    I2C_Restart();
    I2C_Write((BMP180_ADDR << 1) | 1);
    up = (unsigned long)I2C_Read(0) << 16;
    up |= (unsigned long)I2C_Read(0) << 8;
    up |= I2C_Read(1);
    I2C_Stop();
    
    up >>= (8 - 3); // ????? ??? ultra high resolution
    
    return up;
}

// ?????? ??????????? ? ????????
void BMP180_Calculate(long *temperature, long *pressure) {
    long ut = BMP180_ReadUT();
    long up = BMP180_ReadUP();
    
    // ?????? ???????????
    long x1 = ((ut - ac6) * ac5) >> 15;
    long x2 = ((long)mc << 11) / (x1 + md);
    b5 = x1 + x2;
    *temperature = (b5 + 8) >> 4; // ??????????? ? 0.1°C
    
    // ?????? ????????
    long b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = ac2 * b6 >> 11;
    long x3 = x1 + x2;
    long b3 = (((ac1 * 4 + x3) << 3) + 2) >> 2;
    x1 = ac3 * b6 >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = (x1 + x2 + 2) >> 2;
    unsigned long b4 = ac4 * (unsigned long)(x3 + 32768) >> 15;
    unsigned long b7 = ((unsigned long)up - b3) * 50000UL;
    
    long p;
    if (b7 < 0x80000000) {
        p = (b7 << 1) / b4;
    } else {
        p = (b7 / b4) << 1;
    }
    
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    *pressure = p + ((x1 + x2 + 3791) >> 4);
}

void main(void) {
    long temperature, pressure;
    unsigned int counter = 0;
    
    Init();
    
    UART_SendString("BMP180 Pressure and Temperature Sensor\r\n");
    UART_SendString("Reading calibration data...\r\n");
    
    // ?????? ????????????? ??????
    BMP180_ReadCalibration();
    
    UART_SendString("Calibration data read successfully!\r\n");
    UART_SendString("Starting measurements...\r\n\r\n");
    
    while(1) {
        // ????????? ??????????? ? ????????
        BMP180_Calculate(&temperature, &pressure);
        
        // ????? ???????????
        sprintf(buffer, "Measurement #%u\r\n", counter++);
        UART_SendString(buffer);
        
        sprintf(buffer, "Temperature: %ld.%ld C\r\n", temperature/10, temperature%10);
        UART_SendString(buffer);
        
        sprintf(buffer, "Pressure: %lu Pa\r\n", pressure);
        UART_SendString(buffer);
        
        sprintf(buffer, "Pressure: %lu hPa\r\n\r\n", pressure/100);
        UART_SendString(buffer);
        
        __delay_ms(2000);
    }
}