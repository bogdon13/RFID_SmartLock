#include <MFRC522.h>
#include <SPI.h>

// Tag loop connections (pullup buttons)
static int LOOP_BTN_1 = 7;
static int LOOP_BTN_2 = 6;

// RFID Pins
static int RFID_RST = 9;
static int RFID_MISO = 12;
static int RFID_MOSI = 11;
static int RFID_SCK = 13;
static int RFID_SDA = 10;

//Buzzer Pin
static int BUZZER = 4;

//LED Pins
static int GREEN_LED = 3;
static int RED_LED = 2;

//MFRC522 setup
MFRC522 mfrc522(RFID_SDA, RFID_RST);

String paired_UID = "";

//Tag status values
boolean disarmed = true;

void setup() {
  //Make Cereal
  Serial.begin(9600);

  //Get on the bus
  SPI.begin();

  //Get your RFID book out for reading
  mfrc522.PCD_Init();

  Serial.println("RFID reader ready for card pairing. Please approximate card...");
  Serial.println();
  
  //Set up pins
  pinMode(LOOP_BTN_1, INPUT_PULLUP);
  pinMode(LOOP_BTN_2, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  //Test Buzzer
  unlock_sound();

  //Test LED's
  digitalWrite(RED_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, HIGH);
  delay(500);
  digitalWrite(RED_LED, LOW);
  delay(500);
  digitalWrite(GREEN_LED, LOW);

  //Wait for RFID to arm
  RFID_setup();

  //Delay further input
  delay(3000);
}

void loop() {
  boolean loop_1 = digitalRead(LOOP_BTN_1) == 0;
  boolean loop_2 = digitalRead(LOOP_BTN_2) == 0;
    
  if(disarmed){
    //Wait for loops to be dis-connected (bag opened)
    while(loop_1 && loop_2){
      loop_1 = digitalRead(LOOP_BTN_1) == 0;
      loop_2 = digitalRead(LOOP_BTN_2) == 0;
      delay(50);  
    }
    
    //Wait for loops to be connected again
    while(!(loop_1 && loop_2)){
      loop_1 = digitalRead(LOOP_BTN_1) == 0;
      loop_2 = digitalRead(LOOP_BTN_2) == 0;
      delay(50);  
    }

    delay(1000); //Give the user time to connect the clip

    //Re-arm
    disarmed = false;
    led_armed_signal();
    
  }else{
    
    while(!(loop_1 && loop_2)){
      loop_1 = digitalRead(LOOP_BTN_1) == 0;
      loop_2 = digitalRead(LOOP_BTN_2) == 0;
  
      Serial.println("TAG REMOVED WITHOUT PERMISSION! Sounding alarm...");
  
      alarm();
    }
  
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      return;
    }
    //Show UID on serial monitor
    Serial.print("UID tag :");
    String content= "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++) 
    {
       Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
       Serial.print(mfrc522.uid.uidByte[i], HEX);
       content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
       content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    if (content.substring(1) == paired_UID) //change here the UID of the card/cards that you want to give access
    {
      // Disarm the alarm
      Serial.println("Authorized access");
      Serial.println();
      disarmed = true;
      digitalWrite(GREEN_LED, HIGH);
      unlock_sound();
    }
    else
    {
      Serial.println(" Access denied");
      alarm();
      delay(3000);
    }
  }
}

void alarm(){
  digitalWrite(RED_LED, HIGH);
  for(int i = 0; i < 10; i++){
    digitalWrite(BUZZER, HIGH);
    delay(10);
    digitalWrite(BUZZER, LOW);
    delay(20);
  }
  digitalWrite(RED_LED, LOW);
}

void unlock_sound(){
  for(int i = 0; i < 10; i++){
    digitalWrite(BUZZER, HIGH);
    delay(8);
    digitalWrite(BUZZER, LOW);
    delay(8);
  }
  for(int i = 0; i < 20; i++){
    digitalWrite(BUZZER, HIGH);
    delay(4);
    digitalWrite(BUZZER, LOW);
    delay(4);
  }
  for(int i = 0; i < 40; i++){
    digitalWrite(BUZZER, HIGH);
    delay(2);
    digitalWrite(BUZZER, LOW);
    delay(2);
  }
}

void led_armed_signal(){
  // Flash Green LED
  for(int i = 0; i < 5; i++){
    digitalWrite(GREEN_LED, HIGH);
    delay(100);
    digitalWrite(GREEN_LED, LOW);
    delay(100);
  }
}

void RFID_setup(){
  boolean linked = false;
  while(!linked){
    digitalWrite(RED_LED, HIGH);
    // Look for new cards
    if (mfrc522.PICC_IsNewCardPresent()) 
    {
      Serial.println("Card is Present. Reading...");
      // Select one of the cards
      if (mfrc522.PICC_ReadCardSerial()) 
      {
        //Show UID on serial monitor
        Serial.print("Pairing to UID :");
        String content= "";
        byte letter;
        for (byte i = 0; i < mfrc522.uid.size; i++) 
        {
           content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
           content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        Serial.println();
        Serial.print("Paired successfully to : ");
    
        content.toUpperCase();
        paired_UID = content.substring(1);
        
        Serial.println(paired_UID);
        linked = true;

        // Arm the alarm
        disarmed = false;
        led_armed_signal();
      }
    }
    delay(250);
    digitalWrite(RED_LED, LOW);
    delay(250);
  }
}
