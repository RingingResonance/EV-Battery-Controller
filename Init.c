/*  Electric Vehicle Battery Monitoring System.>
    Copyright (C) <2020>  <Jarrett R. Cigainero>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>*/

#ifndef INIT_C
#define INIT_C


#include <p30f3011.h>
#include "Init.h"
#include "DataIO.h"
#include "subs.h"


void Init(void){
/*******************************
 * initialization values setup.
*******************************/
    /* Osc Config*/
    OSCCONbits.NOSC = 1;
    OSCCONbits.OSWEN = 1;
    OSCCONbits.LPOSCEN = 0;
    
    /* Analog inputs and general IO */
    TRISB = 0x008F;              //set portb to mix analog inputs and digital outputs.
    LATB = 0;               //clear portb

    /**************************/
    /* PWM outputs and charge detect input. */
    TRISE = 0xFFFF;              //set porte to all inputs.
    //LATE = 0;               //set all porte outputs to 0
    /**************************/
    /* General IO */
    /* Pins 27 and 28 should be reserved for AUX com on UART2. What was this for again? pins 27/28 are not any kind of serial comm pins??? */
    TRISF = 0x00BE;              //set portf to all inputs and two output.
    LATF = 0;
    /**************************/
    /* General IO. */
    TRISD = 0xFFF3;              //set portd to all inputs except for RD2 and RD3 which is the KEEPALIVE and mShutdown signals.
    //LATD = 0;
    /**************************/
    /* Brake Light Output */
    TRISC = 0x0000;
    LATC = 0;

/*****************************/
/* Configure PWM */
/*****************************/
    PTCON = 0x0036;     //Set the PWM module and set to up/down mode for center aligned PWM.
    PTMR = 0;
    PTPER = 49;         //set period. 0% - 99%
    SEVTCMP = 0;
    PWMCON1 = 0x00F0;       //Set PWM output for complementary mode.
    PWMCON2 = 0x0000;
    DTCON1 = 0;
    FLTACON = 0;
    OVDCON = 0xFF00;
    PDC1 = 0000;            //set output to 0
    PDC2 = 0000;            //set output to 0
    PDC3 = 0000;            //set output to 0

/*****************************/
/* Configure UARTs */
/*****************************/
    //PORT 1 setup
    U1STA = 0;
    U1MODE = 0;
    U1BRG = BaudCalc(BAUD1, IPS);     //calculate the baud rate.
    U1MODEbits.ALTIO = 1;           //Use alternate IO for UART1.
    //Default power up of UART should be 8n1
    
    //PORT 2 setup
    U2STA = 0;
    U2STAbits.UTXISEL = 1;
    U2MODE = 0;
    U2BRG = BaudCalc(BAUD2, IPS);     //calculate the baud rate.
    //Default power up of UART should be 8n1
/*****************************/
/* Configure Timer 1 */
/* Scan IO about every second when KeySwitch is off. */
/*****************************/
/* Timer one CTRL. */
    PR1 = 0xFFFF;
    TMR1 = 0x0000;
    T1CON = 0x0000;
    T1CONbits.TCKPS = 3;        //1:256 prescale

/*****************************/
/* Configure Timer 2 */
/*****************************/
/* For 0.125 second timing operations. */
    //PR2 = 0xE4E2;   //58,594
    PR2 = 0x7075;     //28,789
    TMR2 = 0x0000;
    T2CON = 0x0000;
    T2CONbits.TCKPS = 2;        //1:64 prescale 

/*****************************/
/* Configure Timer 3 */
/*For speed calculation. */
/*****************************/
/* For 1 second timing operations. */
    PR3 = 0xE0EA;   //57,578
    //PR3 = 0x7271;     //29,297
    TMR3 = 0x0000;
    T3CON = 0x0000;
    T3CONbits.TCKPS = 3;        //1:256 prescale 

/*****************************/
/* Enable analog inputs */
/*****************************/
    ADCON3upper8 = 0x0F;
    ADCON3lower8 = 0x0F;
    ADCON1 = 0x02E4;
    ADCON2 = 0x0410;
    ADCON3 = 0x0F0F;    //0x0F0F;
    ADCHS = 0x0000;
    ADPCFG = 0xFF70;
    ADCSSL = 0x008F;

/*****************************/
/* Configure IRQs. */
/*****************************/
    //Configure Priorities
    IPC1bits.T2IP = 1;
    IPC4bits.INT1IP = 2;
    IPC0bits.T1IP = 3;
    IPC2bits.ADIP = 4;
    IPC0bits.INT0IP = 5;
    IPC2bits.U1RXIP = 6;
    IPC5bits.INT2IP = 7;
    // Clear all interrupts flags
    IFS0 = 0;
    IFS1 = 0;
    IFS2 = 0;

	// enable interrupts
	__asm__ volatile ("DISI #0x3FFF");
    IEC0 = 0;
    IEC1 = 0;
    IEC2 = 0;
	IEC0bits.T1IE = 1;	// Enable interrupts for timer 1
    IEC0bits.U1RXIE = 1; //Enable interrupts for UART1 Rx.
    IEC1bits.U2RXIE = 1; //Enable interrupts for UART2 Rx.
    IEC1bits.U2TXIE = 1; //Enable interrupts for UART2 Tx.
    if(EnableChIRQ == 1){
        IEC0bits.INT0IE = 1;    //Charge Detect IRQ
    }
    EnableChIRQ = 1;    //By default, enable charge detect IRQ on init.
    IEC1bits.INT1IE = 1;    //Wheel rotate IRQ
    IEC1bits.INT2IE = 0;  //Disable irq for INT2, not used.
    IEC0bits.T2IE = 1;	// Enable interrupts for timer 2
    IEC0bits.T3IE = 0;	// Disable interrupts for timer 3
    IEC0bits.ADIE = 1;  // Enable ADC IRQs.
    INTCON2bits.INT0EP = 0;
    INTCON2bits.INT2EP = 0;
DISICNT = 0;
/*****************************/
/* Enable our devices. */
/*****************************/
    ADCON1bits.ADON = 1;    // turn ADC ON 
    PTCONbits.PTEN = 1;     // Enable PWM
    T2CONbits.TON = 1;      // Start Timer 2
    T1CONbits.TON = 1;      // Start Timer 1
    T3CONbits.TON = 1;      // Start Timer 3
    U1MODEbits.UARTEN = 1;  //enable UART1 
    U1STAbits.UTXEN = 1;    //enable UART1 TX
    U2MODEbits.UARTEN = 1;  //enable UART2 
    U2STAbits.UTXEN = 1;    //enable UART2 TX
/* End Of Initial Config stuff. */
/* Now do some pre-calculations. */
    
    //Calculate our voltage divider values.
    vltg_dvid = R2_resistance / (R1_resistance + R2_resistance);
    //Calculate our charge/discharge rate.
    calc_125 = 0.125 / 3600;
    //We've done Init.
    init_done = 1;
    //We aren't in low power mode
    lw_pwr = 0;
    //Calculate max charge current.
    max_chrg_current = chrg_C_rating * amp_hour_rating;
    //Send Init message.
    //send_string(NLtxtNL, "Initialized.", PORT1);

}


//Go in to low power mode when not in use.
void low_power_mode(void){
        
    io_off();
    ADCON1bits.ADON = 0;    // turn ADC off
    T2CONbits.TON = 0;      // Stop Timer 2
    T3CONbits.TON = 0;      // Stop Timer 3
//    U1MODEbits.UARTEN = 0;  //disable UART 
//    U1STAbits.UTXEN = 0;    //disable UART TX
    
    	// disable interrupts
	__asm__ volatile ("DISI #0x3FFF");

    IEC1bits.INT1IE = 0;    //disable Wheel rotate IRQ
    IEC0bits.T2IE = 0;	// disable interrupts for timer 2

    INTCON2bits.INT1EP = 0;
    INTCON2bits.INT2EP = 0;
    DISICNT = 0;
    
    //Need to reinit on restart
    init_done = 0;
    //Tell everyone we are in low power mode.
    lw_pwr = 1;
    //Turn off Outputs, etc
    PORTBbits.RB6 = 0;

}

/* Turn everything off so we don't waste any more power.
 * Only plugging in the charge will restart the CPU, or yaknow, just restart the CPU... */
void low_battery_shutdown(void){
    
    io_off();
    cmd_power = 0;
    soft_power = 0;
    ADCON1bits.ADON = 0;    // turn ADC off 
    PTCONbits.PTEN = 0;     // off PWM
    T2CONbits.TON = 0;      // Stop Timer 2
    T1CONbits.TON = 0;      // Stop Timer 1
    T3CONbits.TON = 0;      // Stop Timer 3
    
    // Clear all interrupts flags
    IFS0 = 0;
    IFS1 = 0;
    IFS2 = 0;
    
    	// disable interrupts
	__asm__ volatile ("DISI #0x3FFF");
	IEC0bits.T1IE = 0;	// disable interrupts for timer 1
    IEC1bits.INT1IE = 0;    //disable Wheel rotate IRQ
    IEC0bits.T2IE = 0;	// disable interrupts for timer 2
    IEC0bits.ADIE = 0;  //disable ADC IRQs.

    INTCON2bits.INT1EP = 0;
    INTCON2bits.INT2EP = 0;
    DISICNT = 0;

    //Need to reinit on restart
    init_done = 0;
    deep_sleep = 1;     //Tell the system to enter a deep sleep after we have completed all tasks one last time.
}

#endif