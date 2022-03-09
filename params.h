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
Param DCO_NOISE = {0x08, 0x03, "DCO Noise"};
Param HPF_CUTOFF = {0x09, 0x03, "HPF"};
Param BEND_RANGE = {0x23, 0x0B, "Bend Range"};

Param ENV_L1 = {0x1B, 0x7F, "ENV L1"};
Param ENV_L2 = {0x1D, 0x7F, "ENV L2"};
Param ENV_L3 = {0x1F, 0x7F, "ENV L3"};
