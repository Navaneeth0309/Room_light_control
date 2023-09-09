#include <Arduino.h>
#include <WiFi.h>

void display_HTML(WiFiClient client);
// input from the IR sensor.
const int inp1=32;
const int inp2=34;
// visitor counter
int prev_no_of_persons=0;
int no_of_persons=0;
//state of the sensor
int sensor_state1=0;
int sensor_state2=0;
// to use PWM.
int duty=127;

// Replace with your network credentials
const char* ssid = "Redmi Note 10S";
const char* password = "chennai@01";
//  web server port no: 80
WiFiServer server(80);
//  store the HTTP request
String header;
//   store the current output state of IR and output
String output26State = "off";
String outputIRState = "off";
//  output variables to GPIO pins
const int output26 = 26;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in ms. 
const long timeoutTime = 2000;
//IR inititialization
void IR_Initialization(){
  pinMode(inp1,INPUT);
  pinMode(inp2,INPUT);
}
//How it works with IR
void IR_work(){
  sensor_state1=digitalRead(inp1);
  sensor_state2=digitalRead(inp2);
  //Serial.print(sensor_state1); Serial.print("  ");
  //Serial.print(sensor_state2); Serial.print("  ");
  if(sensor_state1==HIGH && sensor_state2==LOW){
    no_of_persons++;
    delay(1000);
  }
  else if(sensor_state2==HIGH && sensor_state1==LOW){
    if(no_of_persons>0)
      no_of_persons--;
    delay(1000);
  }
  if(no_of_persons>=1 && no_of_persons<=5){
    duty=127;
    analogWrite(output26,duty);
  }
  else if(no_of_persons>5 && no_of_persons<=10){
    duty=192;
    analogWrite(output26,duty);
  }
  else if(no_of_persons<=0){
    duty=0;
    analogWrite(output26,duty);
  }
  else{
    duty=255;
    analogWrite(output26,duty);
  }
  Serial.print(no_of_persons); Serial.print("  ");
  //Serial.println(duty);
  delay(500);
}


void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output26, OUTPUT);
  pinMode(13,OUTPUT);digitalWrite(13,HIGH);
  
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  
  // to inilize the Ir sensor
  IR_Initialization();

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              analogWrite(output26, 255);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("Led 26 off");
              output26State = "off";
              analogWrite(output26, 0);
            } else if (header.indexOf("GET /IR/on") >= 0 && header.indexOf("GET /26/off") < 0 ){
              Serial.println("work with IR");
              outputIRState="on";
            }else if (header.indexOf("GET /IR/off")>= 0){
              Serial.println("stopped working with IR");
              outputIRState="off";
            }
            // Display the HTML web page
            display_HTML(client);
            break;
          } else { // newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got something else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  else{
    if(outputIRState=="on"){
      IR_work();
    }
  }
}

void display_HTML(WiFiClient client)
{
  // Display the HTML web page
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  // CSS to style the on/off buttons.
  client.println("<style>html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".button { background-color: #4CAF50; border: none; color: blue; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
  client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
  client.println("<body><h1>Room Light Control</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
  client.println("<p>GPIO LED - State " + output26State + "</p>");
  // If the output26State is off, it displays the ON button       
  if (output26State=="off") {
    client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
  }

  client.println("<p> IR - State " + outputIRState + "</p>");
  // If the outputIRState is off, it displays the ON button       
  if (outputIRState=="off") {
    client.println("<p><a href=\"/IR/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/IR/off\"><button class=\"button button2\">OFF</button></a></p>");
  } 
  client.println("</body></html>");          
  // The HTTP response ends with another blank line
  client.println();
}