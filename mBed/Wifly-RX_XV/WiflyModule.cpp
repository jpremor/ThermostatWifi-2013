/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "mbed.h"
#include "WiflyModule.h"
#include <string>
#include <algorithm>

WiflyModule * WiflyModule::inst;

WiflyModule::WiflyModule(   PinName tx, PinName rx, PinName _reset, PinName tcp_status, const char * ssid, const char * phrase, T_Security security):
    wifi_serial(tx, rx), reset_pin(_reset), tcp_status(tcp_status)
{
    memset(&wifi_state, 0, sizeof(wifi_state));
    wifi_state.security = security;

    // change all ' ' in '$' in the ssid and the passphrase
    strcpy(this->ssid, ssid);
    for (int i = 0; i < strlen(ssid); i++) {
        if (this->ssid[i] == ' ')
            this->ssid[i] = '$';
    }
    strcpy(this->phrase, phrase);
    for (int i = 0; i < strlen(phrase); i++) {
        if (this->phrase[i] == ' ')
            this->phrase[i] = '$';
    }

    inst = this;
    wifi_state.cmd_mode = false;
}


bool WiflyModule::connect_router()
{
    // put in cmd mode
    if (cmd_mode_set(true) == false) return false;
    
    if (sendCommand("join\r", "DHCP=ON", NULL, 10000)) 
    {
        cmd_mode_set(false);
        wifi_state.associated = true;
        return true;
    }
    else return false;
}

bool WiflyModule::set_tcp_server(int port_n)
{
    char cmd[30];

    // put in cmd mode
    if (cmd_mode_set(true) == false) return false;
    
        // config the TCP port to listen
        sprintf(cmd, "set ip localport %d\r\n",port_n);
        sendCommand(cmd, "AOK");

        sprintf(cmd, "set ip flags 2\r\n");
        sendCommand(cmd, "AOK");

        sprintf(cmd, "set ip protocol 1\r\n");
        sendCommand(cmd, "AOK");

        if (!sendCommand("save\r", "Stor")) return false;
        
    if (cmd_mode_set(false) == false) return false;
    return true;
}


TJoinOutput WiflyModule::config()
{
    char cmd[30];
    TJoinOutput output = ALL_CMD_SUCCESS;

        // no auto join to start... set back to autojoin at the end
        sprintf(cmd, "set wlan join 0\r");
        /*if (! */sendCommand(cmd, "AOK"); //) {} output = WLAN_JOIN_SET_FAIL;

        // ssid
        sprintf(cmd, "set wlan ssid %s\r", ssid);
        if (sendCommand(cmd, "AOK")) output = WLAN_SSID_SET_FAIL;

        //key step
        if (wifi_state.security == SEC_WEP_128)
            sprintf(cmd, "set w k %s\r", phrase);
        else // any other security type
            sprintf(cmd, "set wlan phrase %s\r", phrase);
        if (sendCommand(cmd, "AOK")) output = WLAN_SET_PHRASE_FAIL;


        //no echo
        if (!sendCommand("set uart mode 1\r", "AOK")) output = UART_MODE_SET_FAIL;
 
        // set comm time
        sprintf(cmd, "set comm time 30\r");
        if (!sendCommand(cmd, "AOK")) output = COMM_SET_TIME_FAIL;

        // set size
        sprintf(cmd, "set comm size 1024\r");
        if (!sendCommand(cmd, "AOK")) output = COMM_SET_SIZE_FAIL;

        // red led on when tcp connection active
        sprintf(cmd, "set sys iofunc 0x40\r");
        if (!sendCommand(cmd, "AOK")) output = SYS_SET_LED_FAIL;
        
        // disconnect client after 10 seconds idle
        sprintf(cmd, "set comm idle 300\r");
        if (!sendCommand(cmd,"AOK")) output =  COMM_SET_TIME_FAIL;
        
        // open string set to none, default is *OPEN*
        sprintf(cmd, "set comm open *OP\r");
        if (!sendCommand(cmd,"AOK")) output =  COMM_SET_TIME_FAIL;
        
        // no string sent to the tcp client
        if (!sendCommand("set c r 0\r", "AOK")) output = TCP_STRING_SET_FAIL;
        
        // tcp protocol
        if (!sendCommand("set i p 2\r", "AOK")) output = TCP_PROTOCOL_SET_FAIL;
        
        // tcp retry
        if (!sendCommand("set i f 0x7\r", "AOK")) output = TCP_RETRY_SET_FAIL;
            
        // set dns server
        //if (!sendCommand("set d n rn.microchip.com\r", "AOK"))
        //    continue;

        //dhcp
        sprintf(cmd, "set ip dhcp %d\r", (wifi_state.dhcp) ? 1 : 0);
        if (!sendCommand(cmd, "AOK")) output = TCP_DHCP_SET_FAIL;
        
        //auth
        sprintf(cmd, "set wlan auth %d\r", wifi_state.security);
        if (!sendCommand(cmd, "AOK")) output = WLAN_AUTH_SET_FAIL;
        //    continue;

        // if no dhcp, set ip, netmask and gateway
        if (!wifi_state.dhcp) 
        {
            sprintf(cmd, "set i a %s\r\n", ip_str);
            sendCommand(cmd, "AOK");
        
            sprintf(cmd, "set i n %s\r", netmask_str);
            sendCommand(cmd, "AOK");
            
            sprintf(cmd, "set i g %s\r", gateway_str);
            sendCommand(cmd, "AOK");
        }

        // no auto join to start... set back to autojoin at the end
        sprintf(cmd, "set wlan join 1\r");
        /*if (!*/sendCommand(cmd, "AOK"); //) output = WLAN_JOIN_SET_FAIL;

        //join the network (10s timeout)
//        if (state.dhcp) 
//        {
            if (!sendCommand("join\r", "DHCP=ON", NULL, 10000)) output = JOIN_FAIL;
//        }
//        else if (sendCommand("join\r", "Associated", NULL, 10000)) output = JOIN_FAIL;


        // config the TCP port to listen
        sprintf(cmd, "set ip localport 2001\r\n");
        sendCommand(cmd, "AOK");

        sprintf(cmd, "set ip flags 2\r\n");
        sendCommand(cmd, "AOK");

        sprintf(cmd, "set ip protocol 1\r\n");
        sendCommand(cmd, "AOK");

        if (!sendCommand("save\r", "Stor")) output = SAVE_FAIL;
        cmd_mode_set(false);
        wifi_state.associated = true;
        wait(0.2);
        serial_flush();
        wait(0.2);
        
    return output;
}

char * WiflyModule::getStringSecurity()
{
    switch(wifi_state.security) {
        case SEC_NONE:
            return "NONE";
        case SEC_WEP_128:
            return "WEP_128";
        case SEC_WPA:
            return "WPA";
        case SEC_MIXED:
            return "MIXED";
        case SEC_WPA2_PSK:
            return "WPA2_PSK";
    }
    return "UNKNOWN";
}



void WiflyModule::serial_flush()
{
    if (wifi_serial.readable()) wifi_serial.getc();
}



int WiflyModule::init(char * ssid_str, char * phra_str)
{
 //   ssid = ssid_str;
 //   phrase = phra_str;

    wifi_state.dhcp = true;
    hw_reset();
    return 0;
}

int WiflyModule::init(const char* ip, const char* mask, const char* gateway)
{
    wifi_state.dhcp = false;
    strcpy(ip_str, ip);
    strcpy(netmask_str, mask);
    strcpy(gateway_str, gateway);
    hw_reset();

    return 0;
}

char * WiflyModule::getIPAddress()
{
    char * match = 0;
    if (wifi_state.dhcp) 
    {
        if (!sendCommand("get ip address\r", NULL, ip_str)) return NULL;
        cmd_mode_set(false);
        match = strstr(ip_str, "<");
        
        if (match != NULL) 
        {
            *match = '\0';
        }
        if (strlen(ip_str) < 6) 
        {
            match = strstr(ip_str, ">");
            if (match != NULL) {
                int len = strlen(match + 1);
                memcpy(ip_str, match + 1, len);
            }
        }
    }
    return ip_str;
}

bool WiflyModule::sendCommand(const char * cmd, const char * ack, char * res, int timeout)
{
    if (!wifi_state.cmd_mode) 
    {
        cmd_mode_set(true);
    }
    if (send(cmd, strlen(cmd), ack, res, timeout) == -1) 
    {
        cmd_mode_set(false);
        return false;
    }
    return true;    
}

bool WiflyModule::cmd_mode_set(bool cmd_state)
{
    // enter cmd mode
    if (cmd_state)
    {
        // if already in cmd mode, return
        if (wifi_state.cmd_mode) return true;
    
        // if cant enter coomand mode...
        if (send("$$$", 3, "CMD") == -1) 
        {
            cmd_mode_set(false);
            return false;
        }
        wifi_state.cmd_mode = true;
        return true;
    }
    // exit cmd mode
    else     
    {
        // if already out of cmd mode, return true
        if (!wifi_state.cmd_mode) return true;
        
        // if cant exit cmd mode...
        if (!sendCommand("exit\r", "EXIT")) return false;
        wifi_state.cmd_mode = false;
        return true;
    }
}

bool WiflyModule::disconnect_tcp()
{
    // if already disconnected, return
    if (!wifi_state.associated) return true;
    if (!sendCommand("leave\r", "DeAuth")) return false;
    cmd_mode_set(false);

    wifi_state.associated = false;
    return true;

}

bool WiflyModule::is_connected()
{
    return (tcp_status.read() ==  1) ? true : false;
}


void WiflyModule::hw_reset()
{
    reset_pin = 0;
    wait(1);
    reset_pin = 1;
    wait(0.2);
}

bool WiflyModule::reboot_module()
{
    // if already in cmd mode, return
    if (wifi_state.cmd_mode) 
    {
        if (!sendCommand("reboot\r")) return false;
        wait(0.3);
    }
    wifi_state.cmd_mode = false;
    return true;
}

bool WiflyModule::printf(const char * str)
{
    // dont send if the wifi is in cmd mode
    if (wifi_state.cmd_mode) return false;
    // if not in cmd mode, send the string
    else wifi_serial.printf("%s", str);
    return true;
}

bool WiflyModule::putc(char c)
{
    // dont send if the wifi is in cmd mode
    if (wifi_state.cmd_mode) 
    {
        cmd_mode_set(false);
    }
    // if not in cmd mode, send the string
    else wifi_serial.putc(c);
    return true;
}

int WiflyModule::get_string(char * str)
{
    char n = 0;
    
    while (wifi_serial.readable()) 
    {
        str[n] = wifi_serial.getc();
        n++;
    }
    return n;
}

char WiflyModule::getc()
{
    char c = 0;
    if (wifi_serial.readable()) c = wifi_serial.getc();
    return c;
}

int WiflyModule::send(const char * str, int len, const char * ACK, char * res, int timeout)
{
    char read;
    size_t found = string::npos;
    string checking;
    Timer tmr;
    int result = 0;

    if (!ACK || !strcmp(ACK, "NO")) 
    {
        wifi_serial.printf(str);
    } 
    else 
    {
        tmr.start();
        wifi_serial.printf(str);
        
        while (1) {
            if (tmr.read_ms() > timeout) return -1;
            else if (wifi_serial.readable()) 
            {
                read = wifi_serial.getc();
                if ( read != '\r' && read != '\n') 
                {
                    checking += read;
                    found = checking.find(ACK);
                    if (found != string::npos) 
                    {
                        wait(0.01);
                        break;
                    }
                }
                //found carrier return                
                else
                {
                }
            }
        }
        return result;
    }

    //the user wants the result from the command (ACK == NULL, res != NULL)
    if ( res != NULL) 
    {
        int i = 0;
        Timer timeout;
        timeout.start();
        tmr.reset();
        while (1) 
        {
            if (timeout.read() > 2) 
            {
                if (i == 0) 
                {
                    res = NULL;
                    break;
                }
                res[i] = '\0';

                break;
            } 
            else // no timeout
            {
                if (tmr.read_ms() > 300) 
                {
                    res[i] = '\0';

                    break;
                }
                if (wifi_serial.readable()) 
                {
                    tmr.start();
                    read = wifi_serial.getc();

                    // we drop \r and \n
                    if ( read != '\r' && read != '\n') 
                    {
                        res[i++] = read;
                    }
                }
            }
        }
    }
    return result;
}