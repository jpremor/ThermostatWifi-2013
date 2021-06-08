#include "mbed.h"
#include "WiflyModule.h"
#include "MODSERIAL.h"
extern "C" void mbed_reset();

//***********************
// Defines
#define ON  1
#define OFF  0

#define SERIAL_USB_OUT_BUFFER_SIZE  120         // should be < 255 (255 not included)
#define SERIAL_USB_IN_BUFFER_SIZE  120          // should be < 255 (255 not included)

#define CHAR_COMMA_POINT 59     // char ";" ASCII table
#define CHAR_COMMA 44     // char "," ASCII table
#define CHAR_ENTER_CARRIAGERET      13      // normal order is 13 then 10
#define CHAR_ENTER_NEWLINE  10
#define CHAR_0  48
#define CHAR_1  49
#define CHAR_2  50
#define CHAR_3  51
#define CHAR_4  52
#define CHAR_5  53
#define CHAR_6  54
#define CHAR_7  55
#define CHAR_8  56
#define CHAR_9  57
#define CHAR_AST    42  // ASCII char "*"
#define CHAR_O_UPPER    79  // ASCII char "O"
#define CHAR_P_UPPER    80  // ASCII char "P"

#define SERVER_PORT     2001


#define FUNCTION_ANALOG     97  // ASCII char "a" = 97, define functions for serial communication to analog interface
#define FUNCTION_WIFI       119 // ASCII char "w" = 119, define functions for serial communication to wifi interface
    #define WIFI_IP         105 // ASCII char "I" = 105, IP functions
#define FUNCTION_IO         105 // ASCII char "i" = 105, define functions for serial communication to IOs 
#define FUNCTION_CONFIG     99  // ASCII char "c" = 99, define functions for serial communication to configure
#define FUNCTION_RESET      114 // ASCII char "r" = 114, define functions for system reset
#define FUNCTION_STATUS     115 // ASCII char "s" = 115, define function for full status read. Asnwer all main variables
    #define STATUS_FULL     102 // ASCII char "f" = 102, show full status
    #define STATUS_SHORT    115 // ASCII char "s" = 115, show short status
#define FUNCTION_VAR        118 // ASCII char "v" = 118, define functions for variables reading
 #define COMMAND_READ       114 // ASCII char "r" = 114, read analog input
 #define COMMAND_SET        115 // ASCII char "s" = 115, set analog input
  #define STATE_CLOSE       99  // ASCII char "c" = 99, define state as close
  #define STATE_OPEN        111 // ASCII char "o" = 111, define state as open          


//***********************
// Typedefs

typedef struct
{
    char wifi_enter_cmd : 1;
    char wifi_exit_cmd : 1;
    char wifi_join_cmd : 1;
    char wifi_leave_cmd : 1;
    char wifi_reboot_cmd : 1;
    char wifi_config_cmd : 1;
    char wifi_read_ip : 1;
} T_wifi_commands;

typedef enum
{
    WS_TCP_CLIENT_CONNECTED_MODE,
    WS_TCP_SERVER_UP_MODE,
    WS_CHECK_CONNECTION
} T_wifi_states;

typedef enum
{
    ST_STARTUP = 1, 
    ST_SATISFIED, 
    ST_COOLING, 
    ST_HEATING,
    ST_DISABLED,
} T_states;

typedef struct
{
    char relay_cool_close : 1;
    char relay_cool_open : 1;
    char relay_heat_close : 1;
    char relay_heat_open : 1;
    char relay3_close : 1;
    char relay3_open : 1;
} T_rl_commands;

//***********************
// General Timers
Timer general_timer;

// Tasks timers
Ticker timer_led1;
Ticker timer_digital;
Ticker timer_serial_usb;
Ticker timer_temp_control;
Ticker timer_analog;
Ticker timer_wifi;
Ticker timer_clock;

// Digital IOs definition
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalOut relay1_close(p28);   //working
DigitalOut relay1_open(p30);    //working
DigitalOut relay3_close(p23);   //working
DigitalOut relay3_open(p24);    //working
DigitalOut relay2_close(p29);   // not working
DigitalOut relay2_open(p27);    //working

DigitalIn relay1_fb(p5);
DigitalIn relay2_fb(p13);
DigitalIn relay3_fb(p14);

// Analog channels
AnalogIn    an_thermistor_11(p20);
AnalogIn    an_thermistor_12(p19);
AnalogIn    an_thermistor_13(p18);
AnalogIn    an_thermistor_21(p17);
AnalogIn    an_thermistor_22(p16);
AnalogIn    an_thermistor_23(p15);

// file for parameter storage
LocalFileSystem local("local");               // Create the local filesystem under the name "local"

// Serial port config
MODSERIAL pc_comm_usb(USBTX, USBRX); // tx, rx

// wifi config
WiflyModule wifly(p9, p10, p25, p26, "Remor5024", "asdfgh12", SEC_MIXED);


//***********************
// global vars
static const float temp_table[10] = 
{   42.50521921,    // degrees C
    25.90159711, 
    24.05546624, 
    11.04456157, 
    4.541926542,
    0.646666667,    // AD input
    0.784552846, 
    0.798018293, 
    0.877598152,
    0.907504363,   };
     
int counter;
float thermistor_1_celcius = 0;
float thermistor_2_celcius = 0;
float thermistor_1_measure = 0;
float thermistor_1_inst = 0;
float thermistor_2_measure = 0;
float thermistor_2_inst = 0;

float thermistor_1_sum = 0;
float thermistor_2_sum = 0;
int filter_counter;
#define N_MED_FILTER    166

// FIR filter for thermistor measure
float thermistor_1_measure_k0 = 0;
float thermistor_1_measure_k1 = 0;
float thermistor_2_measure_k0 = 0;
float thermistor_2_measure_k1 = 0;

char string_out[SERIAL_USB_OUT_BUFFER_SIZE];
char string_out_pointer;
char string_in[SERIAL_USB_IN_BUFFER_SIZE];
char string_in_pointer;

enum { PACK_READY, READING } usb_state;

T_rl_commands relay_cmd;
T_states cooling_state;
T_wifi_commands wifi_cmd;
T_wifi_states wifi_state;
 

char relay_cool, relay_heat, relay3;

// configuration setings
float temp_hysteresis;
float temp_reference_cool;
float temp_reference_heat;
char enable_temp_ctrl = 0;
float temp_min_on;
float temp_min_off;
char temp_sens_1;
char temp_sens_2;
char temp_sens_method;   // 0 - medium between two sensors
float temp_measure = 0; // var as input for temp control
float sens1_offset = 0;
float sens1_gain = 1;
float sens2_offset = 0;
float sens2_gain = 1;


long int timer_on, timer_off;
#define MAX_TIMER   30000

//clock
char seconds;
char seconds_alive;
char minutes;
char minutes_alive;
char hours;
char hours_alive;
char weekday;
char weekday_alive;

// wifi variables
char string_ssid[20];
char string_phra[20];





void read_file (void) 
{
    char A[20];

    FILE *set = fopen("/local/setup.txt", "r");  // Open "setup.txt" on the local file system for read
    fscanf(set,"%s %f",A,&temp_reference_cool);
    fscanf(set,"%s %f",A,&temp_reference_heat);
    fscanf(set,"%s %f",A,&temp_hysteresis);
    fscanf(set,"%s %f",A,&temp_min_on);
    fscanf(set,"%s %f",A,&temp_min_off);
    fscanf(set,"%s %d",A,&temp_sens_1);
    fscanf(set,"%s %d",A,&temp_sens_2);
    fscanf(set,"%s %d",A,&temp_sens_method);
    fscanf(set,"%s %s",A,&string_ssid);
    fscanf(set,"%s %s",A,&string_phra);
    fscanf(set,"%s %f",A,&sens1_offset);
    fscanf(set,"%s %f",A,&sens1_gain);
    fscanf(set,"%s %f",A,&sens2_offset);
    fscanf(set,"%s %f",A,&sens2_gain);
    fscanf(set,"%s %d",A,&enable_temp_ctrl);
    fclose(set);
}
 

 
void write_file(void) 
{ // write to USB memory
    FILE *fp = fopen("/local/setup.txt", "w");  // Open "setup.txt" on the local file system for write
    fprintf(fp,"temp_reference_cool %f\r\n", temp_reference_cool); // read offset
    fprintf(fp,"temp_reference_heat %f\r\n", temp_reference_heat); // read offset
    fprintf(fp,"temp_hysteresis %f\r\n",temp_hysteresis);   // read gain
    fprintf(fp,"temp_min_on %f\r\n",temp_min_on);   // read gain
    fprintf(fp,"temp_min_off %f\r\n",temp_min_off);   // read gain
    fprintf(fp,"sensor_1 %d\r\n",temp_sens_1);
    fprintf(fp,"sensor_2 %d\r\n",temp_sens_2);
    fprintf(fp,"sensor_method %d\r\n",temp_sens_method);
    fprintf(fp,"wifi_ssid %s\r\n",string_ssid);
    fprintf(fp,"wifi_phra %s\r\n",string_phra);
    fprintf(fp,"sens1_offset %f\r\n",sens1_offset);
    fprintf(fp,"sens1_gain %f\r\n",sens1_gain);
    fprintf(fp,"sens2_offset %f\r\n",sens2_offset);
    fprintf(fp,"sens2_gain %f\r\n",sens2_gain);
    fprintf(fp,"enable_temp_ctrl %d\r\n",enable_temp_ctrl);
    fclose(fp);
}













//***********************
// Interrupts
// Called everytime a new character goes into
// the RX buffer. Test that character for \n
// Note, rxGetLastChar() gets the last char that
// we received but it does NOT remove it from
// the RX buffer.
void serial_rx_int(MODSERIAL_IRQ_INFO *q) 
{
    char rx_data;
    int buffer_size;

    MODSERIAL *serial = q->serial;
    
    // read the buffer size
    buffer_size = serial->rxBufferGetCount();
    while (buffer_size != 0)
    {
        // get 1st char
        rx_data = serial->rxGetLastChar();
    
        if (rx_data != CHAR_ENTER_NEWLINE)
        {
            string_in[string_in_pointer] = rx_data;
            string_in_pointer++;
        
            if ((rx_data == CHAR_ENTER_CARRIAGERET) |
                (string_in_pointer > SERIAL_USB_IN_BUFFER_SIZE))
            {
                usb_state = PACK_READY;
            }
        }
        buffer_size--;
    }
    pc_comm_usb.rxBufferFlush();
    
}




//***********************
// tasks
//***********************
// ========================
// TEMP CONTROL TASK
// Send and receive data on usb serial port driver
//
// ========================
void temp_control_task()
{
    
    // adjust the measured temperature
    temp_measure = 0;
    if (temp_sens_method == 0)
    {
        if (temp_sens_1 == 1) 
        {
            temp_measure = thermistor_1_celcius;
            if (temp_sens_2 == 1) temp_measure = (temp_measure + thermistor_2_celcius) / 2;
        }
        else if (temp_sens_2 == 1) temp_measure = thermistor_2_celcius;
    }
    else if (temp_sens_method == 1)
    {
    }
    




    // routine disabled
    if (enable_temp_ctrl == 0)
    {
        cooling_state = ST_DISABLED;
        relay_cmd.relay_cool_open = ON;
        relay_cmd.relay_heat_open = ON;
    }
    // routine enabled
    else
    {
        if (cooling_state == ST_STARTUP)
        {
            if (timer_off > temp_min_off)
            {
                cooling_state = ST_SATISFIED;
            }
        }
        else if (cooling_state == ST_SATISFIED)
        {
            // set relay to cool off
            if (relay_cool == ON) relay_cmd.relay_cool_open = ON;
            // set relay to heat off
            if (relay_heat == ON) relay_cmd.relay_heat_open = ON;
            
            // if temperature is high enough to cool and the timer off elapsed
            if ((temp_measure > (temp_reference_cool + temp_hysteresis)) &
                (timer_off > temp_min_off))
            {
                cooling_state = ST_COOLING;
                timer_on = 0;
            }
            // if temperature is high enough to cool and the timer off elapsed
            else if ((temp_measure < (temp_reference_heat - temp_hysteresis)) &
                (timer_off > temp_min_off))
            {
                cooling_state = ST_HEATING;
                timer_on = 0;
            }
        }
        else if (cooling_state == ST_COOLING)
        {
            // set relay to cool on
            if (relay_cool == OFF) relay_cmd.relay_cool_close = ON;
            // set relay to heat off
            if (relay_heat == ON) relay_cmd.relay_heat_open = ON;
            
            //change states
            if ((temp_measure < (temp_reference_cool - temp_hysteresis)) &
                (timer_on > temp_min_on))
            {
                cooling_state = ST_SATISFIED;
                timer_off = 0;
            }
        }
        else if (cooling_state == ST_HEATING)
        {
            // set relay to cool on
            if (relay_cool == ON) relay_cmd.relay_cool_open = ON;
            // set relay to heat off
            if (relay_heat == OFF) relay_cmd.relay_heat_close = ON;
            
            //change states
            if ((temp_measure > (temp_reference_heat + temp_hysteresis)) &
                (timer_on > temp_min_on))
            {
                cooling_state = ST_SATISFIED;
                timer_off = 0;
            }
        }
        else cooling_state = ST_STARTUP;
    } // if enabled temp control
    
    // increment timers
    if ((relay_cool) | (relay_heat)) 
    {
        timer_on++;
        if (timer_on > MAX_TIMER) timer_on = MAX_TIMER;
    }
    else
    {
        timer_off++;
        if (timer_on > MAX_TIMER) timer_on = MAX_TIMER;
    }
}
// ------------------------





// ========================
// CLOCK TASK
// Manage the clock timing
//
// ========================
void clock_task()
{
    seconds++;
    if (seconds > 59) 
    {
        seconds = 0;
        minutes++;
        if (minutes > 59)
        {
            minutes = 0;
            hours++;
            if (hours > 23) hours = 0;
        }
    }
    
    seconds_alive++;
    if (seconds_alive > 59) 
    {
        seconds_alive = 0;
        minutes_alive++;
        if (minutes_alive > 59)
        {
            minutes_alive = 0;
            hours_alive++;
            if (hours_alive > 23) hours_alive = 0;
        }
    }
}
// ------------------------









// ========================
// LED TASK
// Manage the LEDs
//
// ========================
void led1_task()
{
        if (counter>1) led4 = 0;
        else led4 = 1;
        if (counter>20) counter = 0;
        counter++;
}
// ------------------------




// ========================
// ANALOG TASK
// Read analog input and save to global vars
//
// ========================
void analog_task()
{
    float float_temp;
    char char_temp;
    char table_index;

    // read the analog data for thermistor 1
    float_temp = an_thermistor_11.read() + an_thermistor_12.read() + an_thermistor_13.read();
    float_temp = float_temp / 3;
    thermistor_1_inst = float_temp;
    
    float_temp = 0.6*float_temp + 0.55 * thermistor_1_measure_k0 + 0.35 * thermistor_1_measure_k1;
    
    thermistor_1_measure_k1 = thermistor_1_measure_k0;
    thermistor_1_measure_k0 = float_temp;
    thermistor_1_measure = float_temp / 6;
    
    // read the analog data for thermistor 2
    float_temp = an_thermistor_21.read() + an_thermistor_22.read() + an_thermistor_23.read();
    float_temp = float_temp / 3;
    thermistor_2_inst = float_temp;
    
    float_temp = 0.6*float_temp + 0.55 * thermistor_2_measure_k0 + 0.35 * thermistor_2_measure_k1;
    
    thermistor_2_measure_k1 = thermistor_2_measure_k0;
    thermistor_2_measure_k0 = float_temp;
    thermistor_2_measure = float_temp / 6;
    
    // find the table index thermistor 1
    if (thermistor_1_measure < temp_table[5]) thermistor_1_celcius = 0;         //thermistor shorted
    else if (thermistor_1_measure > temp_table[9]) thermistor_1_celcius = 99;   //thermistor open
    else 
    {
        // search in the table
        table_index = 5;
        for (char_temp=5 ; char_temp<9 ; char_temp++)
        {
            if ((thermistor_1_measure > temp_table[char_temp]) &
                (thermistor_1_measure < temp_table[char_temp+1])) table_index = char_temp;
        }
        
        float_temp = (temp_table[table_index-5] - temp_table[table_index-4]);
        float_temp = float_temp * (temp_table[table_index+1] - thermistor_1_measure);
        float_temp = float_temp / (temp_table[table_index+1] - temp_table[table_index]);
        float_temp = float_temp + temp_table[table_index-4];
        thermistor_1_sum = thermistor_1_sum + float_temp;
    }
    
    // find the table index thermistor 2
    if (thermistor_2_measure < temp_table[5]) thermistor_2_celcius = 0;         //thermistor shorted
    else if (thermistor_2_measure > temp_table[9]) thermistor_2_celcius = 99;   //thermistor open
    else 
    {
        // search in the table
        table_index = 5;
        for (char_temp=5 ; char_temp<9 ; char_temp++)
        {
            if ((thermistor_2_measure > temp_table[char_temp]) &
                (thermistor_2_measure < temp_table[char_temp+1])) table_index = char_temp;
        }
        
        float_temp = (temp_table[table_index-5] - temp_table[table_index-4]);
        float_temp = float_temp * (temp_table[table_index+1] - thermistor_2_measure);
        float_temp = float_temp / (temp_table[table_index+1] - temp_table[table_index]);
        float_temp = float_temp + temp_table[table_index-4];
        thermistor_2_sum = thermistor_2_sum + float_temp;
    }
    
    
    filter_counter++;
    if (filter_counter == N_MED_FILTER)
    {
        thermistor_1_celcius = thermistor_1_sum / N_MED_FILTER;
        thermistor_1_celcius = (thermistor_1_celcius*sens1_gain) + sens1_offset;
        thermistor_2_celcius = thermistor_2_sum / N_MED_FILTER;
        thermistor_2_celcius = (thermistor_2_celcius*sens2_gain) + sens2_offset;
        thermistor_1_sum = 0;
        thermistor_2_sum = 0;
        filter_counter = 0;
    }    
}
// ------------------------







void protocol_check_cmd(const char * string_in, char * str_out)
{
    char temp_str[4];
    char temp_char;
    char * p_end;
     
    // command to read analog input
    if ((string_in[0] == FUNCTION_VAR) &
        (string_in[1] == COMMAND_READ))
    {
             if (string_in[2] == CHAR_1) sprintf(str_out,"timer on: %d\r\n" ,timer_on);
        else if (string_in[2] == CHAR_2) sprintf(str_out,"timer off: %d\r\n" ,timer_off);
        else if (string_in[2] == CHAR_3) sprintf(str_out,"state: %d\r\n" ,cooling_state);
        else if (string_in[2] == CHAR_4) sprintf(str_out,"temp measured: %f\r\n" ,temp_measure);
        else sprintf(str_out,"unknown channel\r\n");
    }
        // command to read digital inputs
        else if (string_in[0] == FUNCTION_WIFI)
        {
            if ((string_in[1] == COMMAND_READ) & (string_in[2] == WIFI_IP))
            {
                sprintf(str_out,"%s\r\n", wifly.getIPAddress());
            }
            else if ((string_in[1] == COMMAND_SET) & (string_in[2] == 115))  // "s"
            {
                
            }
            else if ((string_in[1] == COMMAND_SET) & (string_in[2] == 106))  // "j"
            {
                
            }
            else if ((string_in[1] == COMMAND_SET) & (string_in[2] == 114))  // "r"
            {
                
            }
        }
        // command to read analog input
        else if ((string_in[0] == FUNCTION_ANALOG) &
            (string_in[1] == COMMAND_READ))
        {
            // channel 1, read thermistor
                 if (string_in[2] == CHAR_1) sprintf(str_out,"thermistor 1: %f ; in : %f ; C : %f\r\n" ,thermistor_1_measure, thermistor_1_inst, thermistor_1_celcius);
            // channel 2, read thermistor
            else if (string_in[2] == CHAR_2) sprintf(str_out,"thermistor 2: %f ; in : %f ; C : %f\r\n" ,thermistor_2_measure, thermistor_2_inst, thermistor_2_celcius);
            else sprintf(str_out,"unknown channel\r\n");
        }
        // command to read digital inputs
        else if ((string_in[0] == FUNCTION_IO) &
            (string_in[1] == COMMAND_READ))
        {
            // channel 1, read relay 1 status
            if (string_in[2] == CHAR_1)
            {
                if (relay_cool) sprintf(str_out,"relay COOL is close\r\n");
                else sprintf(str_out,"relay COOL is open\r\n");
            }
            
            // channel 2, read relay 2 status
            else if (string_in[2] == CHAR_2)
            {
                if (relay_heat) sprintf(str_out,"relay HEAT is close\r\n");
                else sprintf(str_out,"relay HEAT is open\r\n");
            }
            
            // channel 3, read relay 3 status
            else if (string_in[2] == CHAR_3) 
            {
                if (relay3) sprintf(str_out,"relay 3 is close\r\n");
                else sprintf(str_out,"relay 3 is open\r\n");
            }
            else sprintf(str_out,"unknown relay\r\n");
        }
    
        // command to write digital outputs
        else if ((string_in[0] == FUNCTION_IO) &
            (string_in[1] == COMMAND_SET))
        {
            if (string_in[3] == STATE_OPEN)
            {
                     if (string_in[2] == CHAR_1) relay_cmd.relay_cool_open = ON;
                else if (string_in[2] == CHAR_2) relay_cmd.relay_heat_open = ON;
                else if (string_in[2] == CHAR_3) relay_cmd.relay3_open = ON;
                sprintf(str_out,"relay %c set open\r\n",string_in[2]);
            }
            else if (string_in[3] == STATE_CLOSE)
            {
                     if (string_in[2] == CHAR_1) relay_cmd.relay_cool_close = ON;
                else if (string_in[2] == CHAR_2) relay_cmd.relay_heat_close = ON;
                else if (string_in[2] == CHAR_3) relay_cmd.relay3_close = ON;
                sprintf(str_out,"relay %c set closed\r\n",string_in[2]);
            }
            else sprintf(str_out,"wrong state\r\n");
        }

        // command to configure the module settings
        else if (string_in[0] == FUNCTION_CONFIG)
        {
            if (string_in[1] == COMMAND_READ)
            {
                     if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_1)) sprintf(str_out,"temp hyst = %f;\r\n", temp_hysteresis);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_2)) sprintf(str_out,"ref  cool = %f;\r\n", temp_reference_cool);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_3)) sprintf(str_out,"ref  heat = %f;\r\n", temp_reference_heat);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_4)) sprintf(str_out,"min    on = %f;\r\n", temp_min_on);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_5)) sprintf(str_out,"min   off = %f;\r\n", temp_min_off);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_6)) sprintf(str_out,"sensor 1  = %d;\r\n", temp_sens_1);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_7)) sprintf(str_out,"sensor 2  = %d;\r\n", temp_sens_2);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_8)) sprintf(str_out,"sens meth = %d;\r\n", temp_sens_method);
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_9)) sprintf(str_out,"wifi ssid = %s;\r\n", string_ssid);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_0)) sprintf(str_out,"wifi pass = %s;\r\n", string_phra);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_1)) sprintf(str_out,"s1 offset = %f;\r\n", sens1_offset);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_2)) sprintf(str_out,"s1 gain   = %f;\r\n", sens1_gain);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_3)) sprintf(str_out,"s2 offset = %f;\r\n", sens2_offset);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_4)) sprintf(str_out,"s2 gain   = %f;\r\n", sens2_gain);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_5)) sprintf(str_out,"TC enable = %d;\r\n", enable_temp_ctrl);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_6)) sprintf(str_out,"seconds   = %d\r\n" ,seconds);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_7)) sprintf(str_out,"minutes   = %d\r\n" ,minutes);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_8)) sprintf(str_out,"hours     = %d\r\n" ,hours);
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_9)) sprintf(str_out,"week day  = %d\r\n" ,weekday);
                else sprintf(str_out,"e: set doesnt exist\r\n");
            }
            else if (string_in[1] == COMMAND_SET)
            {
                // save string
                temp_str[0] = string_in[4];
                temp_str[1] = string_in[5];
                temp_str[2] = string_in[6];
                temp_str[3] = string_in[7];
                if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_1))
                {
                    temp_hysteresis = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"temp hyst set to %f\r\n", temp_hysteresis);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_2))
                {
                    temp_reference_cool = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"ref  cool set to %f\r\n", temp_reference_cool);

                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_3)) 
                {
                    temp_reference_heat = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"ref  heat set to %f\r\n", temp_reference_heat);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_4))
                {
                    temp_min_on = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"min   off set to %f\r\n", temp_min_off);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_5))
                {
                    temp_min_off = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"min    on set to %f\r\n", temp_min_off);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_6))
                {
                    temp_sens_1 = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"sens    1 set to %d\r\n", temp_sens_1);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_7))
                {
                    temp_sens_2 = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"sens    2 set to %d\r\n", temp_sens_2);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_8))
                {
                    temp_sens_method = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"sens meth set to %d\r\n", temp_sens_method);
                }
                else if ((string_in[2] == CHAR_0) & (string_in[3] == CHAR_9))
                {
                    sprintf(str_out,"use USB to update\r\n");
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_0))
                {
                    sprintf(str_out,"use USB to update  \r\n");
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_1))
                {
                    sens1_offset = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"s1 offset set to %f\r\n", sens1_offset);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_2))
                {
                    sens1_gain = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"s1 gain   set to %f\r\n", sens1_gain);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_3))
                {
                    sens2_offset = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"s2 offset set to %f\r\n", sens2_offset);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_4))
                {
                    sens2_gain = strtof(temp_str,&p_end);
                    write_file();
                    sprintf(str_out,"s2 gain   set to %f\r\n", sens2_gain);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_5))
                {
                    if (temp_str[0] == CHAR_0) enable_temp_ctrl = 0;
                    else enable_temp_ctrl = 1;
                    write_file();
                    sprintf(str_out,"TC enable set to %d\r\n", enable_temp_ctrl);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_6))
                {
                    temp_char = (temp_str[0] - 48)*10 + (temp_str[1] - 48);
                    seconds = temp_char;
                    sprintf(str_out,"seconds set to   %d\r\n", seconds);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_7))
                {
                    temp_char = (temp_str[0] - 48)*10 + (temp_str[1] - 48);
                    minutes = temp_char;
                    sprintf(str_out,"minutes set to   %d\r\n", minutes);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_8))
                {
                    temp_char = (temp_str[0] - 48)*10 + (temp_str[1] - 48);
                    hours = temp_char;
                    sprintf(str_out,"hours   set to   %d\r\n", hours);
                }
                else if ((string_in[2] == CHAR_1) & (string_in[3] == CHAR_9))
                {
                    temp_char = (temp_str[0] - 48)*10 + (temp_str[1] - 48);
                    weekday = temp_char;
                    sprintf(str_out,"weekday set to   %d\r\n", weekday);
                }
                else sprintf(str_out,"e: set doesnt exist\r\n");
            }
        }
        else if (string_in[0] == FUNCTION_STATUS)
        {
            if (string_in[1] == STATUS_FULL)
            {
                sprintf(str_out,"state:%d;temp measured:%f;refcool:%f;refheat:%f;sens1:%f;sens2:%f;relaycool:%d;relayheat:%d;relay3:%d;s1off:%f;s1gain:%f;s2off:%f;s2gain:%f;tcenable:%d;hours:%d;minutes:%d;seconds:%d;\r\n", 
                    cooling_state, 
                    temp_measure, 
                    temp_reference_cool, 
                    temp_reference_heat,
                    thermistor_1_celcius,
                    thermistor_2_celcius,
                    relay_cool,
                    relay_heat,
                    relay3,
                    sens1_offset,
                    sens1_gain,
                    sens2_offset,
                    sens2_gain,
                    enable_temp_ctrl,
                    hours,
                    minutes,
                    seconds/*,
                    hours_alive,
                    minutes_alive,
                    seconds_alive*/);
            }
            else if (string_in[1] == STATUS_SHORT)
            {
                sprintf(str_out,"state:%d;temp measured:%f;refcool:%f;refheat:%f;\r\n", 
                    cooling_state, 
                    temp_measure, 
                    temp_reference_cool, 
                    temp_reference_heat);
            }
            else sprintf(str_out,"e: status not defined\r\n");
        }
        
        else if (string_in[0] == FUNCTION_RESET) 
        {
            sprintf(str_out,"\r\nreseting...\r\n");
            mbed_reset();
        }
        
        else if ((string_in[0] == CHAR_AST) & (string_in[1] == CHAR_O_UPPER) & (string_in[2] == CHAR_P_UPPER))
        {
            // dont answer, success to connect to TCP answer received
            sprintf(str_out,"");
        }
        
        else sprintf(str_out,"e: unknown command:%d:%d:%d\r\n",string_in[0],string_in[1],string_in[2]);
        
}





// ========================
// SERIAL USB TASK
// Send and receive data on usb serial port driver
//
// ========================
void serial_usb_task()
{
    char string_out[256];
    //char rele1;
    
    
    //pc_comm_usb.printf("thermistor: %f    relay 1 FB: %01d  relay 2 FB: %01d  relay 3 FB: %01d;\r\n" ,thermistor_1_measure,rele1,relay2,relay3);
    
        
    if (usb_state == PACK_READY)
    {
        protocol_check_cmd(string_in, string_out);
        pc_comm_usb.printf(string_out);
                 
        string_in_pointer = 0;
        usb_state = READING;
    }
    
}
// ------------------------














// ========================
// WIFI TASK
// Send and receive data over wifi
//
// ========================
void wifi_task()
{
    char buffer_str[256];
    char string_out[256];
    char n;

    // state machine
    if (wifi_state == WS_CHECK_CONNECTION) 
    {
       // wifi_startup - try to connect again
    }
    else if (wifi_state == WS_TCP_SERVER_UP_MODE)
    {
        
        // check if received a connection
        if (wifly.is_connected())
        {
            wifi_state = WS_TCP_CLIENT_CONNECTED_MODE;
            //tcp_server.accept(tcp_client);
            pc_comm_usb.printf("wifi: client connected\r\n");
            
        }
    }
    else if (wifi_state == WS_TCP_CLIENT_CONNECTED_MODE)
    {
        // check if received a connection
        if (!wifly.is_connected())
        {
            wifi_state = WS_TCP_SERVER_UP_MODE;
            pc_comm_usb.printf("wifi: client DISconnected\r\n");
        }
        
        // check if data is available
        n = wifly.get_string(buffer_str);
        if (n>0) 
        {
            protocol_check_cmd(buffer_str, string_out);
            wifly.printf(string_out);
        }
        
        /*
        if (n <= 0)
        {
            buffer[n] = 0;
            tcp_client.send_all(buffer, n);
        }
        
        */
    }
    
    
    
    // ----
    // commands
    if (wifi_cmd.wifi_config_cmd == ON)
    {
        
    }
}
// ------------------------









// ========================
// DIGITAL TASK
// Send and receive data on usb serial port driver
//
// ========================
void digital_task()
{
    // ======================
    // read digital feedbacks
    if (relay1_fb) { relay_cool = OFF; led1 = OFF; }
    else { relay_cool = ON; led1 = ON; }
    if (relay2_fb) { relay_heat = OFF; led2 = OFF; }
    else { relay_heat = ON; led2 = ON; }
    if (relay3_fb) { relay3 = OFF; led3 = OFF; }
    else { relay3 = ON; led3 = ON; }
    
    // ======================
    // set digital out open
    if (relay_cmd.relay_cool_open)    
    {        
        relay1_open = ON;        
        relay_cmd.relay_cool_open = OFF;    
    }
    else 
    {
        relay1_open = OFF;
    
        // set digital out close
        if ((relay_cmd.relay_cool_close)/* & (relay2_fb == OFF)*/)   // set interlock to close
        {        
            relay1_close = ON;       
            relay_cmd.relay_cool_close = OFF;    
        }
        else 
        {
            relay1_close = OFF;
    
            // set digital out open
            if (relay_cmd.relay_heat_open)    
            {        
                relay2_open = ON;        
                relay_cmd.relay_heat_open = OFF;    
            }
            else 
            {    
                relay2_open = OFF;
        
                // set digital out close
                if ((relay_cmd.relay_heat_close)/* & (relay1_fb == OFF)*/)    // set interlock to close
                {        
                    relay2_close = ON;       
                    relay_cmd.relay_heat_close = OFF;   
                }
                else 
                {
                    relay2_close = OFF;
    
                    // set digital 3 out open
                    if (relay_cmd.relay3_open)    
                    {        
                        relay3_open = ON;        
                        relay_cmd.relay3_open = OFF;    
                    }
                    else 
                    {
                        relay3_open = OFF;
                        
                        // set digital 3 out close
                        if (relay_cmd.relay3_close)   
                        {       
                            relay3_close = ON;       
                            relay_cmd.relay3_close = OFF;    
                        }
                        else relay3_close = OFF;
                        
                    }
                }
            }
        }
    }
}
// ------------------------









void wifi_startup()
{
        TJoinOutput JoinOutput = JOIN_FAIL;

        pc_comm_usb.printf("Init wifi with DHCP...\r\n");
        wifly.init(string_ssid, string_phra); // use DHCP
        
        //pc_comm_usb.printf("Connecting to router...\r\n");
        // cant connect to router, try re-configuring
        //if (wifly.connect_router() == false) 
        //{
            pc_comm_usb.printf("Config wifi module and connect to router...\r\n");
            JoinOutput = wifly.config();    // configure the wifi module
        //}
        // sucess connecting to router
        //else 
        //{   
        //    pc_comm_usb.printf("s: Success connecting to router...\r\n");
        //    JoinOutput = ALL_CMD_SUCCESS;
        //}

        if (JoinOutput == ALL_CMD_SUCCESS)
        {
            printf("s: IP Address is %s\r\n", wifly.getIPAddress());
            wifly.set_tcp_server(2001);       // nao ta funcionando ainda
            
            printf("s: wait for TCP connection...\r\n");
            wifi_state = WS_TCP_SERVER_UP_MODE;
        }
        else 
        {
            pc_comm_usb.printf("e: failed to set join mode\r\n"); 
        }
        
}


//***********************
// Setup function, init
//***********************
void setup()
{
    //serial int config
    pc_comm_usb.baud(9600);
    pc_comm_usb.attach(&serial_rx_int, MODSERIAL::RxIrq);
    usb_state = READING;    // set USB state to "waiting data"
    pc_comm_usb.printf("\r\n\r\nWifly Thermostat...\r\n");     // new line carrier return

    // read parameters from file
    pc_comm_usb.printf("Reading config file...\r\n");     // new line carrier return
    read_file();
    
    
    // init wifi module, apply reset
    wifi_state = WS_CHECK_CONNECTION;
    wifi_startup();        

    // relay out configuration
    relay1_fb.mode(PullDown);
    relay2_fb.mode(PullDown);
    relay3_fb.mode(PullDown);
    pc_comm_usb.printf("Reboot complete...\r\n");     // new line carrier return

    // led state
    led1 = 0;
    led2 = 0;
    led3 = 0;
    led4 = 0;

    
    //set relay states
    pc_comm_usb.printf("Reseting relays...\r\n");     // new line carrier return
    relay_cmd.relay_cool_open = ON;
    relay_cmd.relay_heat_open = ON;
    relay_cmd.relay3_open = ON;

    // set startup variables
    cooling_state = ST_STARTUP;
    timer_on = 0;
    timer_off = 0;
    seconds_alive = 0;
    minutes_alive = 0;
    hours_alive = 0;
    

    //tasks
    pc_comm_usb.printf("Starting task manager...\r\n");     // new line carrier return
    general_timer.start();
    timer_led1.attach(&led1_task, 0.06);    // 300ms task 
    timer_digital.attach(&digital_task, 0.09);
    timer_analog.attach(&analog_task, 0.12);    // 20ms task
    timer_serial_usb.attach(&serial_usb_task, 0.2);
    timer_wifi.attach(&wifi_task, 0.39);
    timer_temp_control.attach(&temp_control_task, 1);
    timer_clock.attach(&clock_task, 1);

    pc_comm_usb.printf("Startup finished, ready to receive commands.\r\n");     // new line carrier return
}



//***********************
// Main
//***********************
int main() 
{
    setup();
    
    while(1) { }
}
