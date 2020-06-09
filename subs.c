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

#ifndef SUBS_C
#define SUBS_C

#include <p30f3011.h>

#include "subs.h"
#include "DataIO.h"
#include "Init.h"
/* Fun fact, you can comment these out and it still compiles even though they are needed!
 * Probably because they are included in main.c IDK mplab is weird.
 * This file needs to be split up into it's seperate systems.
 */

//System power off for power saving.
void power_off(void){
    PORTDbits.RD2 = 0; //Disable Keep Alive signal. TODO: Implement a save to EEPROM function before we cut the power.
}

//Warm start and reset check.
void first_check(void){
    static int reset_chk;              //Do not initialize this var. Needs to stay the same on a reset.
    while(start_timer < 0xFFFE){
        start_timer++;          //Wait here for the power supply to stabilize, POR isn't long enough sometimes.
    }
    //Do not check why we reset on initial power up. No reason to. We don't want a reset error on first power up.
    if(reset_chk == 0xAA55){
        reset_check();              //Check for reset events.
    }
    reset_chk = 0xAA55;         //Warm start.
}
//Main Power Check.
void main_power_check(void){
        /* Check for charger, key, or software power up. */
        if((PORTEbits.RE8 == 1 || PORTFbits.RF1 == 0 || soft_power == 1 || cmd_power == 1 || power_plugged == 1) && first_cal == 9){
            pwr_detect = 1;         //Used for blinking error light when in a fault shutdown.
            
            if(diag_state){
                diag_count += 5;
                diag_state = 0;
            }
            
            if(shutdown_timer == 1){
                fault_shutdown = 0;
                osc_fail_event = 0;
                shutdown_timer = 0;
            }
            
            if(fault_shutdown == 0){
                main_power = 1;     //Main power is ON.
                //Reinit if we haven't already.
                if (init_done == 0){
                    Init();
                }
            }
            else{
                io_off();
                main_power = 0;
                heat_power = 0;               //set heat power
                charge_power = 0;             //set charge control
                output_power = 0;             //set output control
                //Deinit if we haven't already.
                if (lw_pwr == 0){
                    low_power_mode();   //Go into idle mode with heart beat running.
                }
            }
        }
        else{
            diag_state = 1;
            io_off();
            pwr_detect = 0;     //Used for blinking error light when in a fault shutdown.
            main_power = 0;     //Main power is OFF.
            heat_power = 0;               //set heat power
            charge_power = 0;             //set charge control
            output_power = 0;             //set output control
            
            //Deinit if we haven't already.
            if (lw_pwr == 0){
                low_power_mode();   //Go into idle mode with heart beat running.
            }
        }
}

void analog_sanity(void){
    //Battery Voltage
    if(ADCBUF0 >= 0xFFFE){
        fault_log(0x1D);
        general_shutdown();     //If we can't get battery voltage then what's the point of living?
    }
    if(ADCBUF0 <= 0x0001){
        fault_log(0x1E);
        general_shutdown();     //If we can't get battery voltage then what's the point of living?
    }
    //Battery Current
    if(ADCBUF1 >= 0xFFFE){
        fault_log(0x1F);
        general_shutdown();
    }
    if(ADCBUF1 <= 0x0001){
        fault_log(0x20);
        general_shutdown();
    }
    //Battery temperature.
    if(ADCBUF2 >= 0xFFFE){
        fault_log(0x21);
        heat_cal_stage = 5;     //Disable Heater if we can't get battery temperature.
    }
    if(ADCBUF2 <= 0x0001){
        fault_log(0x22);
        heat_cal_stage = 5;     //Disable Heater if we can't get battery temperature.
    }
    //motor controller temperature.
    //We can live with these temps in error so don't bother disabling or shutting anything down here. Just log the error.
    if(ADCBUF3 >= 0xFFFE){
        fault_log(0x23);
    }
    if(ADCBUF3 <= 0x0001){
        fault_log(0x24);
    }
    //Snowman's temperature.
    if(ADCBUF4 >= 0xFFFE){
        fault_log(0x25);
    }
    if(ADCBUF4 <= 0x0001){
        fault_log(0x26);
    }

}

//Check reset conditions and log them.
void reset_check(void){
    if(RCONbits.BOR){
        RCONbits.BOR = 0;
        fault_log(0x13);        //Brown Out Event.
    }
    if(RCONbits.WDTO){
        RCONbits.WDTO = 0;
        fault_log(0x14);        //WDT Reset Event.
    }
    if(RCONbits.TRAPR){
        RCONbits.TRAPR = 0;
        fault_log(0x15);        //TRAP Conflict Event.
    }
    if(RCONbits.IOPUWR){
        RCONbits.IOPUWR = 0;
        fault_log(0x16);        //Illegal opcode or uninitialized W register access Event.
    }
    if(RCONbits.EXTR){
        RCONbits.EXTR = 0;
        fault_log(0x17);        //External Reset Event.
    }
    if(RCONbits.SWR){
        RCONbits.SWR = 0;
        fault_log(0x18);        //Reset Instruction Event.
    }
    //if(reset_chk == 0xAA55){
        fault_log(0x19);        //General Reset Check. Did we start from cold? If not then log that we had a reset.
    //}
}



                //TBLWTL();
        //eedata_word = save_file[x];
                //WORK = save_file[x];
        //__asm__ volatile ("EE_DATA,W1");
/*
void config_save(void){
    int x = 0;
    
    //while(x < 20){
        NVMADRU = 0x7F;
        NVMADR = 0xFC00;
        NVMCON = 0x4044;
        NVMKEY = 0x55;
        NVMKEY = 0xAA;
        Nop();
        Nop();
        NVMCONbits.WR = 1;
        while(NVMCONbits.WR){}
        
        //testguy = WREG1;
        WREG0 = 0x0000;
        WREG1 = 0xE3A1; //Test data pattern.
        //__asm__ volatile ("TBLWTL w1,[w0]");
        NVMADRU = 0x7F;
        NVMADR = 0xFC00;
        NVMCON = 0x4004;
        NVMKEY = 0x55;
        NVMKEY = 0xAA;
        Nop();
        Nop();
        NVMCONbits.WR = 1;
        while(NVMCONbits.WR){}
        x++;
    //}
}
void config_load(void){
    TBLPAG = 0x7F;
    WREG0 = 0xFC00; //address to read from
    __asm__ volatile ("TBLRDL [w0],w4");
    Nop();
    testguy2 = WREG4; //WREG4 will contain data read from eeprom
    Sleep();            //Test sleep. Stop here for debug.
}
void calc_save(void){
    
}
void calc_load(void){
    
}
*/

//Battery Percentage Calculation. This does NOT calculate the % total charge of battery, only the total voltage percentage.
void volt_percent(void){
    if (battery_voltage <= (dischrg_voltage - 2)){
        voltage_percentage = 0;
    }
    else if (battery_current < 0.05 && battery_current > -0.05){
        open_voltage = battery_voltage;
        voltage_percentage = 100 * ((open_voltage - dischrg_voltage) / (battery_rated_voltage - dischrg_voltage));
        got_open_voltage = 1;
    }
}

//Find current compensation value.
void current_cal(void){
    float signswpd_avg_cnt = 0;
    signswpd_avg_cnt = battery_crnt_average * -1;
    //do the current cal.
    if(curnt_cal_stage == 4){
        current_compensate = (signswpd_avg_cnt - circuit_draw);
        curnt_cal_stage = 5;        //Current Cal Complete
        //send_string(NLtxtNL, "Current Cal.", PORT1);
        if(heat_cal_stage != 5){
            heat_cal_stage = 1;     //Do a heater cal after we have done current cal unless it is disabled.
        }
        soft_power = 0;
        //Done with current cal.
    }
    //Initialize current cal.
    if(curnt_cal_stage == 1){
        current_compensate = 0;
        io_off();    //Turn off all inputs and outputs.
        curnt_cal_stage = 4;
        soft_power = 1;         //Turn soft power on to run 0.125s IRQ.
    }
}

//Charge, discharge, and heater regulation.
void regulate(void){
    float dischrg_current = 0;
    float chrg_current = 0;
    double dischrg_read = 0;
    float chrg_current_read = 0;
    //Discharge current read and target calculation.
    //Copy battery values so that we don't disturb them.
    dischrg_read = battery_current;
    chrg_current_read = battery_current;
    
    //Calculate max discharge current based off battery remaining and battery temp.
    temp_dischrg_rate = 1;
    if (battery_temp > dischrg_reduce_high_temp || battery_temp < dischrg_reduce_low_temp){
        temp_dischrg_rate = 0.8;     //Decrease the current by 20% if we are too hot or too cold.
    }
    if (battery_temp < dischrg_min_temp){
        temp_dischrg_rate = 0.5;     //Decrease the current by 50% if we are too cold.
    }
    dischrg_current = (dischrg_C_rating * battery_remaining) * temp_dischrg_rate;
    
    //Don't set our max discharge current below the limp current setpoint.
    if(dischrg_current < limp_current || battery_temp < dischrg_min_temp){
        dischrg_current = limp_current;
    }
    
    //Get absolute value for battery current.
    if (dischrg_read < 0){
        dischrg_read *= -1;
    }
    else{
        dischrg_read = 0;
    }
    //Calculate max charge current based off battery temp.
    temp_dischrg_rate = 1;
    if (battery_temp > chrg_reduce_high_temp || battery_temp < chrg_reduce_low_temp){
        temp_dischrg_rate = 0.8;     //Decrease the current by 20% if we are too hot or too cold.
    }
    
    //Charge current read and target calculation.
    chrg_current = (chrg_C_rating * amp_hour_rating) * temp_dischrg_rate;
    if(battery_temp > chrg_max_temp || battery_temp < chrg_min_temp){
        chrg_current = 0;   //Inhibit charging battery if temperature is out of range.
    }
    
    //Charge current should be positive, if it is negative then set it to 0.
    if (chrg_current_read < 0){
        chrg_current_read = 0;
    }
    
    //Delay is over, start checking for charger again.
    power_plugged = 0;
    
    //RF1 is key switch and active low, RE8 is Charger Port and active high.
//// Check for Charger.
    if(PORTEbits.RE8 == 1 && first_cal == 9){
        PORTBbits.RB4 = 1;          //charger light on.
        //Charger timeout check. If charger is plugged in but we aren't getting current then we 
        //need to shutdown and log an error code so we don't run down the battery.
        float chk_voltage = 0;
        chk_voltage = chrg_voltage - 0.5;       //Half a volt less than charge voltage.
        if(chrg_check < 20000 && chrg_current_read == 0 && PORTFbits.RF1 == 1 && battery_voltage < chk_voltage){
            chrg_check++;
        }
        else if(chrg_check >= 20000 && PORTFbits.RF1 == 1){
            fault_log(0x1B);            //Log insufficient current from charger.
            fault_shutdown = 1;
            chrg_check = 0;
            //This usually happens when we detect a charge voltage but the charge regulator isn't passing enough current.
        }
        else if(chrg_check > 0){
            chrg_check--;
        }
        
        //Run heater if needed, but don't turn it up more than what the charger can handle.
        if(chrg_current_read > 0.01 || (battery_voltage >= chrg_voltage - 0.15)){
            heat_control(chrg_target_temp);
        }
        else if(chrg_current_read == 0 && heat_power > 0){
            heat_power--;
        }
        else if(battery_temp > (chrg_target_temp + 0.5) && heat_power > 0){
            heat_power--;
        }
        
        //Set current to 0 if battery voltage is over the charge set voltage.
        //Do this so that we can still run the heater without charging or discharging the battery when doing a partial charge.
        float   input_current = 0;
        if((battery_voltage > chrg_voltage + 0.05)){
            zero_current = 0;
        }
        else if((battery_voltage < chrg_voltage - 0.05)){
            zero_current = 1;
        }
        //We do it like this so that float_current can be updated even when neither of the previous if statements are true.
        if(zero_current){
            input_current = chrg_current;
        }
        else{
            input_current = 0;
        }
        
        // Charge regulation routine.
        if((battery_voltage >= battery_rated_voltage - 0.01) || (chrg_current_read > input_current + 0.01)){

            if(charge_power > 0){
                charge_power--;
            }
        }
        else if((battery_voltage < battery_rated_voltage - 0.07) || (chrg_current_read < input_current)){
            if(charge_power < 101){
                charge_power++;
            }
        }
        if(heat_cal_stage == 2){
            charge_power = 0;       //Inhibit charging if we are in the middle of heater calibration.
        }
    }
    else{
        PORTBbits.RB4 = 0;          //charger light off.
        charge_power = 0;
        chrg_check = 0;
    }

////Check for key power or command power signal, but not soft power signal.
    if((PORTFbits.RF1 == 0 || cmd_power == 1) && first_cal == 9){
        //Run heater if needed, but don't run this sub a second time if we are getting charge power while key is on.
        //If we are getting charge power then we need to use it to warm the battery to a higher temp if needed.
        if(PORTEbits.RE8 == 0){
            heat_control(dischrg_target_temp);
        }
        //Discharge regulation.
        if(hunter){
            //Hunter Mode
            if(battery_temp > (dischrg_max_temp + 1) || battery_voltage < (dischrg_voltage - 1) 
            || dischrg_read > (dischrg_current + 0.2) || dischrg_read > (absolute_max_current - 0.1)){

                if(output_power > 0){
                    output_power--;
                }
            }
            else if(battery_temp < (dischrg_max_temp - 1) && battery_voltage > (dischrg_voltage + 1) 
            && dischrg_read < (dischrg_current - 0.2)){
                if(output_power < 101){
                    output_power++;
                }
            }
        }
        else{
            //Integral Mode
            //High temp limiting.
            float temp_error = 0;
            temp_error = battery_temp - dischrg_max_temp;
            voltage_output += analog_smpl_time * (temp_error * temp_proportion);
            if(voltage_output > battery_rated_voltage){
                voltage_output = battery_rated_voltage;
            }
            else if(voltage_output < dischrg_voltage){
                voltage_output = dischrg_voltage;
            }
            
            //Low voltage limiting.
            float vltg_error = 0;
            vltg_error = battery_voltage - voltage_output;
            current_output += analog_smpl_time * (vltg_error * vltg_proportion);
            if(current_output > dischrg_current){
                current_output = dischrg_current;
            }
            else if(current_output < 0){
                current_output = 0;
            }
            
            //Current regulation.
            if(current_output > absolute_max_current){
                current_output = absolute_max_current;
            }
            crnt_error = (current_output - dischrg_read) * crnt_proportion;
            if(crnt_error > 10000){
                crnt_error = 10000;
            }
            else if(crnt_error < -10000){
                crnt_error = -10000;
            }
            crnt_integral = crnt_integral + (analog_smpl_time * crnt_error);
            if(crnt_integral > 128){
                crnt_integral = 128;
            }
            else if(crnt_integral < 0){
                crnt_integral = 0;
            }
            output_power = crnt_integral;
        }
    }
    else{
        output_power = 0;
        crnt_integral = 0;
    }
    
    if(main_power == 0 || fault_shutdown == 1){
        heat_power = 0;
    }
    PDC1 = heat_power;
    PDC2 = charge_power;             //set charge control
    PDC3 = output_power;             //set output control
    
    //Check for fault shutdown.
    if(fault_shutdown == 1){
        io_off();
    }
}

// Output voltage is Vo = (PWM3/65535) * battery_voltage
//math test for current regulation algorithm.
void test(void){
    float error = 0;
    float time = 0.001;
    float integral = 0;
    float proportion = 100;
    int output = 0;
    float dischrg_current = 0;
    float dischrg_read = 0;
    
    
    error = dischrg_current - dischrg_read;
    integral += time * (error * proportion);
    if(integral > 65535){
        integral = 65535;
    }
    else if(integral < 0){
        integral = 0;
    }
    output = integral;
    /*output = integral + (proportion * error);
    if(output > 65535){
        output = 65535;
    }
    else if(output < 0){
        output = 0;
    }*/
}

//Heater regulation.
void heat_control(float target_temp){
    /* Heater regulation. Ramp the heater up or down. If the battery temp is out
         * of range then the target charge or discharge current will be set to 0 and the charge 
         * regulation routine will power the heater without charging the battery.
         */
        if(heat_cal_stage == 3){
            if(battery_temp < (target_temp - 0.5) && heat_power < heat_set){
                heat_power++;
            }
            else if(battery_temp > (target_temp + 0.5) && heat_power > 0){
                heat_power--;
            }
        }
}

/* Read fault codes to serial port.
 * If you have been wondering why this firmware is over 14K it's partly because
 * of the built in error reporting below. ;) You're Welcome.
 */
void fault_read(int smpl, int serial_port){
    int x = 0;
    if(fault_count > 10){
        send_string(NLtxtNL, "Fault Log Is Full. Please clear faults.", serial_port);
    }
    if(fault_count == 0){
        send_string(NLtxtNL, "No fault codes to report.", serial_port);
    }
    else{
        while (x < fault_count){
            if(fault_codes[x] == 0x01){
                send_string(NLtxt, "Heater load too small.", serial_port);
            }
            else if(fault_codes[x] == 0x02){
                send_string(NLtxt, "No heater detected.", serial_port);
            }
            else if(fault_codes[x] == 0x03){
                send_string(NLtxt, "Heater load too large.", serial_port);
            }
            else if(fault_codes[x] == 0x04){
                send_string(NLtxt, "Low battery shutdown.", serial_port);
            }
            else if(fault_codes[x] == 0x05){
                send_string(NLtxt, "Discharge over current.", serial_port);
            }
            else if(fault_codes[x] == 0x06){
                send_string(NLtxt, "Charge over current.", serial_port);
            }
            else if(fault_codes[x] == 0x07){
                send_string(NLtxt, "High battery voltage.", serial_port);
            }
            else if(fault_codes[x] == 0x08){
                send_string(NLtxt, "Battery over temp.", serial_port);
            }
            else if(fault_codes[x] == 0x09){
                send_string(NLtxt, "Motor controller over temp.", serial_port);
            }
            else if(fault_codes[x] == 0x0A){
                send_string(NLtxt, "Abominable Snow Monster over temp.", serial_port);
            }
            else if(fault_codes[x] == 0x0B){
                send_string(NLtxt, "Shutdown Event.", serial_port);
            }
            else if(fault_codes[x] == 0x0C){
                send_string(NLtxt, "TRAP: PWM Fault Event.", serial_port);
            }
            else if(fault_codes[x] == 0x0D){
                send_string(NLtxt, "TRAP: Oscillator Fail Event.", serial_port);
            }
            else if(fault_codes[x] == 0x0E){
                send_string(NLtxt, "TRAP: Address Error Event.", serial_port);
            }
            else if(fault_codes[x] == 0x0F){
                send_string(NLtxt, "TRAP: Stack Error Event.", serial_port);
            }
            else if(fault_codes[x] == 0x10){
                send_string(NLtxt, "TRAP: Math Error Event.", serial_port);
            }
            else if(fault_codes[x] == 0x11){
                send_string(NLtxt, "TRAP: Reserved Trap 7 Event.", serial_port);
            }
            else if(fault_codes[x] == 0x12){
                send_string(NLtxt, "TRAP: PLL out of lock.", serial_port);
            }
            else if(fault_codes[x] == 0x13){
                send_string(NLtxt, "CPU: Brown Out Event.", serial_port);
            }
            else if(fault_codes[x] == 0x14){
                send_string(NLtxt, "CPU: WDT Reset Event.", serial_port);
            }
            else if(fault_codes[x] == 0x15){
                send_string(NLtxt, "CPU: TRAP Conflict Event.", serial_port);
            }
            else if(fault_codes[x] == 0x16){
                send_string(NLtxt, "CPU: Illegal opcode or uninitialized W register access Event.", serial_port);
            }
            else if(fault_codes[x] == 0x17){
                send_string(NLtxt, "CPU: External Reset Event.", serial_port);
            }
            else if(fault_codes[x] == 0x18){
                send_string(NLtxt, "CPU: Instruction Reset Event.", serial_port);
            }
            else if(fault_codes[x] == 0x19){
                send_string(NLtxt, "CPU: Reset.", serial_port);
            }
            else if(fault_codes[x] == 0x1A){
                send_string(NLtxt, "Invalid serial port.", serial_port);
            }
            else if(fault_codes[x] == 0x1B){
                send_string(NLtxt, "Insufficient current from charger.", serial_port);
            }
            else if(fault_codes[x] == 0x1C){
                send_string(NLtxt, "Partial charge was set higher than 100%. Clamping value to 1.00 or 100%.", serial_port);
            }
            else if(fault_codes[x] == 0x1D){
                send_string(NLtxt, "Voltage High on Battery voltage input.", serial_port);
            }
            else if(fault_codes[x] == 0x1E){
                send_string(NLtxt, "Voltage Low on Battery voltage input.", serial_port);
            }
            else if(fault_codes[x] == 0x1F){
                send_string(NLtxt, "Voltage High on Battery current input.", serial_port);
            }
            else if(fault_codes[x] == 0x20){
                send_string(NLtxt, "Voltage Low on Battery current input.", serial_port);
            }
            else if(fault_codes[x] == 0x21){
                send_string(NLtxt, "Voltage High on Battery temp input.", serial_port);
            }
            else if(fault_codes[x] == 0x22){
                send_string(NLtxt, "Voltage Low on Battery temp input.", serial_port);
            }
            else if(fault_codes[x] == 0x23){
                send_string(NLtxt, "Voltage High on motor control temp input.", serial_port);
            }
            else if(fault_codes[x] == 0x24){
                send_string(NLtxt, "Voltage Low on motor control temp input.", serial_port);
            }
            else if(fault_codes[x] == 0x25){
                send_string(NLtxt, "Voltage High on snowman's temp input.", serial_port);
            }
            else if(fault_codes[x] == 0x26){
                send_string(NLtxt, "Voltage Low on snowman's temp input.", serial_port);
            }
            else{
                send_string(NLtxt, "Unknown Fault code.", serial_port);
            }
            x++;
        }
    }
}

//Used to log fault codes. Simple eh? Just call it with the code you want.
void fault_log(int f_code){
    if (fault_count < 10){
        fault_codes[fault_count] = f_code;
        fault_count++;
    }
    else{
        fault_count = 11;       //Fault log full.
    }
}

//Check and calibrate heater to the wattage chosen by the user.
void heater_calibration(void){
        if (heat_cal_stage == 2 && main_power == 1){
            float watts = 0;
            watts = (battery_voltage * battery_current) * -1;
            if (watts < max_heat){
                heat_set++;
                PDC1 = heat_set;
                if (heat_set > 95){
                    fault_log(0x0001);      //Log fault, heater is too small for the watts you want.
                    heat_cal_stage = 4;
                    float_send(watts, PORT1);
                }
                if (heat_set > 50 && watts < 2){
                    fault_log(0x0002);      //Log fault, no heater detected.
                    heat_cal_stage = 4;
                    float_send(watts, PORT1);
                }
                if (heat_set < 5 && watts > 10){
                    fault_log(0x0003);      //Log fault, short circuit on heater.
                    heat_cal_stage = 4;
                    float_send(watts, PORT1);
                }
            }
            else{
                heat_cal_stage = 3; // Heater calibration completed.
                PDC1 = 0000;
                soft_power = 0; //Go back to normal operation.
            }
    }
    
    if (heat_cal_stage == 1 && curnt_cal_stage == 5){
        PDC1 = 0000;
        Init();         //Re-init.
        io_off();    //Turn off all inputs and outputs.
        soft_power = 1; //Force device to run in power mode.
        heat_set = 0;
        heat_power = 0;
        heat_cal_stage = 2; //If heat_cal_stage is 2 then a calibration is in progress.
    }
    

    //Heater calibration is in error.
    if(heat_cal_stage == 4){
        PDC1 = 0000;
        heat_set = 0;
        heat_power = 0;
        soft_power = 0; //Go back to normal operation.
    }
}

//Check battery status for faults and dangerous conditions.
void explody_preventy_check(void){
    //Battery over voltage check
    if(battery_voltage >= max_battery_voltage){
        fault_log(0x07);    //Log a high battery voltage shutdown event.
        general_shutdown();
    }
    //Battery under voltage check.
    if(battery_voltage < low_voltage_shutdown && PORTEbits.RE8 == 0){
        fault_log(0x04);    //Log a low battery shutdown event.
        low_battery_shutdown();
    }
    //Battery over current check.
    if(battery_current < 0){
        dischr_current = battery_current * -1;
    }
    else {
        dischr_current = 0;
    }
    if(dischr_current > over_current_shutdown){
        if(oc_shutdown_timer > 5){
            fault_log(0x05);    //Log a discharge over current shutdown event.
            general_shutdown();
            oc_shutdown_timer = 0;
        }
        oc_shutdown_timer++;
    }
    //Battery charge over current check.
    if(battery_current > max_chrg_current){
        fault_log(0x06);    //Log a charge over current shutdown event.
        general_shutdown();
    }
    //Battery temp shutdown check
    if(battery_temp > battery_shutdown_temp){
        fault_log(0x08);    //Log a battery over temp shutdown event.
        general_shutdown();
    }
    //Controller temp shutdown check
    if(motor_ctrl_temp > ctrlr_shutdown_temp){
        fault_log(0x09);    //Log a motor controller over temp shutdown event.
        general_shutdown();
    }
    //My temp shutdown check
    if(my_temp > ctrlr_shutdown_temp){
        fault_log(0x0A);    //Log a My Temp over temp shutdown event.
        general_shutdown();
    }
}

//Turns off all outputs and logs a general shutdown event.
void general_shutdown(void){
    io_off();               //Shutdown all IO except Serial Comms.
    fault_shutdown = 1;       //Tells other stuff that we had a general shutdown.
    cmd_power = 0;
    soft_power = 0;
    power_plugged = 0;
    fault_log(0x0B);            //Log a general Shutdown Event.
}

//Turns off all outputs.
void io_off(void){
    PDC1 = 0000;            //set heater control
    PDC2 = 0000;            //set charge control
    PDC3 = 0000;            //set output control
    PORTFbits.RF0 = 0;
    PORTFbits.RF6 = 0;
    PORTCbits.RC15 = 0;
    heat_power = 0;               //set heater control to 0
    charge_power = 0;             //set charge control to 0
    output_power = 0;             //set output control to 0
    current_output = 0;
    crnt_integral = 0;
}

//Print vars to serial port.
void print_vars(int serial_port){
        send_string(NLtxtNL, "Voltage ", serial_port);
        float_send(battery_voltage, serial_port);
        send_string(TXT, "V", serial_port);
        send_string(NLtxtNL, "Open Voltage", serial_port);
        float_send(open_voltage, serial_port);
        send_string(TXT, "V", serial_port);
        send_string(NLtxtNL, "Target Voltage", serial_port);
        float_send(chrg_voltage, serial_port);
        send_string(TXT, "V", serial_port);
        send_string(NLtxtNL, "Current ", serial_port);
        float_send(battery_crnt_average, serial_port);
        send_string(TXT, "A", serial_port);
        send_string(NLtxtNL, "Current Usage ", serial_port);
        float_send(battery_usage, serial_port);
        send_string(TXT, "AH", serial_port);
        send_string(NLtxtNL, "Peak Output ", serial_port);
        float_send(peak_power, serial_port);
        send_string(TXT, "W  At ", serial_port);
        float_send(peak_pwr_vlts, serial_port);
        send_string(TXT, "V  and ", serial_port);
        float_send(peak_pwr_crnt, serial_port);
        send_string(TXT, "A", serial_port);
        send_string(NLtxtNL, "Battery Temp ", serial_port);
        float_send(battery_temp, serial_port);
        send_string(TXT, "C", serial_port);
        send_string(NLtxtNL, "Controller Temp ", serial_port);
        float_send(motor_ctrl_temp, serial_port);
        send_string(TXT, "C", serial_port);
        send_string(NLtxtNL, "My Temp ", serial_port);
        float_send(my_temp, serial_port);
        send_string(TXT, "C", serial_port);
        send_string(NLtxtNL, "Current Compensation ", serial_port);
        float_send(current_compensate, serial_port);
        send_string(TXT, "A", serial_port);
        send_string(txtNL, " ", serial_port);
}

//Initialize the display.
void POS_disp_init(void){
    //Check to see if our buffer is busy or if we have already initialized the display.
    if(IFS1bits.U2TXIF == 0 && dispinit == 0){
        Port2_Buffer[0] = 0x0C;     //Clear display.
        Port2_Buffer[1] = 0x12;     //Auto Return Off.
        Port2_Buffer[2] = 0x0E;     //Cursor Off.
        //Dispatch the data in the buffers to the display by creating a TX IRQ on PORT 2.
        CBuff_index = 0;            //Start Index at 0.
        CBuff_max_data = 2;        //End Index at 43 bytes. 44? Arrays start at 0 right?
        IFS1bits.U2TXIF = 1;        //Start transmitting by manually send an IRQ.
        dispinit = 1;
    }
    
}

//Writes data to Display Port Buffer for a 20 x 2 POS display. Model:??????
//This model uses rs232 so you must use a level converter for rs232.
//Dash Board display.
void POS_dash_display(void){
    
    //Check to see if our buffer is busy or if our display is initialized.
    if(IFS1bits.U2TXIF == 0 && dispinit == 1){
        //Top Row.
        //Display Setup.
        CBuff_index = 0;
        Port2_Buffer[CBuff_index] = 0x16;             //Cursor Home
        //Display Voltage.
        CBuff_index = 1;
        float_send(battery_voltage, BigBuffer);  //battery voltage
        CBuff_index = 6;
        send_string(TXT, "V ", BigBuffer);
        //Battery Watts In/Out
        CBuff_index = 8;
        space = 1;
        float_send((battery_vltg_average * battery_crnt_average), BigBuffer);      //battery watts
        CBuff_index = 13;
        send_string(TXT, "W ", BigBuffer);
        //Battery  % charge
        CBuff_index = 15;
        float batt_percent = 0;
        batt_percent = ((battery_remaining / battery_capacity) * 100);
        if(batt_percent >= 100){
            send_string(TXT, " ", BigBuffer);
            float_send(batt_percent, BigBuffer);   //battery percentage remaining
            CBuff_index = 20;
            send_string(TXT, "%", BigBuffer);
        }
        else{
            float_send(batt_percent, BigBuffer);   //battery percentage remaining
            CBuff_index = 20;
            send_string(TXT, "%", BigBuffer);
        }

        //Bottom Row Start.
        CBuff_index = 21;
        nl_send(BigBuffer);
        return_send(BigBuffer);
        CBuff_index = 23;
        send_string(TXT, "BT ", BigBuffer);
        CBuff_index = 25;
        space = 0;
        float_send(battery_temp, BigBuffer);
        CBuff_index = 30;
        send_string(TXT, "C   ", BigBuffer);
        CBuff_index = 32;
        //Low Battery
        if(batt_percent < 10){
            //Make low battery message blink.
            if(error_blink){
                send_string(TXT, "Low Batt!  ", BigBuffer);
            }
            else{
                send_string(TXT, "           ", BigBuffer);
            }
        }
        else{
            send_string(TXT, "           ", BigBuffer);
        }
        //Error status.
        CBuff_index = 42;
        if(fault_count != 0){
            //Make error message blink.
            if(error_blink){
                send_string(TXT, "E", BigBuffer);
            }
            else{
                send_string(TXT, " ", BigBuffer);
            }
        }
        else{
            send_string(TXT, " ", BigBuffer);
            err_scroll = 0;
        }
        Port2_Buffer[44] = 0x12;     //Auto Return Off.
        Port2_Buffer[45] = 0x0E;     //Cursor Off.
        
        //Diag variable viewer.
        //CBuff_index = 23;
        //float_send(crnt_integral, BigBuffer);
        //Dispatch the data in the buffers to the display by creating a TX IRQ on PORT 2.
        CBuff_index = 0;            //Start Index at 0.
        CBuff_max_data = 45;        //End Index at 43 bytes. 44? Arrays start at 0 right?
        IFS1bits.U2TXIF = 1;        //Start transmitting by manually send an IRQ.
    }
}
//*************************************88
//Diagnostics display.
void POS_diag_display(void){
    
    //Check to see if our buffer is busy or if our display is initialized.
    if(IFS1bits.U2TXIF == 0 && dispinit == 1){
        //Top Row.
        //Display Setup.
        CBuff_index = 0;
        Port2_Buffer[CBuff_index] = 0x16;             //Cursor Home
        //Display Voltage.
        CBuff_index = 1;
        float_send(battery_voltage, BigBuffer);  //battery voltage
        CBuff_index = 6;
        send_string(TXT, "V ", BigBuffer);
        //Battery Watts In/Out
        CBuff_index = 8;
        space = 1;
        float_send((battery_vltg_average * battery_crnt_average), BigBuffer);      //battery watts
        CBuff_index = 13;
        send_string(TXT, "W ", BigBuffer);
        //Battery  % charge
        CBuff_index = 15;
        float batt_percent = 0;
        batt_percent = ((battery_remaining / battery_capacity) * 100);
        if(batt_percent >= 100){
            send_string(TXT, " ", BigBuffer);
            float_send(batt_percent, BigBuffer);   //battery percentage remaining
            CBuff_index = 20;
            send_string(TXT, "%", BigBuffer);
        }
        else{
            float_send(batt_percent, BigBuffer);   //battery percentage remaining
            CBuff_index = 20;
            send_string(TXT, "%", BigBuffer);
        }

        //Bottom Row Start.
        CBuff_index = 21;
        nl_send(BigBuffer);
        return_send(BigBuffer);
        CBuff_index = 23;
        //Charging status. 4 char
        if(battery_crnt_average > 0.1){
            send_string(TXT, "Chrg ", BigBuffer);
        }
        else if(battery_crnt_average < -0.1){
            send_string(TXT, "Dis  ", BigBuffer);
        }
        else {
            send_string(TXT, "Idle ", BigBuffer);
        }
        //Heating status. 8 char
        CBuff_index = 28;
        if(heat_power >= 1){
            send_string(TXT, "Htr On  ", BigBuffer);
        }
        else{
            send_string(TXT, "Htr Off ", BigBuffer);
        }
        //Error status. 7 char
        CBuff_index = 36;
        if(fault_count != 0){
            //Make error message blink.
            if(error_blink){
                if(err_scroll >= fault_count - 1){
                    err_scroll = 0;
                }
                else{
                    err_scroll++;
                }
                if(err_scroll == fault_count - 1){
                    send_string(TXT, "END    ", BigBuffer);
                }
                else if(err_scroll == 0){
                    send_string(TXT, "Strt   ", BigBuffer);
                }
                else{
                    send_string(TXT, "       ", BigBuffer);
                }
            }
            else{
                if(err_scroll == fault_count - 1){
                    send_string(TXT, "END    ", BigBuffer);
                }
                else if(err_scroll == 0){
                    send_string(TXT, "Strt   ", BigBuffer);
                }
                else{
                    send_string(TXT, "       ", BigBuffer);
                }
            }
            Port2_Buffer[41] = four_bit_hex_cnvt((fault_codes[err_scroll] & 0xF0) / 16);
            Port2_Buffer[42] = four_bit_hex_cnvt(fault_codes[err_scroll] & 0x0F);
            Port2_Buffer[43] = 'h';
        }
        else{
            send_string(TXT, "Sys OK ", BigBuffer);
            err_scroll = 0;
        }

        //Dispatch the data in the buffers to the display by creating a TX IRQ on PORT 2.
        CBuff_index = 0;            //Start Index at 0.
        CBuff_max_data = 43;        //End Index at 43 bytes. 44? Arrays start at 0 right?
        IFS1bits.U2TXIF = 1;        //Start transmitting by manually send an IRQ.
    }
}

#endif