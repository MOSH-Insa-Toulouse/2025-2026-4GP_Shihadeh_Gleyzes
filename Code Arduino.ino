#include <Wire.h>           //communication pour l'OLED
#include <Adafruit_GFX.h>   //bibliotheque necessaire pour adafruit
#include <Adafruit_SSD1306.h> //Ecran oled (une sorte de pilote)
#include <SPI.h>            //Com SPI pour le potar

//---------------------------- Definitions ----------------------------
//encodeur
#define ENCODER_CLK_PIN 2  //horloge encodeur
#define ENCODER_DT_PIN 4   //direction encodeur
#define ENCODER_SW_PIN 5   // SWITCH (bouton poussoir)
//OLED
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1 //pas de reset dédié
#define OLED_ADDR 0x3C //adresse I2C de l'OLED
//Capteur et potar
#define GRAPHITE_SENSOR_PIN A0
#define POT_CS_PIN 10

//---------------------------- Constantes ----------------------------
//bruit
const int numReadings = 15;      
int readings[numReadings];
int readIndex = 0;        
long total = 0;          
int averageADC = 0;           
//circuit
const float VCC = 5.0;
const float R2 = 100000.0;
const float R4 = 100000.0;
const float R3 = 10000.0;
//potar
const int POT_MAX_POS = 255;  
const int POT_MIN_POS = 0;   
const long POT_BASE_RESISTANCE = 10000;  
const byte POT_OFFSET = 125;
const byte POT_ADDR_0 = 0x11; 

//---------------------------- Objets ----------------------------
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

volatile int encoderPos = 0;
int lastEncoderPos = 0;
int menuSelection = 0;
bool inSubMenu = false;
unsigned long lastUpdate = 0;

int potPosition = 128; 
float potResValue = 17000.0; // Valeur par défaut

//---------------------------- Fonctions ----------------------------
//gestion du potar au demarrage
void calibration() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("CALIBRATION RAPIDE"));
  display.display();

  int targetADC = 5000;
  int low = 0;
  int high = 255;
  int bestPos = 128;
  
  for (int i = 0; i < 8; i++) {
    bestPos = (low + high) / 2;
    updateDigitalPot(10000/bestPos);
    delay(50);
    
    int currentADC = analogRead(GRAPHITE_SENSOR_PIN);

    if (currentADC < targetADC) {
      high = bestPos; 
    } else {
      low = bestPos;
    }
    
    display.fillRect(0, 20, 128, 10, SSD1306_BLACK); // Efface la ligne
    display.setCursor(0, 20);
    display.print(F("Etape ")); display.print(i+1);
    display.print(F(" Val: ")); display.print(currentADC);
    display.display();
  }

  potPosition = bestPos;
  updateDigitalPot(potPosition);
  
  display.setCursor(0, 45);
  display.println(F("CALIBRATION TERMINEE"));
  display.print(F("Pot final: ")); display.println(potPosition);
  display.display();
  delay(1500);
}
void onEncoderInterrupt() {
  if (digitalRead(ENCODER_CLK_PIN) == HIGH) {
    if (digitalRead(ENCODER_DT_PIN) == HIGH) encoderPos++;
    else encoderPos--;
  }
}

void updateDigitalPot(int position) {
  position = constrain(position, 0, 255);
  digitalWrite(POT_CS_PIN, LOW);
  SPI.transfer(POT_ADDR_0);
  SPI.transfer(position);
  digitalWrite(POT_CS_PIN, HIGH);
  potResValue = (POT_BASE_RESISTANCE * (float)position / (float)POT_MAX_POS) + POT_OFFSET;
}

float getGraphiteRes() {
  int adc = getSmoothedADC();
  float Vadc = adc * VCC / 1023.0;

  if (Vadc < 0.01) return -1.0;
  
  float res = R2 * (1.0 + (R4 / potResValue)) * (VCC / Vadc) - R2 - R3;

  return res;
}

int getSmoothedADC() {

  total = total - readings[readIndex];
  readings[readIndex] = analogRead(GRAPHITE_SENSOR_PIN);
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  return total / numReadings;
}

void setup() {
  Serial.begin(9600);


  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), onEncoderInterrupt, RISING);

  pinMode(POT_CS_PIN, OUTPUT);
  digitalWrite(POT_CS_PIN, HIGH);
  SPI.begin();

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
  readings[thisReading] = 0;
}

  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED introuvable"));
    for(;;);
  }


  calibration();

  updateDigitalPot(potPosition);
  display.clearDisplay();
  display.display();
}

void loop() {
  //Serial.println("HELLO BLUETOOTH");  //<-test du bluetooth
  //delay(1000)
  if (digitalRead(ENCODER_SW_PIN) == LOW) {
    delay(200); 
    inSubMenu = !inSubMenu;
    while (digitalRead(ENCODER_SW_PIN) == LOW);
  }

  if (!inSubMenu) {
    menuSelection = abs(encoderPos / 2) % 2; 
  } else if (menuSelection == 0) {
    if (encoderPos != lastEncoderPos) {
      int diff = encoderPos - lastEncoderPos;
      potPosition = constrain(potPosition + diff, 0, 255);
      updateDigitalPot(potPosition);
      lastEncoderPos = encoderPos;
    }
  }

  // 3. AFFICHAGE (Toutes les 150ms)
  if (millis() - lastUpdate > 150) {
    lastUpdate = millis();
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    if (!inSubMenu) {
      // --- MAIN ---
      display.setCursor(30, 0); 
      display.println(F("PROJET GRAPH'"));
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
      
      display.setCursor(0, 25);
      display.print(menuSelection == 0 ? F("> MESURE ") : F("  CAPTEUR"));

      display.setCursor(0, 45);
      display.print(menuSelection == 1 ? F("> CREDITS") : F("BINOME"));
    } 
    else {
      // --- SUB ---
      if (menuSelection == 0) {
        // MENU MESURE
        display.setCursor(0, 0); 
        display.println(F("MESURE EN DIRECT"));
        display.print(F("R_Pot: ")); display.print(potResValue, 0); display.println(F(" Ohm"));
        
        float rGraph = getGraphiteRes();
        display.setCursor(0, 30);
        if (rGraph < 0) {
          display.setTextSize(2); display.print(F("CONNECTER"));
        } else {
          display.setTextSize(2);
          if (rGraph > 1000000.0) { 
            display.print(rGraph/1000000.0, 1); 
            display.print(F(" M")); 
            }
          else if (rGraph > 1000.0) { 
            display.print(rGraph/1000.0, 1); display.print(F(" k")); 
            }
          else { 
            display.print(rGraph, 0);
            }
          display.setTextSize(1); display.print(F(" Ohm"));
        }
        Serial.println(rGraph);
      } 
      else if (menuSelection == 1) {
        // MENU CREDITS
        display.setCursor(0, 0); 
        display.println(F("EQUIPE PROJET"));
        display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
        
        display.setCursor(0, 20); 
        display.println(F("Samy Shihadeh"));
        
        display.setCursor(0, 35); 
        display.println(F("Jules Gleyzes"));
        
        display.setCursor(0, 50); 
        display.println(F("GP 4A - 2026"));
      }
    }
    
    display.display(); // UN SEUL DISPLAY A LA TOUTE FIN
  }
  delay(200);
}