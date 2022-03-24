
#include <MIDI.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include "ArducamSSD1306.h"
#include "Wire.h"
#include "params.h"
MIDI_CREATE_DEFAULT_INSTANCE();

#define POTS_NUM 16
#define PAGES 2
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
int tolerance = 6;

// Counters
unsigned long idleRefresh, lastRefresh, defaultRefresh, eepromRefresh = 0;
const unsigned long REFRESH_INTERVAL = 500, DEFAULT_INTERVAL = 5000, EEPROM_INTERVAL = 20, IDLE_INTERVAL = 900; 

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


int currentPatch[PARAM_AMT];  // Holds the values for the current patch

// This array handles which params are on which fader
Param junoParams[PAGES][POTS_NUM] = {{ LFO_RATE, LFO_DELAY_TIME, DCO_LFO, DCO_PWM_DEPTH, DCO_SUB_LVL, VCF_FREQ, VCF_RES, VCF_ENV, VCF_LFO, PAGE_SEL /*VCF_KYBD*/, VCA_LVL, ENV_T1, ENV_T2, ENV_T3, ENV_T4, CHORUS_RATE },  // Juno 106 Main Layout
                                  { DCO_WAVE_PULSE, DCO_WAVE_SAW, DCO_WAVE_SUB, DCO_RANGE, VCF_KYBD /*HPF_CUTOFF*/, DCO_NOISE, DCO_ENV_MODE, VCF_ENV_MODE, VCA_ENV_MODE, PAGE_SEL, VCF_LFO, ENV_T1, ENV_T2, ENV_T3, ENV_T4, CHORUS_RATE }}; // Placeholder

const int faderValue[POTS_NUM] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}; //Analog in Array


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


  
void loop() {
  
  MIDI.read();

  CheckFaderUpdate();
  
  UpdateParameters();



  if(millis() - defaultRefresh >= DEFAULT_INTERVAL){
    defaultRefresh += DEFAULT_INTERVAL;    
    SetDefaultParams(); // Sends default parameters every 5 seconds
  }

  DrawOLED();


  
  
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

    if (curParam.id == PAGE_SEL.id) {
      currentPage = value;
      return;
    }
    
    SendSysExJuno(curParam.id, value);

  } 
  
}


// Sets the default parameters of Juno 106 (sets pulse and saw wave on by default, I don't have buttons on the interface yet)
void SetDefaultParams() {
//  SendSysExJuno(DCO_ENV_MODE.id, 0);
//  SendSysExJuno(VCF_ENV_MODE.id, 2);
//  SendSysExJuno(VCA_ENV_MODE.id, 0);
//
//  SendSysExJuno(DCO_WAVE_PULSE.id, 3);
//  SendSysExJuno(DCO_WAVE_SAW.id, 1);
//  SendSysExJuno(DCO_WAVE_SUB.id, 0); //maybe #5?
//  SendSysExJuno(DCO_RANGE.id, 2); // 16'?
//  SendSysExJuno(HPF_CUTOFF.id, 0);
//  SendSysExJuno(BEND_RANGE.id, 2);

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
