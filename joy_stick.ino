#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>
#define DEBUG true
 
SoftwareSerial esp8266(2,3); // make RX Arduino line is pin 2, make TX Arduino line is pin 3.
                          // This means that you need to connect the TX line from the esp to the Arduino's pin 2
                          // and the RX line from the esp to the Arduino's pin 3


RF24 radio(7, 8); // CNS, CE
int ClickPin = 4;
const byte address[6] = "00001";

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(ClickPin, INPUT);
  Serial.begin(9600);
  Serial.println("Start: ");

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  esp8266.begin(9600); // your esp's baud rate might be different

  Serial.print("123123123123");
  
  sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
  sendCommand("AT+CWMODE=1\r\n",1000,DEBUG); // configure as access point
  sendCommand("AT+CWJAP=\"iGEM_HQ\",\"igemnckutainnan\"\r\n",3000,DEBUG);
  delay(10000);
  sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendCommand("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  sendCommand("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // turn on server on port 80
  
  Serial.println("write");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  
  int x = analogRead(A0) - 512;
  int y = analogRead(A1) - 512;

  // Regulized the zero of controller
  if (x < 30 && x > -30) {
    x = 0;
  }
  if (y < 30 && y > -30) {
    y = 0;
  }
  
  boolean Click = digitalRead(ClickPin);   
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(" y = ");
  Serial.print(y);
  Serial.print(" CL = ");
  Serial.println(Click);


    String tex = String(x, DEC) + "," + String(y, DEC) + "," + String(Click, DEC);
    char text[15];
    tex.toCharArray(text, 15);
    radio.write(&text, sizeof(text));
    
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
  
    Serial.println("read");
    delay(70);
      
    if (radio.available()) {
      char tex[32] = "";
      radio.read(&tex, sizeof(tex));
      Serial.println(tex);
    }

    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
    Serial.println("write");
  

  if (Click) {
    String tex = String(x, DEC) + "," + String(y, DEC) + "," + String(Click, DEC);
    char text[15];
    tex.toCharArray(text, 15);
    radio.write(&text, sizeof(text));
    
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
  
    Serial.println("read");
    delay(50);
      
    if (radio.available()) {
      char tex[32] = "";
      radio.read(&tex, sizeof(tex));
      Serial.println(tex);
    }

    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
    Serial.println("write");  
  }
  else {
    String tex = String(x, DEC) + "," + String(y, DEC) + "," + String(Click, DEC);
    char text[15];
    tex.toCharArray(text, 15);
    radio.write(&text, sizeof(text));
    delay(100);
  }
}
 
/*
* Name: sendData
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    
    int dataSize = command.length();
    char data[dataSize];
    command.toCharArray(data,dataSize);
           
    esp8266.write(data,dataSize); // send the read character to the esp8266
    if(debug)
    {
      Serial.println("\r\n====== HTTP Response From Arduino ======");
      Serial.write(data,dataSize);
      Serial.println("\r\n========================================");
    }
    
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
    
    if(debug)
    {
      Serial.print(response);
    }
    
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
     httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n"; 
     httpHeader += "Content-Length: ";
     httpHeader += content.length();
     httpHeader += "\r\n";
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
   sendCommand(cipSend,1000,DEBUG);
   sendData(data,1000,DEBUG);
}
 
/*
* Name: sendCommand
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendCommand(String command, const int timeout, boolean debug)
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
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
 
