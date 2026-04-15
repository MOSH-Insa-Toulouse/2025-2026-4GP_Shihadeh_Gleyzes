#include <Wire.h>            
#include <Adafruit_GFX.h>     
#include <Adafruit_SSD1306.h> 
#include <SPI.h>            

// =============================================================================
//                                CONFIGURATION PINS
// =============================================================================

// --- Encodeur Rotatoire ---
#define ENCODER_CLK_PIN 2
#define ENCODER_DT_PIN 4
#define ENCODER_SW_PIN 5

// --- Ecran OLED SSD1306 ---
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

// --- Capteur & Actionneur ---
#define GRAPHITE_SENSOR_PIN A0 
#define POT_CS_PIN 10      

// =============================================================================
//                            CONSTANTES & VARIABLES
// =============================================================================

// --- Lissage des données ---
const int numReadings = 15;   
int readings[numReadings];    
int readIndex = 0;            
long total = 0;               

// --- Carac Circuit ---
const float VCC = 5.0;
const float R2 = 100000.0;
const float R4 = 100000.0;
const float R3 = 10000.0;

// --- Potar ---
const int POT_MAX_POS = 255;   
const long POT_BASE_RESISTANCE = 50000;  
const byte POT_OFFSET = 10000;
const byte POT_ADDR_0 = 0x11; 

// --- Objets ---
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

volatile int encoderPos = 0; 
int lastEncoderPos = 0;       
int menuSelection = 0;        
bool inSubMenu = false;       
unsigned long lastUpdate = 0;

int potPosition = 128;        
float potResValue = 17000.0;  

// =============================================================================
//                                   FONCTIONS 
// =============================================================================


// Interruption de l'encodeur : détecte le sens de rotation
 
void onEncoderInterrupt() {
  if (digitalRead(ENCODER_CLK_PIN) == HIGH) {
    if (digitalRead(ENCODER_DT_PIN) == HIGH) encoderPos++;
    else encoderPos--;
  }
}


//MAJ potar via SPI

void updateDigitalPot(int position) {
  position = constrain(position, 0, 255);
  digitalWrite(POT_CS_PIN, LOW);
  SPI.transfer(POT_ADDR_0);
  SPI.transfer(position);
  digitalWrite(POT_CS_PIN, HIGH);
  
  // Calcul de la résistance réelle équivalente
  potResValue = (POT_BASE_RESISTANCE * position / POT_MAX_POS) + POT_OFFSET;
}


 // Lissage du signal ADC pour stabiliser la mesure (moyenne glissante)

int getSmoothedADC() {
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(GRAPHITE_SENSOR_PIN);
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % numReadings; // Incrémente et boucle
  return total / numReadings;
}

//Calcule Graph Volt
float getGraphiteVoltage(){
  int adcValue = analogRead(GRAPHITE_SENSOR_PIN);
  return adcValue * VCC / 1023.0;
}
//Calcule Graph Res
float getGraphiteRes() {
  int adc = getSmoothedADC();
  float Vadc = adc * VCC / 1023.0;

  if (Vadc < 0.05) return -1.0; // Seuil 
  
  return R2 * (1.0 + R4 / potResValue) * (VCC / Vadc) - R2 - R3;
}

// Routine de démarrage : Cherche une plage de mesure optimale

void calibration() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 25);
  display.println(F("CALIBRATION..."));
  display.display();

const float targetVoltage = 2.5;
  int bestPos = 0;
  float minDiff = 5.0; 
  for (int pos = 0; pos <= 255; pos += 5) {
    updateDigitalPot(pos);
    delay(40); 
    
    float v = getGraphiteVoltage();
    float diff = abs(v - targetVoltage);
    if (diff < minDiff) {
      minDiff = diff;
      bestPos = pos;
    }
  }
  
  // On applique la meilleure position trouvée
  potPosition = bestPos;
  updateDigitalPot(potPosition);
}
// =============================================================================
//                                  SETUP
// =============================================================================

void setup() {
  Serial.begin(9600);

  // Config Inputs
  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), onEncoderInterrupt, RISING);

  // Config Outputs
  pinMode(POT_CS_PIN, OUTPUT);
  digitalWrite(POT_CS_PIN, HIGH);
  SPI.begin();

  // Init Ecran
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    for(;;); // Bloque si écran non trouvé
  }

  // Init Lissage
  for (int i = 0; i < numReadings; i++) readings[i] = 0;

  calibration();
  display.clearDisplay();
}

// =============================================================================
//                                  BOUCLE PRINCIPALE
// =============================================================================

void loop() {
  
  // 1. GESTION DU BOUTON (Navigation Menu/Sous-menu)
  if (digitalRead(ENCODER_SW_PIN) == LOW) {
    delay(50);
    inSubMenu = !inSubMenu;
    display.clearDisplay(); // Nettoyage lors du changement de page
    while (digitalRead(ENCODER_SW_PIN) == LOW); // Attente relâchement
  }

  // 2. GESTION DE L'ENCODEUR
  if (!inSubMenu) {
    menuSelection = abs(encoderPos / 2) % 2; 
  } 
  else if (menuSelection == 0) { // On est dans le menu Mesure
    if (encoderPos != lastEncoderPos) {
      potPosition = constrain(potPosition + (encoderPos - lastEncoderPos), 0, 255);
      updateDigitalPot(potPosition);
      lastEncoderPos = encoderPos;
    }
  }

  // 3. AFFICHAGE (Frequence : ~6Hz)
  if (millis() - lastUpdate > 150) {
    lastUpdate = millis();
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    if (!inSubMenu) {
      // --- ÉCRAN D'ACCUEIL ---
      display.setTextSize(1);
      display.setCursor(25, 0); display.println(F("PROJET GRAPH'"));
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
      
      display.setCursor(0, 25);
      display.print(menuSelection == 0 ? F("> MESURE ") : F("  CAPTEUR"));
      display.setCursor(0, 45);
      display.print(menuSelection == 1 ? F("> CREDITS") : F("  BINOME"));
    } 
    else {
      // --- SOUS-MENUS ---
      if (menuSelection == 0) { // Page Mesure
        float rGraph = getGraphiteRes();
        
        display.setTextSize(1);
        display.setCursor(0, 0); display.println(F("MESURE EN DIRECT"));
        display.print(F("Gain (Pot): ")); display.print(potResValue, 0); display.println(F(" Ohm"));
         display.setCursor(0, 0); display.println(F("EQUIPE PROJET"));
        display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
        display.setCursor(0, 25); display.println(F("Samy Shihadeh"));
        display.setCursor(0, 40); display.println(F("Jules Gleyzes"));
        display.setCursor(0, 55); display.println(F("GP 4A - 2026"));
      }
    }
    display.display(); 
  }
}  
        display.setCursor(0, 30);
        if (rGraph < 0) {
          display.setTextSize(2); display.print(F("CONNECTER"));
        } else {
          display.setTextSize(2);
          if (rGraph > 1000000.0) { display.print(rGraph/1000000.0, 1); display.print(F(" M")); }
          else if (rGraph > 1000.0) { display.print(rGraph/1000.0, 1); display.print(F(" k")); }
          else { display.print(rGraph, 0); }
          display.setTextSize(1); display.print(F(" Ohm"));
        }
        
        Serial.println(rGraph); 
      } 
      else { // Page Crédits
        display.setTextSize(1);
        display.setCursor(0, 0); display.println(F("EQUIPE PROJET"));
        display.drawLine(0, 12, 128, 12, SSD1306_WHITE);
        display.setCursor(0, 25); display.println(F("Samy Shihadeh"));
        display.setCursor(0, 40); display.println(F("Jules Gleyzes"));
        display.setCursor(0, 55); display.println(F("GP 4A - 2026"));
      }
    }
    display.display(); 
  }
}
