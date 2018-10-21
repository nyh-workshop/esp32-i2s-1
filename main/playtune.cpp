#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "math.h"

#include "playtune.h"
#include "wavetable.h"
#include "songdata.h"
#include "tuningwords.h"
#include "envelope.h"

// Buffer for the I2S module!
unsigned int buffer_pp[BUFFER_LENGTH];

// FM channels here:
FMchannel ch[NUM_OF_CHANNELS];

void FMchannel::resetEnvelopePointers() {
  adsrChannel.envelope_ptr1 = 0;
  adsrChannel.decayDurationCount = 0;
  adsrChannel.attackDurationCount = 0;
  adsrChannel.releaseDurationCount = 0;
}

void FMchannel::setADSR(float inputAttackS, float inputDecayS, float inputSustainS, float inputReleaseS) {

  // these values must not be negative!
  inputAttackS = fabs(inputAttackS);
  inputDecayS = fabs(inputDecayS);
  inputSustainS = fabs(inputSustainS);
  inputReleaseS = fabs(inputReleaseS);

  // Need to do this in a more neat way!
  adsrChannel.attackDuration = (unsigned int) ( (float)inputAttackS / (1.0f/(float)SAMPLE_RATE) );
  adsrChannel.attackDurationSlope = 1.00f / (inputAttackS / (1.0f/(float)SAMPLE_RATE) );

  adsrChannel.decayDuration = (unsigned int) ( ((float)inputDecayS/256.0f) / (1.0f/(float)SAMPLE_RATE) );

  adsrChannel.releaseDuration = (unsigned int) ( (float)inputReleaseS / (1.0f/(float)SAMPLE_RATE) );
  adsrChannel.releaseDurationSlope = -(1.00f / (inputReleaseS / (1.0f/(float)SAMPLE_RATE) ));

}

void generateSineTask(void *pvParameter) {

  unsigned int accum1t = 0;
  unsigned int tuningWord1t = 42949673;

  unsigned int n = 0;
  unsigned int bytesWritten = 0;
  while(1) {
    for (n = 0; n < BUFFER_LENGTH; n++) {
      accum1t += tuningWord1t;
      buffer_pp[n] = wavetable[accum1t >> 20] & 0x0000ffff;
    }

    i2s_write(I2S_NUM_0, buffer_pp, BUFFER_LENGTH*4, &bytesWritten, 100);
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void generateFModTask(void *pvParameter) {
  unsigned int bytesWritten = 0;

  while(1) {
    for (auto n = 0; n < BUFFER_LENGTH; n++) {
      for (auto i = 0; i < NUM_OF_CHANNELS; i++) {
        ch[i].generateFMsample();

        if( ch[i].getStateADSR() == stateAttack ) {
          ch[i].calculateAttackSample();
        }

        if( ch[i].getStateADSR() == stateDecay ) {
          ch[i].calculateDecaySample();
        }

        if( ch[i].getStateADSR() == stateRelease ) {
          ch[i].calculateReleaseSample();
        }

        if( ch[i].getStateADSR() == stateOff ) {
          ch[i].setOutput(0);
        }

      }
      // Temporary: only six channels are manually added!
      buffer_pp[n] = ( (short)ch[0].getOutput() + (short)ch[1].getOutput() + (short)ch[2].getOutput() + (short)ch[3].getOutput() + (short)ch[4].getOutput() + (short)ch[5].getOutput() ) / NUM_OF_CHANNELS;
    }

    i2s_write(I2S_NUM_0 , buffer_pp, BUFFER_LENGTH*4, &bytesWritten, 100);
  }
}

void updateNoteTask(void *pvParameter) {
  unsigned int timePlay = 0;
  unsigned int timePlayCount = 0;
  unsigned char isPlaying = 1;
  unsigned int songIndex = 0;
  float speed = 1.25;

  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 1;

  while(1) {
    if(isPlaying) {
      if(timePlayCount > timePlay) {
        timePlayCount = 0;
        updateNote(isPlaying, timePlay, timePlayCount, songIndex, speed);
      }
      else
      timePlayCount++;

      vTaskDelayUntil( &xLastWakeTime, xFrequency );
    } else {
      // When all is done, wait here!
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void updateNote(unsigned char& isPlaying, unsigned int& timePlay, unsigned int& timePlayCount, unsigned int& songIndex, float& speed) {

  unsigned char cmd = songData1[songIndex];
  unsigned char opcode = cmd & 0xf0;
  unsigned char chan = cmd & 0x0f;

  //printf("cmd: %x\n", cmd);

  if (opcode == 0x90) {
    while (opcode == 0x90) {
      // Play a note here! Reset accumulators and set tuning words from the next note!
      // Also reset decay and attack states!
      ch[chan].setTuningWord_c( (unsigned int) ( (float)tuningWords[songData1[songIndex + 1]] * ch[chan].getFreqMult_m() ) ); // convert and put next notes!
      ch[chan].setTuningWord_m( (unsigned int) ( (float)tuningWords[songData1[songIndex + 1]] * ch[chan].getFreqMult_c() ) ); // same as previous.

      ch[chan].resetEnvelopePointers();
      ch[chan].setStateADSR(stateAttack);

      songIndex += 2;
      cmd = songData1[songIndex];
      opcode = cmd & 0xf0;
      chan = cmd & 0x0f;
    }
  }

  if (opcode == 0x80) {
    while (opcode == 0x80) {
      // Stop note: the note is dampened immediately in order to prevent the click
      // once the next note is played. Not all the clicks in the sound can be removed: this is currently
      // being investigated.
      ch[chan].setStateADSR(stateRelease);
      songIndex += 1;
      cmd = songData1[songIndex];
      opcode = cmd & 0xf0;
      chan = cmd & 0x0f;
      timePlay = 20;
    }
    return;
  }

  if (opcode == 0xf0) { // stop playing score!
    isPlaying = 0;
    return;
  }

  if (opcode == 0xe0) { // start playing from beginning!
    songIndex = 0;
    timePlay = 0;
    timePlayCount = 0;
    return;
  }

  //cmd = songData1[songIndex];
  //opcode = cmd & 0xf0;

  if ( ((opcode & 0x80) == 0) ||  ((opcode & 0x90) == 0) ) {
    timePlay = (unsigned int)( ((songData1[songIndex] << 8) | songData1[songIndex + 1]) * (1/speed) );
    songIndex += 2;
  }
}
