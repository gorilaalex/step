#include <SoftwareSerial.h>


//source code from http://allaboutee.com/2015/01/20/esp8266-android-application-for-arduino-pin-control/

SoftwareSerial esp8266(2,3); //tx from esp is connected to pin 2, and rx from esp is connected to pin 3

void setup() {
  Serial.begin(9600);
  esp8266.begin(9600);
  
  //initWifiAccessPointBroadcast("BelowZero","wader1917");
  initWifiAccessPoint("BelowZero","wader1917");
}

String getOutput(int number){
  if(number == 1) return "HIGH";
  else return "LOW";
}

void loop() {
if(esp8266.available()) // check if the esp is sending a message 
  { 
    if(esp8266.find("+IPD,"))
    {
     delay(1000); // wait for the serial buffer to fill up (read all the serial data)
     // get the connection id so that we can then disconnect
     int connectionId = esp8266.read()-48; // subtract 48 because the read() function returns 
                                           // the ASCII decimal value and 0 (the first decimal number) starts at 48
          
     esp8266.find("c="); // advance cursor to "pin="
          
     int pinNumber = (esp8266.read()-48); // get boolean value (1 open, 0 close)
     
     Serial.write(esp8266.read());
     if(pinNumber==1){
       //open
       Serial.write("pin is 1");
       turnLeft();
     }
     else {
       //close
       Serial.write("pin is 0");
       turnRight();
     }
      // toggle pin    
     
     // build string that is send back to device that is requesting pin toggle
     String content;
     content = "Pin ";
     content += pinNumber;
     content += " is ";
     
     if(pinNumber==0){
       content += analogRead(A0);
       content += " ";
       content += analogRead(A1);
       content += " ";
       content += analogRead(A2);
       content += " ";
       content += analogRead(A3);
       
     }
     
     /*if(digitalRead(pinNumber))
     {
       content += "Open";
     }
     else
     {
       content += " Close";
     }*/
     
     sendCIPData(connectionId,content);
     sendHTTPResponse(connectionId,content);
     
     // make close command
     String closeCommand = "AT+CIPCLOSE="; 
     closeCommand+=connectionId; // append connection id
     closeCommand+="\r\n";
     
     sendCommand(closeCommand,1000); // close connection
    }
  }
}


/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout)
{
    String response = "";
    
    int dataSize = command.length();
    char data[dataSize];
    command.toCharArray(data,dataSize);
           
    esp8266.write(data,dataSize); // send the read character to the esp8266
    
      Serial.println("\r\n====== HTTP Response From Arduino ======");
      Serial.write(data,dataSize);
      Serial.println("\r\n========================================");
    
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
      Serial.print(response);
    
    return response;
}

/*
* Name: sendHTTPResponse
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPResponse(int connectionId, String content)
{
     
     // build HTTP response
     String httpResponse;
     String httpHeader;
     // HTTP Header
     httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\n"; 
     httpHeader += "Content-Length: ";
     httpHeader += content.length();
     httpHeader += "\r\n";
     //httpHeader += " Keep-Alive: timeout=3, max=8\r\n";
     httpHeader +="Connection: close\r\n\r\n";
     httpResponse = httpHeader + content + " "; // There is a bug in this code: the last character of "content" is not sent, I cheated by adding this extra space
     
     sendCIPData(connectionId,httpResponse);
}

/*
* Name: sendCIPDATA
* Description: sends a CIPSEND=<connectionId>,<data> command
*
*/
void sendCIPData(int connectionId, String data)
{
   String cipSend = "AT+CIPSEND=";
   cipSend += connectionId;
   cipSend += ",";
   cipSend +=data.length();
   cipSend +="\r\n";
   sendCommand(cipSend,1000);
   sendData(data,1000);
}

/*
* Name: sendCommand
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendCommand(String command, const int timeout)
{
    String response = "";
           
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
      Serial.print(response);
    
    return response;
}

/*
* Name: initWifiServer
* Description: Function used to init wifi module as a server
*/
void initWifiServer(){
  sendCommand("AT+RST\r\n",2000); // reset module
  sendCommand("AT+CWMODE=3\r\n",1000); // configure as 3 router
  delay(10000);
  sendCommand("AT+CIPMUX=1\r\n",1000); // configure for multiple connections
  sendCommand("AT+CIPSERVER=1,80\r\n",1000); // turn on server on port 80
  //default ip is 192.168.4.1
  Serial.println("Server Ready");
}



void getIpAddress(){
  sendCommand("AT+CIFSR\r\n",1000); // get ip address
}

/*
* Name: initWifiAccessPoint
* Description: Function used to init wifi module as an access point
* Params: ssis - network ssid/name ; password - password of wirelles network, if empty put ""
*/
void initWifiAccessPoint(String ssid,String password){
  sendCommand("AT+RST\r\n",2000); // reset module
  sendCommand("AT+CWMODE=1\r\n",1000); // configure as access point
  sendCommand("AT+CWJAP=\""+ssid +"\",\""+password+"\"\r\n",3000);//connection details
  delay(10000);
  sendCommand("AT+CIFSR\r\n",1000); // get ip address
  sendCommand("AT+CIPMUX=1\r\n",1000); // configure for multiple connections
  sendCommand("AT+CIPSERVER=1,80\r\n",1000); // turn on server on port 80
 
  Serial.println("Access point Ready");
}

void initWifiAccessPointBroadcast(String ssid,String password){
  sendCommand("AT+RST\r\n",2000); // reset module
  sendCommand("AT+CWMODE=3\r\n",1000); // configure as access point
  sendCommand("AT+CWJAP=\""+ssid +"\",\""+password+"\"\r\n",3000);//connection details
  delay(10000);
  sendCommand("AT+CIFSR\r\n",1000); // get ip address
  sendCommand("AT+CIPMUX=1\r\n",1000); // configure for multiple connections
  
  sendCommand("AT+CIPSTART=4,\"UDP\",\"192.168.43.184\",\"3555\",\"8080\",0\r\n",1000); // configure for multiple connections
  
  sendCIPData(0,">mesage");
  
  //sendCommand("+IPD")
  //sendCommand("AT+CIPSERVER=1,80\r\n",1000); // turn on server on port 80
  //sendCommand("AT+CIPSTO=1,9000\r\n",1000);
 
 // sendCommand("AT+CIPSTATUS=?\r\n",1000);
 
  //sendCommand("AT+CIFSR",1000);
 
  //Serial.println("Broadcast Ready");
}

//"open" function
void turnLeft() {
  Serial.println("turnLeft");
}

//"close" function
void turnRight() {
  Serial.println("turnRight");
}


