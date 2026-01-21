/*
----------------------------------------------------
Project Title:
Ultrasonic Distance Monitoring System with LCD,
Danger Indicators, UART Transmission, and ESP32
Web Visualization

Microcontroller:
PIC16F877A @ 4 MHz Crystal

Course:
Microprocessors

Description:
This firmware measures distance using an ultrasonic
sensor (HC-SR04), displays the measured distance and
status on a 16x2 LCD, activates LEDs and a buzzer
based on safety thresholds, and transmits the distance
value via UART to an ESP32 module for web visualization.

Peripherals Used:
- Timer1 (pulse width measurement)
- UART (9600 baud, TX only)
- GPIO (LCD, ultrasonic sensor, LEDs, buzzer)

Author:
 * Anas Hani Abd Allah Almakhamreh

----------------------------------------------------
*/

#include <xc.h>
#include <stdio.h>

#define _XTAL_FREQ 4000000

/* -------- CONFIG -------- */
#pragma config FOSC = XT, WDTE = OFF, PWRTE = ON, BOREN = ON
#pragma config LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF

/* -------- LCD -------- */
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

/* -------- IO -------- */
#define BUZZER RB3
#define TRIG   RB1
#define ECHO   RB2

/* -------- LCD FUNCTIONS -------- */
void Lcd_Port(unsigned char a){
    D4 = (a >> 0) & 1;
    D5 = (a >> 1) & 1;
    D6 = (a >> 2) & 1;
    D7 = (a >> 3) & 1;
}

void Lcd_Cmd(unsigned char a){
    RS = 0;
    Lcd_Port(a >> 4); EN = 1; __delay_ms(2); EN = 0;
    Lcd_Port(a & 0x0F); EN = 1; __delay_ms(2); EN = 0;
}

void Lcd_Init(void){
    __delay_ms(100);

    Lcd_Port(0x03); EN = 1; __delay_ms(5); EN = 0;
    Lcd_Port(0x03); EN = 1; __delay_ms(5); EN = 0;
    Lcd_Port(0x03); EN = 1; __delay_ms(5); EN = 0;
    Lcd_Port(0x02); EN = 1; __delay_ms(5); EN = 0;

    Lcd_Cmd(0x28);   // 4-bit, 2-line
    Lcd_Cmd(0x0C);   // Display ON
    Lcd_Cmd(0x06);   // Cursor increment
    Lcd_Cmd(0x01);   // Clear
}

void Lcd_Char(char a){
    RS = 1;
    Lcd_Port(a >> 4); EN = 1; __delay_us(50); EN = 0;
    Lcd_Port(a & 0x0F); EN = 1; __delay_us(50); EN = 0;
}

void Lcd_String(char *s){
    while(*s) Lcd_Char(*s++);
}

void Lcd_SetCursor(char r, char c){
    Lcd_Cmd((r == 1 ? 0x80 : 0xC0) + c - 1);
}

/* -------- ULTRASONIC -------- */
unsigned int get_distance(void){
    unsigned int timeout;

    T1CON = 0x10;   // Timer1 off, 1:2 prescaler
    TMR1 = 0;

    TRIG = 1;
    __delay_us(10);
    TRIG = 0;

    /* Wait for ECHO HIGH */
    timeout = 30000;
    while(!ECHO && timeout--);
    if(timeout == 0) return 0;   // no object

    TMR1 = 0;
    T1CONbits.TMR1ON = 1;

    /* Wait for ECHO LOW */
    timeout = 30000;
    while(ECHO && timeout--);
    T1CONbits.TMR1ON = 0;

    if(timeout == 0) return 0;   // echo too long / invalid

    /* Reject absurd values */
    if(TMR1 > 20000) return 0;

    return TMR1 / 29;   // valid distance
}

void UART_Init(void) {
    TXSTA = 0x24;   // TXEN=1, BRGH=1
    RCSTA = 0x90;   // SPEN=1
    SPBRG = 25;     // 9600 baud @ 4 MHz
}

void UART_SendChar(char c) {
    while (!TXIF);
    TXREG = c;
}

void UART_SendString(const char *s) {
    while (*s) {
        UART_SendChar(*s++);
    }
}

/* -------- MAIN -------- */
void main(void){

    ADCON1 = 0x06;   // PORTA + PORTB digital
    TRISC6 = 0;   // TX as output
    TRISA = 0x00; PORTA = 0;
    TRISB = 0x04; PORTB = 0;
    TRISD = 0x00; PORTD = 0;

    Lcd_Init();
    UART_Init();
    while(1){
        unsigned int d = get_distance();
        char txbuf[8];
        sprintf(txbuf, "%u\n", d);
        UART_SendString(txbuf);

        char buf[16];

        Lcd_SetCursor(1,1);
        sprintf(buf, "Dist:%3ucm   ", d);
        Lcd_String(buf);

        Lcd_SetCursor(2,1);

        if(d == 0 || d > 200){
            Lcd_String("STATUS: CLEAR  ");
            PORTA = 0x00;
            BUZZER = 0;
        }
        else if(d < 15){
            Lcd_String("STATUS: CRITICAL");
            PORTA = 0x08;
            BUZZER = 1;
        }
        else if(d < 30){
            Lcd_String("STATUS: WARNING ");
            PORTA = 0x04;
            BUZZER = 0;
        }
        else if(d < 50){
            Lcd_String("STATUS: MEDIUM  ");
            PORTA = 0x02;
            BUZZER = 0;
        }
        else{
            Lcd_String("STATUS: SAFE    ");
            PORTA = 0x01;
            BUZZER = 0;
        }

        __delay_ms(150);
    }
}

