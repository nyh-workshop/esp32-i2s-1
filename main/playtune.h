#ifndef PLAYTUNE_H
#define PLAYTUNE_H

#include "wavetable.h"
#include "envelope.h"

// Number of channels:
#define NUM_OF_CHANNELS 6

#define SAMPLE_RATE 44100
#define BUFFER_LENGTH 1024             // Buffer length = 2 * working buffers. This is for the audio DAC where there are left + right channels.

void generateFModTask(void *pvParameter);
void updateNoteTask(void *pvParameter);
void updateNote(unsigned char& isPlaying, unsigned int& timePlay, unsigned int& timePlayCount, unsigned int& songIndex, float& speed);
void generateSineTask(void *pvParameter);

enum envelopeState_t { stateAttack = 0, stateDecay, stateSustain, stateRelease, stateOff };

struct ADSR {
  float attack;
  float decay;
  float sustain;
  float release;
  unsigned int attackDuration;
  unsigned int decayDuration;
  unsigned int sustainDuration;
  unsigned int releaseDuration;

  unsigned int attackDurationCount;
  unsigned int decayDurationCount;
  unsigned int sustainDurationCount;
  unsigned int releaseDurationCount;

  unsigned int envelope_ptr1;

  float attackDurationSlope = 0.0f;
  float releaseDurationSlope = 0.0f;
  float lastAmplitude = 0.0f;

  enum envelopeState_t envStateChannel;
};

class FMchannel {
public:
  inline void generateFMsample();
  inline int getOutputSample();
  inline void calculateAttackSample();
  inline void calculateDecaySample();
  inline void calculateReleaseSample();
  inline void setTuningWord_c(unsigned int inputTuningWord_c) { tuningWord_c = inputTuningWord_c; }
  inline void setTuningWord_m(unsigned int inputTuningWord_m) { tuningWord_m = inputTuningWord_m; }
  inline void setStateADSR(envelopeState_t inputStateADSR) { adsrChannel.envStateChannel = inputStateADSR; }

  inline envelopeState_t getStateADSR() { return adsrChannel.envStateChannel; }

  inline float getFreqMult_m() { return freq_mult_m; }
  inline void  setFreqMult_m(float inputFreqMult_m) { freq_mult_m = inputFreqMult_m; }

  inline float getFreqMult_c() { return freq_mult_c; }
  inline void  setFreqMult_c(float inputFreqMult_c) { freq_mult_c = inputFreqMult_c; }

  inline float getAmpl_c() { return amplitude_c; }
  inline void  setAmpl_c(float inputAmpl_c) { amplitude_m = inputAmpl_c; }

  inline float getAmpl_m() { return amplitude_m; }
  inline void  setAmpl_m(float inputAmpl_m) { amplitude_m = inputAmpl_m; }

  inline int getOutput() { return output; }
  inline void setOutput(int _output) { output = _output; }

  inline void setModMultiplier(unsigned int input_mm) { mod_multiplier = input_mm; }

  void resetEnvelopePointers();
  void setADSR(float inputAttackS, float inputDecayS, float inputSustainS, float inputReleaseS); // for now, first two only! Can be overloaded in the future!

private:
  ADSR adsrChannel;
  unsigned int accum_c;
  unsigned int tuningWord_c;
  unsigned int accum_m;
  unsigned int tuningWord_m;
  unsigned int mod_multiplier;
  float amplitude_m;
  float amplitude_c;
  float freq_mult_m;
  float freq_mult_c;
  int temp1_m;
  int output;
};

inline void FMchannel::generateFMsample() {
  accum_m += tuningWord_m;
  temp1_m  = wavetable[accum_m >> 20];
  // Modified to take in float for modulator amplitude:
  //ch[i].accum_c += ch[i].tuningWord_c + (int)((float)ch[i].temp1_m * ch[i].amplitude_m);

  accum_c += tuningWord_c + (int)temp1_m * mod_multiplier;
  output = wavetable[accum_c >> 20];
}

inline int FMchannel::getOutputSample() {
  return output;
}


inline void FMchannel::calculateAttackSample() {
  float temp = 0.0f;

  if(adsrChannel.attackDurationCount >= adsrChannel.attackDuration) {
    adsrChannel.attackDurationCount = 0;
    adsrChannel.envStateChannel = stateDecay;
   } else {
    temp = (adsrChannel.attackDurationSlope * adsrChannel.attackDurationCount) * (float)output;
    output = (short)temp;
    adsrChannel.attackDurationCount++;
  }
}

inline void FMchannel::calculateDecaySample() {

  if (adsrChannel.decayDurationCount >= adsrChannel.decayDuration) {
    adsrChannel.decayDurationCount = 0;

    if (adsrChannel.envelope_ptr1 >= sizeof(envelope)-1)
      //envelope_ptr1 = sizeof(envelope)-1;
      adsrChannel.envelope_ptr1 = (sizeof(envelope)-1);
    else {

      adsrChannel.envelope_ptr1++;
    }

  } else adsrChannel.decayDurationCount++;

  adsrChannel.lastAmplitude = (float)(envelope[adsrChannel.envelope_ptr1]) / float(sizeof(envelope));
  output = (output * (long)(envelope[adsrChannel.envelope_ptr1])) / sizeof(envelope);

}

inline void FMchannel::calculateReleaseSample() {
  float temp = 0.0f;
  if(adsrChannel.releaseDurationCount >= adsrChannel.releaseDuration) {
    adsrChannel.releaseDurationCount = 0;
    adsrChannel.envStateChannel = stateOff;
   } else {
    temp = ( (adsrChannel.lastAmplitude * adsrChannel.releaseDurationSlope * adsrChannel.releaseDurationCount) + adsrChannel.lastAmplitude ) * (float)output;
    output = (short)temp;
    adsrChannel.releaseDurationCount++;
  }
}

#endif
