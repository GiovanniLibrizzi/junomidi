
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#define POTS_NUM 16
#define PAGES 3
#define FADER_RANGE 1023
byte MIDI_CHANNEL = 1;

int currentPage = 0;

int oldVals[16] = {0};
int currentFader = -1;
int tolerance = 4;


// Names for the master array's position (not used yet as there's only one page)
enum arr {
  Juno106,
  Env
};

struct param {
  int id;
  int maxRange;
};


// Roland Alpha Juno 2 parameters (their IDs and max range)
struct param LFO_RATE = {0x18, 0x7F};
struct param LFO_DELAY_TIME = {0x19, 0x7F};
struct param DCO_LFO = {0x0B, 0x7F};
struct param DCO_PWM_DEPTH = {0x0E, 0x7F};
struct param DCO_SUB_LVL = {0x07, 3};
struct param VCF_FREQ = {0x10, 0x7F};
struct param VCF_RES = {0x11, 0x7F};
struct param VCF_ENV = {0x13, 0x7F};
struct param VCF_LFO = {0x12, 0x7F};
struct param VCF_KYBD = {0x14, 0x7F};
struct param VCA_LVL = {0x16, 0x7F};
struct param ENV_T1 = {0x1A, 0x7F}; // ENV A
struct param ENV_T2 = {0x1C, 0x7F}; // ENV D
struct param ENV_T3 = {0x1E, 0x7F}; // ENV S
struct param ENV_T4 = {0x20, 0x7f}; // ENV R
struct param CHORUS = {0x0A, 0x01}; // On off chorus
struct param CHORUS_RATE = {0x22, 0x7F};

struct param DCO_ENV_MODE = {0x00, 0x03};
struct param VCF_ENV_MODE = {0x01, 0x03};
struct param VCA_ENV_MODE = {0x02, 0x03};

struct param DCO_WAVE_PULSE = {0x03, 0x03};
struct param DCO_WAVE_SAW = {0x04, 0x05};
struct param DCO_WAVE_SUB = {0x05, 0x05};
struct param DCO_RANGE = {0x06, 0x03};
struct param HPF_CUTOFF = {0x09, 0x03};

struct param ENV_L1 = {0x1B, 0x7F};
struct param ENV_L2 = {0x1D, 0x7F};
struct param ENV_L3 = {0x1F, 0x7F};



// This array handles which params are on which fader
struct param junoParams[2][16] = {{ LFO_RATE, LFO_DELAY_TIME, DCO_LFO, DCO_PWM_DEPTH, DCO_SUB_LVL, VCF_FREQ, VCF_RES, VCF_ENV, VCF_LFO, VCF_KYBD, VCA_LVL, ENV_T1, ENV_T2, ENV_T3, ENV_T4, CHORUS_RATE },  // Juno 106 Main Layout
                                  { LFO_RATE, VCF_LFO, DCO_LFO, DCO_PWM_DEPTH, DCO_SUB_LVL, VCF_FREQ, VCF_RES, VCF_ENV, VCF_LFO, VCF_KYBD, VCA_LVL, ENV_T1, ENV_T2, ENV_T3, ENV_T4 }}; // Placeholder


byte dbtime = 30; // Debounce time

const int faderValue[16] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15}; //Analog in Array


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
}
void setup() {
  //Serial.begin(9600); // For debugging
  Serial.begin(31250); //Start serial at baud rate 31250
  
  InitializeFaders();

  //Serial.println(junoParams[Juno106][2].value);
  //pinMode(faderValue[2], INPUT);  //Set val A2 as INPUT
}


void loop() {
  
  MIDI.read();

  CheckFaderUpdate();
  UpdateParameters();

  //int value = analogRead(faderValue[1]) / 8;
  //Serial.print("\t | Raw Value A2:");
  //Serial.println(analogRead(val[2]));
  //Serial.println(value);

  //if (millis() % 5000 == 0) { // Every 5 seconds?
    SetDefaultParams(); // Sends default parameters constantly
  //}


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
    struct param curParam = junoParams[currentPage][currentFader];
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

  SendSysExJuno(ENV_L1.id, 127);
  SendSysExJuno(ENV_L2.id, 64);
  //SendSysExJuno(ENV_L3.id, 64);
}
