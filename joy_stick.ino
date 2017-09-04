#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CNS, CE
int ClickPin = 2;
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
  Serial.println("write");
}

void loop() {
  // put your main code here, to run repeatedly:
  int x = analogRead(A0) - 512;
  int y = analogRead(A1) - 512;
  
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
  delay(1000);
  
}
