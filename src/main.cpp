/*
 *  35-button-led.ino
 *      Web server that controls a LED in it with ON and OFF
 */

/*
 *  Includes
 */

#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#define HEADING "<body><h1>ESP32 Web Server</h1>"
#else
#include <ESP8266WiFi.h>
#define HEADING "<body><h1>ESP8266 Web Server</h1>"
#endif
//#include <WebServer.h>

#include "wifi_ruts.h"

/*
 *  Objects invocation
 */

WiFiServer server(80);
WiFiClient client;

/*
 *   Variables
 */

String header; // Variable to store the HTTP request
String output_led1_state = "off";

/*
 *  Definitions included in platformio.ini
 *      LED
 *      SERIAL_BAUD
 *      MY_SSID
 *      MY_PASS
 */

//  Other definitions

#define LED_OFF LOW
#define LED_ON !LED_OFF

/*
 *  Static functions
 */

/*
 *  http_header_ok
 *      response ok to client
 */

static void
http_header_ok(void)
{
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
}

/*
 *  take_action
 *      takes action based in GET header
 *      Turn LED on or off
 */

static void
take_action(void)
{
    // turns the GPIOs on and off
    if (header.indexOf("GET /LED1/on") >= 0)
    {
        Serial.println("GPIO LED1 on");
        output_led1_state = "on";
        digitalWrite(LED, LED_ON);
    }
    else if (header.indexOf("GET /LED1/off") >= 0)
    {
        Serial.println("GPIO LED1 off");
        output_led1_state = "off";
        digitalWrite(LED, LED_OFF);
    }
}

/*
 *  display_html_page:
 */

static void
display_html_page(void)
{
    client.println("<!DOCTYPE html><html>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");

    // CSS to style the on/off buttons
    // Feel free to change the background-color and font-size attributes to fit your preferences
    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 {background-color: #555555;}</style></head>");

    // Web Page Heading
    client.println(HEADING);

    // Display current state, and ON/OFF buttons for GPIO LED1
    client.println("<p>GPIO internal - State " + output_led1_state + "</p>");
    // If the output_led1_state is off, it displays the ON button

    if (output_led1_state == "off")
        client.println("<p><a href=\"/LED1/on\"><button class=\"button\">ON</button></a></p>");
    else
        client.println("<p><a href=\"/LED1/off\"><button class=\"button button2\">OFF</button></a></p>");
    client.println("</body></html>");

    // The HTTP response ends with another blank line
    client.println();
}

/*
 *  message_from_client:
 *      Gets character by character from client
 *      and stores it in 'header String'
 *      After two \n, return true
 *      else if client disconnected, return false
 */

static int
message_from_client(void)
{
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected())
    {
        if (client.available())
        {                           // if there's bytes to read from the client,
            char c = client.read(); // read a byte, then
            Serial.write(c);        // print it out the serial monitor
            header += c;
            if (c == '\n')
            {
                // if the current line is blank, you got two newline characters in a row.
                // that's the end of the client HTTP request, so send a response:
                if (currentLine.length() == 0)
                    return 1;
                else
                    currentLine = ""; // if you got a newline, then clear currentLine
            }
            else if (c != '\r')
                currentLine += c; // add it to the end of the currentLine
        }
    }
    return 0;
}

/*
 *  Public functions
 */

void
setup(void)
{
    Serial.begin(SERIAL_BAUD);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LED_OFF);

    connect_wifi(MY_SSID, MY_PASS);
    server.begin();
}

void
loop(void)
{
    client = server.available(); // Listen for incoming clients

    if (client)
    { // If a new client connects,
        if (message_from_client())
        {
            http_header_ok();
            take_action();
            display_html_page();
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println();
    }
}
