
#include <MIDI.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include "ArducamSSD1306.h"
#include "Wire.h"
MIDI_CREATE_DEFAULT_INSTANCE();

#define POTS_NUM 16
#define PAGES 3
#define FADER_RANGE 1023
#define PARAM_AMT 36
const byte MIDI_CHANNEL = 1;
const byte dbtime = 10; // Debounce time


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
ArducamSSD1306 display(16);

int currentPage = 0;

int oldVals[POTS_NUM] = {0};
int currentFader = -1;
int faderVal = -1; // Will never be -1 once currentFader is greater than -1
int tolerance = 5;

// Counters
unsigned long idleRefresh, lastRefresh, defaultRefresh, eepromRefresh = 0;
const unsigned long REFRESH_INTERVAL = 350, DEFAULT_INTERVAL = 5000, EEPROM_INTERVAL = 20, IDLE_INTERVAL = 900; 

String EepromTxt;

// Names for the master array's position (not used yet as there's only one page)
enum arr {
  Juno106,
  Env
};

enum Scr {
  idle,
  working,
  eeprom
};

Scr screenState = idle;

typedef struct param {
  int id;
  int maxRange;
  String name;
} Param;


// Roland Alpha Juno 2 parameters (their IDs and max range)
Param LFO_RATE = {0x18, 0x7F, "LFO Rate"};
Param LFO_DELAY_TIME = {0x19, 0x7F, "LFO Delay"};
Param DCO_LFO = {0x0B, 0x7F, "DCO LFO"};
Param DCO_PWM_DEPTH = {0x0E, 0x7F, "DCO PWM"};
Param DCO_SUB_LVL = {0x07, 3, "Sub Level"};
Param VCF_FREQ = {0x10, 0x7F, "VCF Freq"};
Param VCF_RES = {0x11, 0x7F, "VCF Res"};
Param VCF_ENV = {0x13, 0x7F, "VCF Env"};
Param VCF_LFO = {0x12, 0x7F, "VCF LFO"};
Param VCF_KYBD = {0x14, 0x7F, "VCF KYBD"};
Param VCA_LVL = {0x16, 0x7F, "VCA Level"};
Param ENV_T1 = {0x1A, 0x7F, "ENV T1"}; // ENV A
Param ENV_T2 = {0x1C, 0x7F, "ENV T2"}; // ENV D
Param ENV_T3 = {0x1E, 0x7F, "ENV T3"}; // ENV S
Param ENV_T4 = {0x20, 0x7f, "ENV T4"}; // ENV R
Param CHORUS = {0x0A, 0x01, "Chorus"}; // On off chorus
Param CHORUS_RATE = {0x22, 0x7F, "Crs Rate"};

Param DCO_ENV_MODE = {0x00, 0x03, "DCO ENV"};
Param VCF_ENV_MODE = {0x01, 0x03, "VCF ENV"};
Param VCA_ENV_MODE = {0x02, 0x03, "VCA ENV"};

Param DCO_WAVE_PULSE = {0x03, 0x03, "DCO Pulse"};
Param DCO_WAVE_SAW = {0x04, 0x05, "DCO Saw"};
Param DCO_WAVE_SUB = {0x05, 0x05, "DCO Sub"};
Param DCO_RANGE = {0x06, 0x03, "DCO Range"};
Param HPF_CUTOFF = {0x09, 0x03, "HPF"};
Param BEND_RANGE = {0x23, 0x0B, "Bend Range"};

Param ENV_L1 = {0x1B, 0x7F, "ENV L1"};
Param ENV_L2 = {0x1D, 0x7F, "ENV L2"};
Param ENV_L3 = {0x1F, 0x7F, "ENV L3"};

int currentPatch[PARAM_AMT];  // Holds the values for the current patch

// This array handles which params are on which fader
Param junoParams[2][POTS_NUM] = {{ LFO_RATE, LFO_DELAY_TIME, DCO_LFO, DCO_PWM_DEPTH, DCO_SUB_LVL, VCF_FREQ, VCF_RES, VCF_ENV, VCF_LFO, VCF_KYBD, VCA_LVL, ENV_T1, ENV_T2, ENV_T3, ENV_T4, CHORUS_RATE },  // Juno 106 Main Layout
                                  { LFO_RATE, VCF_LFO, DCO_LFO, DCO_PWM_DEPTH, DCO_SUB_LVL, VCF_FREQ, VCF_RES, VCF_ENV, VCF_LFO, VCF_KYBD, VCA_LVL, ENV_T1, ENV_T2, ENV_T3, ENV_T4 }}; // Placeholder

const int faderValue[POTS_NUM] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15}; //Analog in Array


typedef struct patch {
  String name;
  int patch[PARAM_AMT];
} Patch;

void SendSysExJuno(int par, int value) {
    Serial.write(0xF0);                     // SysEx start
    Serial.write(0x41);                     // Roland ID#
    Serial.write(0x36);                     // operation code = individual tone parameters
    Serial.write(MIDI_CHANNEL-1);           // Unit # = MIDI basic channel
    Serial.write(0x23);                     // Format type (JU-1, JU-2)
    Serial.write(0x20);                     // Level # = 1 (?)
    Serial.write(0x01);                     // Group # (?)
    Serial.write(par);                      // SysEx parameter
    Serial.write(value);                    // parameter value
    Serial.write(0xF7);                     // SysEx end
    currentPatch[par] = value;
}

void setup() {
  //Serial.begin(9600); // For debugging
  Serial.begin(31250); //Start serial at baud rate 31250
  
  InitializeFaders();

  //Serial.println(junoParams[Juno106][2].value);
  //pinMode(faderValue[2], INPUT);  //Set val A2 as INPUT

  
  // Display
  display.begin(); // Switch OLED
  // Clear the buffer
    //IdleLCD();
  
}


  boolean buttonPressed = false;
  int prevVal = -1;
  boolean buttonPressed2 = false;
  int prevVal2 = -1;
  
void loop() {
  
  MIDI.read();

  CheckFaderUpdate();
  
  UpdateParameters();



  if(millis() - defaultRefresh >= DEFAULT_INTERVAL){
    defaultRefresh += DEFAULT_INTERVAL;    
    SetDefaultParams(); // Sends default parameters every 5 seconds
  }

  DrawOLED();


  // Debug: fader as button press  
  int val = analogRead(faderValue[15]);
  int cst = 1014;

  if (prevVal <= cst && val > cst) {
    buttonPressed = true;
  } else {
    buttonPressed = false;
  }

  prevVal = val;


    // Debug: fader as button press  
  int val2 = analogRead(faderValue[14]);

  if (prevVal2 <= cst && val2 > cst) {
    buttonPressed2 = true;
  } else {
    buttonPressed2 = false;
  }

  prevVal2 = val2;

  if (buttonPressed) {
    LoadParams(0);
  }

  if (buttonPressed2) {
    SaveParams(0);
  }
  
  delay(dbtime);
}



void InitializeFaders() {
  for (int i = 0; i < POTS_NUM; i++) {
    pinMode(faderValue[i], INPUT);
  }
}


// Loops through all faders, checks if one is changing. If it is, set the current fader var to that potentiometer's index
// potentially change it so it appends to a currentFader array that allows multiple faders changing at once
void CheckFaderUpdate() {
  for (int i = 0; i < POTS_NUM; i++) {
  //int i = 0;
    int value = analogRead(faderValue[i]);
    int difference = abs(value - oldVals[i]);

    if (difference >= tolerance) {
      oldVals[i] = value;
      currentFader = i;
      
    } else if (i == currentFader && difference < tolerance) {
      currentFader = -1;
    }

  }
}


// When the faders are in use, update the Juno parameters.
void UpdateParameters() {

  if (currentFader >= 0) {
    faderVal = currentFader;
    Param curParam = junoParams[currentPage][currentFader];
    int faderDivisor = FADER_RANGE / curParam.maxRange;
    int value = (FADER_RANGE - analogRead(faderValue[currentFader])) / faderDivisor;


    // Special chorus fader condition (turns on chorus if value is greater than 0)
    if (curParam.id == CHORUS_RATE.id) {
      if (value > 0) {
        SendSysExJuno(CHORUS.id, 0x01);
      } else {
        SendSysExJuno(CHORUS.id, 0x00);
      }
    }

    // Links the Envelope Sustain time to the envelope sustain level (as the Juno 106 does I think?)
    if (curParam.id == ENV_T3.id) {
      SendSysExJuno(ENV_L3.id, value);
    }
    
    SendSysExJuno(curParam.id, value);

  } 
  
}


// Sets the default parameters of Juno 106 (sets pulse and saw wave on by default, I don't have buttons on the interface yet)
void SetDefaultParams() {
  SendSysExJuno(DCO_ENV_MODE.id, 0);
  SendSysExJuno(VCF_ENV_MODE.id, 2);
  SendSysExJuno(VCA_ENV_MODE.id, 0);

  SendSysExJuno(DCO_WAVE_PULSE.id, 3);
  SendSysExJuno(DCO_WAVE_SAW.id, 1);
  SendSysExJuno(DCO_WAVE_SUB.id, 0); //maybe #5?
  SendSysExJuno(DCO_RANGE.id, 2); // 16'?
  SendSysExJuno(HPF_CUTOFF.id, 0);
  SendSysExJuno(BEND_RANGE.id, 2);

  SendSysExJuno(ENV_L1.id, 127);
  SendSysExJuno(ENV_L2.id, 64);
  //SendSysExJuno(ENV_L3.id, 64);
}

void SaveParams(int slot) {
  EepromTxt = "Patch " + String(slot) + "\nSaved.";
  screenState = eeprom;
  EEPROM.put(sizeof(currentPatch)*slot, currentPatch);
}
void LoadParams(int slot) {
  EepromTxt = "Patch " + String(slot) + "\nLoaded.";
  screenState = eeprom;
  EEPROM.get(sizeof(currentPatch)*slot, currentPatch);
  for (int i = 0; i < PARAM_AMT; i++) {
    SendSysExJuno(i, currentPatch[i]);
  }
}


void DrawOLED() {
  switch (screenState) {
    case idle:
      IdleOLED();
      if (currentFader >= 0) {
        screenState = working;
      }
      break;

    case working:

      if (currentFader >= 0) {
        if(millis() - lastRefresh >= REFRESH_INTERVAL){
          lastRefresh += REFRESH_INTERVAL;
          idleRefresh = 0;
          DrawParamToOLED();
        }
      } else {
          // Idle Transition
          idleRefresh++;
          if (idleRefresh >= IDLE_INTERVAL) {
            screenState = idle;
          }
      }
      
      break;

    case eeprom:
      EepromOLED();
      eepromRefresh++;

      // Transition
      if (eepromRefresh >= EEPROM_INTERVAL) {
        DrawParamToOLED();
        eepromRefresh = 0;
        screenState = working;
      }
      break;

    default:
      IdleOLED();
  }
}

void DrawParamToOLED() {
  if (faderVal >= 0) {


    // Clear the buffer
    Param curParam = junoParams[currentPage][faderVal];
    int curValue = currentPatch[curParam.id];

    display.clearDisplay();
    
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(SCREEN_WIDTH/2-(curParam.name.length()*6),0);
    display.println(curParam.name);
    
    display.setCursor(SCREEN_WIDTH/2-(countDigit(curValue)*6),20); 
    display.println(curValue);
    
    display.drawRect(10, 42, 108, 16, WHITE);
    display.fillRect(10, 42, int(((double)curValue/(double)curParam.maxRange)*108), 16, WHITE);
    
    display.display();
  } 
}

void IdleOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  String j1 = "JunoMidi";
  display.setCursor(SCREEN_WIDTH/2-(j1.length()*6), 16);
  display.println(j1);
  display.setCursor(8, 42);
  display.setTextSize(1);
  display.println("by:");
  display.setCursor(0, 50);
  display.println("Giovanni L.");
  display.display();
}

void EepromOLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(SCREEN_WIDTH/2-(EepromTxt.length()*6)/2, 16);
  display.println(EepromTxt);

  display.display();
}

int countDigit(int num) {
  if (num < 10)   return 1;
  if (num < 100)  return 2;
  if (num < 1000) return 3;
  if (num < 10000)return 4;
}
