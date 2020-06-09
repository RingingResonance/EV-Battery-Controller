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

#ifndef SUBS_H
#define	SUBS_H


extern void explody_preventy_check(void);
extern void heater_calibration(void);
extern void fault_log(int);
extern void general_shutdown(void);
extern void io_off(void);
extern void fault_read(int,int);
extern void regulate(void);
extern void config_save(void);
extern void config_load(void);
extern void calc_save(void);
extern void calc_load(void);
extern void current_cal(void);
extern void volt_percent(void);
extern void print_vars(int);
extern void reset_check(void);
extern void hud_vars(int);
extern void heat_control(float);
extern void POS_disp_init(void);
extern void POS_diag_display(void);
extern void POS_dash_display(void);
extern void main_power_check(void);
extern void first_check(void);
extern void power_off(void);
extern void analog_sanity(void);

/*****************************/
/* Init vars and stuff. */
/*Temperatures are in C */
/*****************************/
//TODO: Implement a way to configure these variables externally and save them to EEPROM
//Analog input constants
float   R1_resistance = 200;            //R1 resistance in Kohms
float   R2_resistance = 16;             //R2 resistance in Kohms
float   bt_vlt_adjst = 0;               //battery voltage input compensation in volts.
/*****************************/
//Battery Ratings and setpoints
float   partial_charge = 0.90;            //Percentage of voltage to charge the battery up to. Set to 0 to disable.
float   max_battery_voltage = 59.08;    //Max battery voltage before shutdown.     
float   battery_rated_voltage = 58.8;           //Target max charge voltage
float   dischrg_voltage = 39.2;        //Minimum battery voltage
float   low_voltage_shutdown = 35;    //Battery Low Total Shutdown Voltage
float   dischrg_C_rating = 2;       //Discharge C rating
float   limp_current = 5;              //Limp mode current in amps
float   chrg_C_rating = 0.5;          //Charge C rating.
float   amp_hour_rating = 22;         //Battery amp hour rating.
float   over_current_shutdown = 30;        //Shutdown current. Sometimes the regulator isn't fast enough and this happens.
float   absolute_max_current = 25;      //Max regulating current.
//Charge temps.
float   chrg_min_temp = 10;          //Battery minimum charge temperature. Stop Charging at this temp.
float   chrg_reduce_low_temp = 15;      //Reduce charge current when lower than this temp.
float   chrg_max_temp = 40;          //Battery max charge temp. Stop charging at this temp.
float   chrg_reduce_high_temp = 35; //Reduce charge current when higher than this temp.
float   chrg_target_temp = 25;      //Battery heater charge target temp. Keeps us nice and warm in the winter time.
//Discharge temps.
float   dischrg_min_temp = 0;       //Battery minimum discharge temperature.
float   dischrg_reduce_low_temp = 10;    //Reduced current discharge low temperature.
float   dischrg_max_temp = 55;       //Battery max discharge temperature.
float   dischrg_reduce_high_temp = 50;       //Battery reduced discharge high temperature.
float   dischrg_target_temp = 15;      //Battery heater discharge target temp. Keeps us nice and warm in the winter time.
//Shutdown temps.
float   battery_shutdown_temp = 60;      //Max battery temp before shutting down everything.
float   ctrlr_shutdown_temp = 80;        //Max motor or motor controller temp shutdown.
//Fan ctrl temps.
float   ctrlr_fan_start = 50;               //Turns on cooling fan.
float   batt_fan_start = 30;
//Some other stuff.
float   max_heat = 50;              //Heater watts that you want to use.
float   travel_dist = 1200;         //Travel Distance in CM per tire rotation.
float   circuit_draw = 0.05;        //Amount of current that Yeti himself draws. Used for current calibration.
/*****************************/
//TODO: Implement a way to configure these variables externally and save them to EEPROM
// Fault Codes.
int fault_codes[10];
int fault_count;
/*****************************/
/*****************************/
/*****************************/
/*****************************/
// Calculated Battery Ratings
double  battery_capacity;       //Calculated total battery capacity in ah
float   absolute_battery_usage = 0;   //Max total power used from battery.
int partial_chrg_cnt = 0;
int PowerOffAfter = 120;//Power off the system after this many minutes of not being plugged in or keyed on. 120 minutes is 2 hours.
/*****************************/
/*****************************/
/* End of SAVE vars. */
/*****************************/
/*****************************/
// Calculated battery values. These don't need to be saved on shutdown.
float   chrge_rate = 0;             //calculated charge rate based off temperature
float   vltg_dvid = 0;              //Value for calculating the ratio of the input voltage divider.
float   calc_125 = 0.00003472;          //Value for calculating the total current in and out of battery.
int     ADCON3upper8 = 0;
int     ADCON3lower8 = 0;
double analog_smpl_time = 0;
float crnt_proportion = 70000;
double crnt_error = 0;
double crnt_integral = 0;
float vltg_proportion = 70000;
double current_output = 0;
float temp_proportion = 70000;
double voltage_output = 0;
float   watts;                  //watts in or out of battery.
float   temp_dischrg_rate;      //calculated discharge rate based off temp and capacity remaining. Is in %
float   max_chrg_current;       //calculated charge rate based off temp and capacity remaining. Is in %
double  battery_usage;          //Calculated Ah usage in/out of battery
double  battery_usage_smll;
float   chrg_voltage;
double  battery_remaining;      //Calculated remaining capacity in battery.
double  battery_remaining_smll;
float   chrg_percent;           //Percentage of battery charge
int     heat_set;               //Calculated heater output for wattage shown.
float   current_compensate;     //Current compensation.
int     heat_cal_stage = 0;     //0 - 4, stage 0 = not run, set 1 to start, stage 2 = in progress, stage 3 = completed, 4 is Error. 5 is disable heater.
int     use_POS_display = 1;
int     zero_current = 0;
float   peak_power = 0;
float   peak_pwr_vlts = 0;
float   peak_pwr_crnt = 0;
/*****************************/
//Battery Sensor Input
float   battery_temp = 0;           //Battery Temperature
float   my_temp = 0;                //Controller board Temperature
float   motor_ctrl_temp = 0;         //Motor or Motor controller Temperature
float   battery_voltage = 0;        //Battery voltage
float   open_voltage = 0;           //Battery Open Circuit Voltage
float   voltage_percentage = 0;     //Battery Voltage Percentage.
float   battery_current = 0;        //Battery charge/discharge current
/*****************************/
/* General Vars */
int got_open_voltage = 0;
int oc_shutdown_timer = 0;
float dischr_current = 0;
int p_charge = 0;
float speed = 0;                  //How fast are we going?
int soft_power = 0;
int main_power = 0;
int pwr_detect = 0;
int init_done = 0;
int lw_pwr = 0;
int deep_sleep = 0;
int analog_avg_cnt = 0;
double adctemp0 = 0;
double adctemp1 = 0;
double adctemp2 = 0;
double adctemp3 = 0;
double adctemp4 = 0;
int crnt_avg_cnt = 0;
int vltg_avg_cnt = 0;
float battery_crnt_average = 0;        //Battery charge/discharge current
float battery_vltg_average = 0;
double bt_crnt_avg_temp = 0;
double bt_vltg_avg_temp = 0;
int adc_sample_burn = 0;            //Burn it. Don't touch this var it will burn you if you do.
int read_vars = 0;
int curnt_cal_stage = 0;     
/* 0 - 4, stage 0 = not run, set 1 to start, stage 2 = in progress, stage 3 = completed, 4 is Error. 
 */
int fault_shutdown = 0;       //General shutdown event.
int cmd_power = 0;
int b_safe = 1; 
/* Always start up with this != 0x55FF. It should only ever be set to 0x55FF 
 * when in a controlled debugging environment because this disables 
 * explody-preventy-check and other shutdown systems.
 */
int error_blink = 0;
int power_session = 1;
int power_plugged = 0;
int shutdown_timer = 0;
int first_cal = 0;
int chrg_check = 0;
int start_timer = 0;
int osc_fail_event = 0;
int hunter = 0;
int dispinit = 0;
int err_scroll = 0;
int diagmode = 0;
int diag_count = 0;
int diag_state = 0;
int charger_detected = 0;       //Used for confirming the charger is plugged in.
int EnableChIRQ = 1;            //By default, enable charge detect IRQ on init.
int PowerOffTimer = 0;
int PowerOffTimerSec = 59;      //default state.

/*****************************/
//Control Output
int     output_power = 0;          //output power
int     charge_power = 0;           //charge rate
int     heat_power = 0;             //heater power
/*****************************/


#endif	/* SUBS_H */

