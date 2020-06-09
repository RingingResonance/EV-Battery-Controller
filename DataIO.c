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

#ifndef DATAIO_C
#define DATAIO_C

#include <p30f3011.h>
#include "DataIO.h"
#include "subs.h"

//TODO: Not Finished. What's not finished here?
void smpl_err_send(int serial_port){    
    if (serial_port == PORT1){
        U1TXREG = four_bit_hex_cnvt((fault_codes[err_scroll] & 0xF0) / 16);
        U1TXREG = four_bit_hex_cnvt(fault_codes[err_scroll] & 0x0F);
    }
    //Transmit on port 2
    if (serial_port == PORT2){
        U2TXREG = four_bit_hex_cnvt((fault_codes[err_scroll] & 0xF0) / 16);
        U2TXREG = four_bit_hex_cnvt(fault_codes[err_scroll] & 0x0F);
    }
    //Save to Buffer
    if (serial_port == BigBuffer){
        if (CBuff_index < 45){
            CBuff_index++;
        }
        Port2_Buffer[CBuff_index] = four_bit_hex_cnvt((fault_codes[err_scroll] & 0xF0) / 16);
        if (CBuff_index < 45){
            CBuff_index++;
        }
        Port2_Buffer[CBuff_index] = four_bit_hex_cnvt(fault_codes[err_scroll] & 0x0F);
    }
    if (serial_port == 0x00){
        fault_log(0x1A);        //Log invalid port error.
    }
}

char four_bit_hex_cnvt(int numb){
    char asci_hex = 0;
    int temp = 0;
    temp = 0x0F & numb;
    
    if(temp < 10){
        asci_hex = temp + 48;
    }
    else{
        asci_hex = temp + 55;
    }
    return asci_hex;
}

void data_send(char data, int serial_port){
    while (1){
        if (U1STAbits.UTXBF == 0 && serial_port == 0x01){
            U1TXREG = data;     //send data out on UART1
            break;           //wait until we can send data to the buffer.
        }
        if (U2STAbits.UTXBF == 0 && serial_port == 0x02){
            U2TXREG = data;     //send data out on UART2
            break;           //wait until we can send data to the buffer.
        }
        if (serial_port == 0x00){
            fault_log(0x1A);        //Log invalid port error.
            break;
        }
    }
}

//Send a string of text to UART 1 or 2
//nl value of 1 sends newline and return signal before text.
//nl value of 2 sends newline and return signal after text.
//nl value of 3 sends newline and return signal before and after text.
void send_string(int nl, char *string_point, int serial_port){
    int x = 0;
    if (nl == 1 || nl == 3){
        nl_send(serial_port);
        return_send(serial_port);
    }
    while (string_point[x] != 0){
            //Transmit on port 1
            if (U1STAbits.UTXBF == 0 && serial_port == 0x01){
                U1TXREG = string_point[x];
                x++;
            }
            //Transmit on port 2
            if (U2STAbits.UTXBF == 0 && serial_port == 0x02){
                U2TXREG = string_point[x];
                x++;
            }
            //Save to Buffer
            if (serial_port == BigBuffer){
                if (CBuff_index < 45){
                    CBuff_index++;
                }
                Port2_Buffer[CBuff_index] = string_point[x];
                x++;
            }
            if (serial_port == 0x00){
                fault_log(0x1A);        //Log invalid port error.
                x = 0;
                break;
            }
        }
    if (nl == 2 || nl == 3){
        nl_send(serial_port);
        return_send(serial_port);
    }
}

/* Sends a float through the UART*/
void float_send(float f_data, int serial_port){
    
    int x = 0;
    float tx_float = 0;
    int tx_temp = 0;
    //f_data = 0.0010;

    float_out[0] = ' ';        //Put a SPACE in "0"
    if (f_data < 0){
        f_data *= -1;
        float_out[0] = '-';    //Put a - in "0"
    }
    if (f_data > 9999.999){
        f_data = 9999.999;
        float_out[0] = '?';    //Put a ? in "0"
    }
    tx_float = f_data / 1000;
    x++;
    while (x <= 8){
        if (x == 5){
            float_out[5] = '.';
            x++;
        }
        tx_temp = tx_float;
        float_out[x] = tx_temp + 48;
        x++;
        tx_float = (tx_float - tx_temp) * 10;
    }

/* Write to UART 1 or 2 */
    x = 0;
    int ifzero = 1;
    while (x <= 8){
        if(float_out[x] == 0x30 && ifzero){
            if(space){
                if (U1STAbits.UTXBF == 0 && serial_port == PORT1){
                    U1TXREG = 0x20;
                    x++;
                }
                if (U2STAbits.UTXBF == 0 && serial_port == PORT2){
                    U2TXREG = 0x20;
                    x++;
                }
                //Save to Buffer
                if (serial_port == BigBuffer){
                    if (CBuff_index < 45){
                        CBuff_index++;
                    }
                    Port2_Buffer[CBuff_index] = 0x20;
                    x++;
                }
            }
            else{
                x++;
            }
        }
        else{
            if (U1STAbits.UTXBF == 0 && serial_port == PORT1){
                U1TXREG = float_out[x];
                x++;
            }
            if (U2STAbits.UTXBF == 0 && serial_port == PORT2){
                U2TXREG = float_out[x];
                x++;
            }
            //Save to Buffer
            if (serial_port == BigBuffer){
                if (CBuff_index < 45){
                    CBuff_index++;
                }
                Port2_Buffer[CBuff_index] = float_out[x];
                x++;
            }
            if (serial_port == 0x00){
                fault_log(0x1A);        //Log invalid port error.
                x = 0;
                break;
            }
        }
        if(float_out[x] != 0x30 && float_out[x] != 0x2D && float_out[x] != 0x20){
            ifzero = 0;
        }
    }
    space = 0;
}

void nl_send(int serial_port){
    while (1){
        if (U1STAbits.UTXBF == 0 && serial_port == 0x01){
            U1TXREG = 0x0A;     //send new line out on UART1
            break;           //wait until we can send data to the buffer.
        }
        if (U2STAbits.UTXBF == 0 && serial_port == 0x02){
            U2TXREG = 0x0A;     //send new line out on UART2
            break;           //wait until we can send data to the buffer.
        }
        //Save to Buffer
        if (serial_port == BigBuffer){
            if (CBuff_index < 45){
                CBuff_index++;
            }
            Port2_Buffer[CBuff_index] = 0x0A;
            break;
        }
        if (serial_port == 0x00){
            fault_log(0x1A);        //Log invalid port error.
            break;
        }
    }
}

void return_send(int serial_port){
    while (1){
        if (U1STAbits.UTXBF == 0 && serial_port == 0x01){
            U1TXREG = 0x0D;     //send return out on UART1
            break;           //wait until we can send data to the buffer.
        }
        if (U2STAbits.UTXBF == 0 && serial_port == 0x02){
            U2TXREG = 0x0D;     //send return out on UART2
            break;           //wait until we can send data to the buffer.
        }
        //Save to Buffer
        if (serial_port == BigBuffer){
            if (CBuff_index < 45){
                CBuff_index++;
            }
            Port2_Buffer[CBuff_index] = 0x0D;
            break;
        }
        if (serial_port == 0x00){
            fault_log(0x1A);        //Log invalid port error.
            break;
        }
    }
}


unsigned int BaudCalc(double BD, double mlt){
    /* Calculate baud rate. */
    double INS;
    double OutPut;
    unsigned int Oputs;
    INS = mlt * 1000000;
    OutPut = ((INS/BD)/16)-1;
    Oputs = OutPut;
    return Oputs;             //Weird things happen when you try to calculate
                              //a float directly into an int.
    /************************/
}

#endif