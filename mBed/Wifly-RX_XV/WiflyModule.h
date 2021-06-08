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
 *
 * @section DESCRIPTION
 *
 * Wifly RN131-C, wifi module
 *
 * Datasheet:
 *
 * http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Wireless/WiFi/WiFly-RN-UM.pdf
 */

#ifndef WIFLY_MODULE_H
#define WIFLY_MODULE_H

#include "mbed.h"

#define DEFAULT_WAIT_RESP_TIMEOUT 500

typedef enum
{
    ALL_CMD_SUCCESS = 0,
    WLAN_JOIN_SET_FAIL = 1,
    WLAN_SSID_SET_FAIL,
    WLAN_SET_PHRASE_FAIL,
    WLAN_AUTH_SET_FAIL,
    UART_MODE_SET_FAIL,
    COMM_SET_TIME_FAIL,
    COMM_SET_SIZE_FAIL,
    SYS_SET_LED_FAIL,
    TCP_STRING_SET_FAIL,
    TCP_PROTOCOL_SET_FAIL,
    TCP_RETRY_SET_FAIL,
    TCP_DHCP_SET_FAIL,
    JOIN_FAIL,
    SAVE_FAIL
} TJoinOutput;

enum T_Security {
    SEC_NONE = 0,
    SEC_WEP_128 = 1,
    SEC_WPA = 2,
    SEC_MIXED = 3,
    SEC_WPA2_PSK = 4
};

enum Protocol {
    UDP = (1 << 0),
    TCP = (1 << 1)
};

class WiflyModule
{

public:
    /*
    * Constructor
    *
    * @param tx mbed pin to use for tx line of Serial interface
    * @param rx mbed pin to use for rx line of Serial interface
    * @param reset reset pin of the wifi module ()
    * @param tcp_status connection status pin of the wifi module (GPIO 6)
    * @param ssid ssid of the network
    * @param phrase WEP or WPA key
    * @param sec Security type (NONE, WEP_128 or WPA)
    */
    WiflyModule(  PinName tx, PinName rx, PinName reset, PinName tcp_status, const char * ssid, const char * phrase, T_Security security);

    /** Initialize the interface with DHCP.
    * Initialize the interface and configure it to use DHCP (no connection at this point).
    * \return 0 on success, a negative number on failure
    */
    int init(char * ssid_str, char * phra_str); //With DHCP
  
    /** Initialize the interface with a static IP address.
    * Initialize the interface and configure it with the following static configuration (no connection at this point).
    * \param ip the IP address to use
    * \param mask the IP address mask
    * \param gateway the gateway to use
    * \return 0 on success, a negative number on failure
    */
    int init(const char* ip, const char* mask, const char* gateway);

    /** Get IP address
    *
    * @ returns ip address
    */
    char* getIPAddress();
  
    /*
    * Connect the wifi module to the ssid contained in the constructor.
    *
    * @return true if connected, false otherwise
    */
    TJoinOutput config();
    
    /*
    * Connect the wifi module to the ssid contained in the constructor.
    *
    * @return true if connected, false otherwise
    */
    bool connect_router();

    /*
    * Disconnect the wifly module from the access point
    *
    * @ returns true if successful
    */
    bool disconnect_tcp();

    /*
    * Open a tcp connection with the specified host on the specified port
    *
    * @param host host (can be either an ip address or a name. If a name is provided, a dns request will be established)
    * @param port port
    * @ returns true if successful
    */
    //bool connect(const char * host, int port);


    /*
    * Set the protocol (UDP or TCP)
    *
    * @param p protocol
    * @ returns true if successful
    */
    //bool setProtocol(Protocol p);

    /*
    * Reset the wifi module
    */
    void hw_reset();
    
    
    /*
    * Set tcp server
    *
    * @param port_n is the TCP port to listen connections
    * @ returns true if successful
    */
    bool set_tcp_server(int port_n);
    
    /*
    * Reboot the wifi module
    */
    bool reboot_module();

    /*
    * Check if a tcp link is active
    *
    * @returns true if successful
    */
    bool is_connected();


    /*
    * Get string from tcp port
    *
    * @param str pointer
    * @returns string size readed from tcp
    */
    int get_string(char * str);

    /*
    * Read a character
    *
    * @return the character read
    */
    char getc();

    /*
    * Flush the buffer
    */
    void serial_flush();

    /*
    * Write a character
    *
    * @param the character which will be written
    */
    bool putc(char c);

    /*
    * Write a string
    *
    * @param constant string to send to the tcp connection.
    */
    bool printf(const char * str);

    /*
    * To enter in command mode (we can configure the module)
    *
    * @return true if successful, false otherwise
    * @param true to enter command mode and false to exit.
    */
    bool cmd_mode_set(bool cmd_state);
    
    /*
    * Send a string to the wifi module by serial port. This function desactivates the user interrupt handler when a character is received to analyze the response from the wifi module.
    * Useful to send a command to the module and wait a response.
    *
    *
    * @param str string to be sent
    * @param len string length
    * @param ACK string which must be acknowledge by the wifi module. If ACK == NULL, no string has to be acknoledged. (default: "NO")
    * @param res this field will contain the response from the wifi module, result of a command sent. This field is available only if ACK = "NO" AND res != NULL (default: NULL)
    *
    * @return true if ACK has been found in the response from the wifi module. False otherwise or if there is no response in 5s.
    */
    int send(const char * str, int len, const char * ACK = NULL, char * res = NULL, int timeout = DEFAULT_WAIT_RESP_TIMEOUT);

    /*
    * Send a command to the wify module. Check if the module is in command mode. If not enter in command mode
    *
    * @param str string to be sent
    * @param ACK string which must be acknowledge by the wifi module. If ACK == NULL, no string has to be acknoledged. (default: "NO")
    * @param res this field will contain the response from the wifi module, result of a command sent. This field is available only if ACK = "NO" AND res != NULL (default: NULL)
    *
    * @returns true if successful
    */
    bool sendCommand(const char * cmd, const char * ack = NULL, char * res = NULL, int timeout = DEFAULT_WAIT_RESP_TIMEOUT);
    
    /*
    * Return true if the module is using dhcp
    *
    * @returns true if the module is using dhcp
    */
    bool isDHCP() {
        return wifi_state.dhcp;
    }

    //bool gethostbyname(const char * host, char * ip);

    //static WiflyModule * getInstance() {
    //    return inst;
    //};

protected:
    
    Serial wifi_serial;
    DigitalOut reset_pin;
    DigitalIn tcp_status;
    char phrase[30];
    char ssid[30];

    char ip_str[16];
    char netmask_str[16];
    char gateway_str[16];
    int channel;

    static WiflyModule * inst;

    typedef struct STATE {
        bool associated;
        bool tcp;
        bool dhcp;
        T_Security security;
        Protocol proto;
        bool cmd_mode;
    } T_State;

    T_State wifi_state;
    char * getStringSecurity();
};

#endif