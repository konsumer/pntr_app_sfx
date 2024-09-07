#ifndef PNTR_APP_SFX_H__
#define PNTR_APP_SFX_H__

#include <stdint.h>

// Apply squareDuty to sawtooth waveform.
#define SAWTOOTH_DUTY

#define PINK_SIZE 5

typedef struct {
  char riff_header[4];  // Contains "RIFF"
  int32_t wav_size;     // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
  char wave_header[4];  // Contains "WAVE"

  // Format Header
  char fmt_header[4];      // Contains "fmt " (includes trailing space)
  int32_t fmt_chunk_size;  // Should be 16 for PCM
  int16_t audio_format;    // Should be 1 for PCM. 3 for IEEE Float
  int16_t num_channels;
  int32_t sample_rate;
  int32_t byte_rate;         // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
  int16_t sample_alignment;  // num_channels * Bytes Per Sample
  int16_t bit_depth;         // Number of bits per sample

  // Data
  char data_header[4];  // Contains "data"
  int32_t data_bytes;   // Number of bytes in data. Number of samples * num_channels * sample byte size
                        // uint8_t bytes[]; // Remainder of wave file is bytes
} RIFF_header;

enum SfxWaveType {
  SFX_SQUARE,
  SFX_SAWTOOTH,
  SFX_SINE,
  SFX_NOISE,
  SFX_TRIANGLE,
  SFX_PINK_NOISE
};

// Sound parameters (96 bytes matching rFXGen WaveParams)
typedef struct SfxParams {
  // Random seed used to generate the wave
  uint32_t randSeed;

  // Wave type (square, sawtooth, sine, noise)
  int waveType;

  // Wave envelope parameters
  float attackTime;
  float sustainTime;
  float sustainPunch;
  float decayTime;

  // Frequency parameters
  float startFrequency;
  float minFrequency;
  float slide;
  float deltaSlide;
  float vibratoDepth;
  float vibratoSpeed;
  // float vibratoPhaseDelay;      // Unused in sfxr code.

  // Tone change parameters
  float changeAmount;
  float changeSpeed;

  // Square wave parameters
  float squareDuty;
  float dutySweep;

  // Repeat parameters
  float repeatSpeed;

  // Phaser parameters
  float phaserOffset;
  float phaserSweep;

  // Filter parameters
  float lpfCutoff;
  float lpfCutoffSweep;
  float lpfResonance;
  float hpfCutoff;
  float hpfCutoffSweep;
} SfxParams;

// There are 8 parameters with -1,1 range:
//      slide, deltaSlide, changeAmount, dutySweep,
//      phaserOffset, phaserSweep, lpfCutoffSweep, hpfCutoffSweep
#define SFX_NEGATIVE_ONE_MASK 0x0025A4C0

enum SfxSampleFormat {
  SFX_U8,   // uint8_t
  SFX_I16,  // int16_t
  SFX_F32   // float
};

typedef struct SfxSynth {
  int sampleFormat;
  int sampleRate;   // Must be 44100 for now
  int maxDuration;  // Length in seconds
  union {
    uint8_t* u8;
    int16_t* i16;
    float* f;
  } samples;                // sampleRate * maxDuration
  float noiseBuffer[32];    // Random values for SFX_NOISE/SFX_PINK_NOISE
  float pinkWhiteValue[5];  // SFX_PINK_NOISE
  float phaserBuffer[1024];
} SfxSynth;

void pntr_app_sfx_reset_params(SfxParams* params);
SfxSynth* pntr_app_sfx_alloc_synth(int format, int sampleRate, int maxDuration);
int pntr_app_sfx_generate_wave(pntr_app* app, SfxSynth*, const SfxParams* params);

// Load/Save functions
bool pntr_app_sfx_load_params(SfxParams* params, const char* fileName);
bool pntr_app_sfx_save_params(SfxParams* params, const char* fileName);

// Parameter generator functions
void pntr_app_sfx_gen_pickup_coin(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_laser_shoot(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_explosion(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_powerup(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_hit_hurt(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_jump(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_blip_select(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_synth(pntr_app* app, SfxParams* sp);
void pntr_app_sfx_gen_randomize(pntr_app* app, SfxParams*, int waveType);
void pntr_app_sfx_mutate(pntr_app* app, SfxParams* params, float range, uint32_t mask);

// load a SfxParams as a pntr_sound
pntr_sound* pntr_app_sfx_sound(pntr_app* app, SfxParams* params);

#endif  // PNTR_APP_SFX_H__

#ifdef PNTR_APP_SFX_IMPLEMENTATION
#ifndef PNTR_APP_SFX_IMPLEMENTATION_ONCE
#define PNTR_APP_SFX_IMPLEMENTATION_ONCE

#ifndef PNTR_POW
#ifdef PNTR_ENABLE_MATH
#include <math.h>
#define PNTR_POW powf
#else  // PNTR_ENABLE_MATH
static float _pntr_pow(float base, float exponent) {
  float result = 1.0f;
  if (exponent >= 0) {
    for (int i = 0; i < exponent; i++) {
      result *= base;
    }
  } else {
    for (int i = 0; i > exponent; i--) {
      result /= base;
    }
  }
  return result;
}
#define PNTR_POW(x, y) _pntr_pow((x), (y))
#endif  // PNTR_ENABLE_MATH
#endif  // PNTR_POW

static int sfx_random(pntr_app* app, int range) {
  return pntr_app_random(app, 0, range);
}

// Return float in the range 0.0 to 1.0 (both inclusive).
static float frnd(pntr_app* app, float range) {
  return pntr_app_random_float(app, 0.0f, range);
}

// Return float in the range -1.0 to 1.0 (both inclusive).
static float rndNP1(pntr_app* app) {
  return pntr_app_random_float(app, 0.0f, 1.0f);
}

// Return -1.0 to 1.0.
static float pinkValue(pntr_app* app, int* pinkI, float* whiteValue) {
  float sum = 0.0;
  int bitsChanged;
  int lastI = *pinkI;
  int i = lastI + 1;
  if (i > 0x1f) {  // Number of set bits matches PINK_SIZE.
    i = 0;
  }
  bitsChanged = lastI ^ i;
  *pinkI = i;

  for (i = 0; i < PINK_SIZE; ++i) {
    if (bitsChanged & (1 << i)) {
      whiteValue[i] = frnd(app, 1.0f);
    }
    sum += whiteValue[i];
  }
  return (sum / PINK_SIZE) * 2.0f - 1.0f;
}

/*
 * Reset sound parameters to a default square wave.
 * The randSeed is set to zero.
 */
void pntr_app_sfx_reset_params(SfxParams* sp) {
  sp->randSeed = 0;
  sp->waveType = SFX_SQUARE;

  // Wave envelope parameters
  sp->attackTime = 0.0f;
  sp->sustainTime = 0.3f;
  sp->sustainPunch = 0.0f;
  sp->decayTime = 0.4f;

  // Frequency parameters
  sp->startFrequency = 0.3f;
  sp->minFrequency = 0.0f;
  sp->slide = 0.0f;
  sp->deltaSlide = 0.0f;
  sp->vibratoDepth = 0.0f;
  sp->vibratoSpeed = 0.0f;
  // sp->vibratoPhaseDelay = 0.0f;

  // Tone change parameters
  sp->changeAmount = 0.0f;
  sp->changeSpeed = 0.0f;

  // Square wave parameters
  sp->squareDuty = 0.0f;
  sp->dutySweep = 0.0f;

  // Repeat parameters
  sp->repeatSpeed = 0.0f;

  // Phaser parameters
  sp->phaserOffset = 0.0f;
  sp->phaserSweep = 0.0f;

  // Filter parameters
  sp->lpfCutoff = 1.0f;
  sp->lpfCutoffSweep = 0.0f;
  sp->lpfResonance = 0.0f;
  sp->hpfCutoff = 0.0f;
  sp->hpfCutoffSweep = 0.0f;
}

/*
 * Allocate a synth structure and sample buffer as a single block of memory.
 * Returns a pointer to an initialized SfxSynth structure which the caller
 * must free().
 *
 * The use of this function is optional, as the user can provide their own
 * SfxSynth struct and buffer.
 */
SfxSynth* pntr_app_sfx_alloc_synth(int format, int sampleRate, int maxDuration) {
  SfxSynth* syn;
  size_t bufLen = sampleRate * maxDuration;

  if (format == SFX_I16) {
    bufLen *= sizeof(int16_t);
  } else if (format == SFX_F32) {
    bufLen *= sizeof(float);
  }

  syn = (SfxSynth*)PNTR_MALLOC(sizeof(SfxSynth) + bufLen);
  if (syn) {
    syn->sampleFormat = format;
    syn->sampleRate = sampleRate;
    syn->maxDuration = maxDuration;
    syn->samples.f = (float*)(syn + 1);
  }
  return syn;
}

/*
 * Synthesize wave data from parameters.
 * A 44100Hz, mono channel wave is generated.
 *
 * Return the number of samples generated.
 */
int pntr_app_sfx_generate_wave(pntr_app* app, SfxSynth* synth, const SfxParams* sp) {
  float* phaserBuffer = synth->phaserBuffer;
  float* noiseBuffer = synth->noiseBuffer;
  int phase = 0;
  double fperiod;
  double fmaxperiod;
  double fslide;
  double fdslide;
  int period;
  float squareDuty;
  float squareSlide;
  int envStage;
  int envTime;
  int envLength[3];
  float envVolume;
  float fphase;
  float fdphase;
  int iphase;
  int ipp;
  float fltp;
  float fltdp;
  float fltw;
  float fltwd;
  float fltdmp;
  float fltphp;
  float flthp;
  float flthpd;
  float vibratoPhase;
  float vibratoSpeed;
  float vibratoAmplitude;
  int repeatTime;
  int repeatLimit;
  int arpeggioTime;
  int arpeggioLimit;
  double arpeggioModulation;
  float minFreq, sslide;
  int pinkI;
  int i, sampleCount;

  // Sanity check some related parameters.
  minFreq = sp->minFrequency;
  if (minFreq > sp->startFrequency)
    minFreq = sp->startFrequency;

  sslide = sp->slide;
  if (sslide < sp->deltaSlide)
    sslide = sp->deltaSlide;

#define RESET_SAMPLE                                                                                                                            \
  fperiod = 100.0 / (sp->startFrequency * sp->startFrequency + 0.001);                                                                          \
  period = (int)fperiod;                                                                                                                        \
  fmaxperiod = 100.0 / (minFreq * minFreq + 0.001);                                                                                             \
  fslide = 1.0 - PNTR_POW(sslide, 3.0) * 0.01;                                                                                                  \
  fdslide = -PNTR_POW(sp->deltaSlide, 3.0) * 0.000001;                                                                                          \
  squareDuty = 0.5f - sp->squareDuty * 0.5f;                                                                                                    \
  squareSlide = -sp->dutySweep * 0.00005f;                                                                                                      \
  arpeggioModulation = (sp->changeAmount >= 0.0f) ? 1.0 - PNTR_POW(sp->changeAmount, 2.0) * 0.9 : 1.0 + PNTR_POW(sp->changeAmount, 2.0) * 10.0; \
  arpeggioTime = 0;                                                                                                                             \
  arpeggioLimit = (sp->changeSpeed == 1.0f) ? 0 : (int)(PNTR_POW(1.0f - sp->changeSpeed, 2.0f) * 20000 + 32);

  RESET_SAMPLE

  // Reset filter
  fltp = fltdp = 0.0f;
  fltw = PNTR_POW(sp->lpfCutoff, 3.0f) * 0.1f;
  fltwd = 1.0f + sp->lpfCutoffSweep * 0.0001f;
  fltdmp = 5.0f / (1.0f + PNTR_POW(sp->lpfResonance, 2.0f) * 20.0f) * (0.01f + fltw);
  if (fltdmp > 0.8f)
    fltdmp = 0.8f;
  fltphp = 0.0f;
  flthp = PNTR_POW(sp->hpfCutoff, 2.0f) * 0.1f;
  flthpd = 1.0f + sp->hpfCutoffSweep * 0.0003f;

  // Reset vibrato
  vibratoPhase = 0.0f;
  vibratoSpeed = PNTR_POW(sp->vibratoSpeed, 2.0f) * 0.01f;
  vibratoAmplitude = sp->vibratoDepth * 0.5f;

  // Reset envelope
  envVolume = 0.0f;
  envStage = envTime = 0;
  envLength[0] = (int)(sp->attackTime * sp->attackTime * 100000.0f);
  envLength[1] = (int)(sp->sustainTime * sp->sustainTime * 100000.0f);
  envLength[2] = (int)(sp->decayTime * sp->decayTime * 100000.0f);

  fphase = PNTR_POW(sp->phaserOffset, 2.0f) * 1020.0f;
  if (sp->phaserOffset < 0.0f)
    fphase = -fphase;

  fdphase = PNTR_POW(sp->phaserSweep, 2.0f) * 1.0f;
  if (sp->phaserSweep < 0.0f)
    fdphase = -fdphase;

  iphase = abs((int)fphase);
  ipp = 0;
  for (i = 0; i < 1024; i++)
    phaserBuffer[i] = 0.0f;

  if (sp->waveType == SFX_PINK_NOISE) {
    pinkI = 0;
    for (i = 0; i < PINK_SIZE; i++)
      synth->pinkWhiteValue[i] = frnd(app, 1.0f);
  }

#define RESET_NOISE                                                   \
  if (sp->waveType == SFX_NOISE) {                                    \
    for (i = 0; i < 32; i++)                                          \
      noiseBuffer[i] = rndNP1(app);                                   \
  } else if (sp->waveType == SFX_PINK_NOISE) {                        \
    for (i = 0; i < 32; i++)                                          \
      noiseBuffer[i] = pinkValue(app, &pinkI, synth->pinkWhiteValue); \
  }

  RESET_NOISE

  repeatTime = 0;
  repeatLimit = (int)(PNTR_POW(1.0f - sp->repeatSpeed, 2.0f) * 20000 + 32);
  if (sp->repeatSpeed == 0.0f)
    repeatLimit = 0;

  // Synthesize samples.
  {
    const float sampleCoefficient = 0.2f;  // Scales sample value to [-1..1]
#if SINGLE_FORMAT == 1
    uint8_t* buffer = synth->samples.u8;
#elif SINGLE_FORMAT == 2
    int16_t* buffer = synth->samples.i16;
#else
    float* buffer = synth->samples.f;
#endif
    float ssample, rfperiod, fp, pp;
    int sampleEnd = synth->sampleRate * synth->maxDuration;
    int si;

    for (sampleCount = 0; sampleCount < sampleEnd; sampleCount++) {
      repeatTime++;
      if (repeatLimit != 0 && repeatTime >= repeatLimit) {
        repeatTime = 0;
        RESET_SAMPLE
      }

      // Frequency envelopes/arpeggios
      arpeggioTime++;

      if ((arpeggioLimit != 0) && (arpeggioTime >= arpeggioLimit)) {
        arpeggioLimit = 0;
        fperiod *= arpeggioModulation;
      }

      fslide += fdslide;
      fperiod *= fslide;

      if (fperiod > fmaxperiod) {
        fperiod = fmaxperiod;
        if (minFreq > 0.0f)
          sampleEnd = sampleCount;  // End generator loop.
      }

      rfperiod = (float)fperiod;

      if (vibratoAmplitude > 0.0f) {
        vibratoPhase += vibratoSpeed;
        rfperiod = (float)(fperiod * (1.0 + PNTR_SINF(vibratoPhase) * vibratoAmplitude));
      }

      period = (int)rfperiod;
      if (period < 8)
        period = 8;

      squareDuty += squareSlide;
      if (squareDuty < 0.0f)
        squareDuty = 0.0f;
      else if (squareDuty > 0.5f)
        squareDuty = 0.5f;

      // Volume envelope
      envTime++;
      if (envTime > envLength[envStage]) {
        envTime = 0;
      next_stage:
        envStage++;
        if (envStage == 3)
          break;  // End generator loop.
        if (envLength[envStage] == 0)
          goto next_stage;
      }

      switch (envStage) {
        case 0:
          envVolume = (float)envTime / envLength[0];
          break;
        case 1:
          envVolume = 1.0f + PNTR_POW(1.0f - (float)envTime / envLength[1], 1.0f) * 2.0f * sp->sustainPunch;
          break;
        case 2:
          envVolume = 1.0f - (float)envTime / envLength[2];
          break;
      }

      // Phaser step
      fphase += fdphase;
      iphase = abs((int)fphase);

      if (iphase > 1023)
        iphase = 1023;

      if (flthpd != 0.0f) {
        flthp *= flthpd;
        if (flthp < 0.00001f)
          flthp = 0.00001f;
        else if (flthp > 0.1f)
          flthp = 0.1f;
      }

      // 8x supersampling
      ssample = 0.0f;
      for (si = 0; si < 8; si++) {
        float sample = 0.0f;
        phase++;

        if (phase >= period) {
          // phase = 0;
          phase %= period;

          RESET_NOISE
        }

        // Base waveform
        fp = (float)phase / period;

#define RAMP(v, x1, x2, y1, y2) (y1 + (y2 - y1) * ((v - x1) / (x2 - x1)))

        switch (sp->waveType) {
          case SFX_SQUARE:
            sample = (fp < squareDuty) ? 0.5f : -0.5f;
            break;
          case SFX_SAWTOOTH:
#ifdef SAWTOOTH_DUTY
            sample = (fp < squareDuty) ? -1.0f + 2.0f * fp / squareDuty : 1.0f - 2.0f * (fp - squareDuty) / (1.0f - squareDuty);
#else
            sample = 1.0f - fp * 2;
#endif
            break;
          case SFX_SINE:
            sample = PNTR_SINF(fp * 2 * PNTR_PI);
            break;
          case SFX_NOISE:
          case SFX_PINK_NOISE:
            sample = noiseBuffer[phase * 32 / period];
            break;
          case SFX_TRIANGLE:
            sample = (fp < 0.5) ? RAMP(fp, 0.0f, 0.5f, -1.0f, 1.0f) : RAMP(fp, 0.5f, 1.0f, 1.0f, -1.0f);
            break;
        }

        // Low-pass filter
        pp = fltp;
        fltw *= fltwd;

        if (fltw < 0.0f)
          fltw = 0.0f;
        else if (fltw > 0.1f)
          fltw = 0.1f;

        if (sp->lpfCutoff != 1.0f) {
          fltdp += (sample - fltp) * fltw;
          fltdp -= fltdp * fltdmp;
        } else {
          fltp = sample;
          fltdp = 0.0f;
        }

        fltp += fltdp;

        // High-pass filter
        fltphp += fltp - pp;
        fltphp -= fltphp * flthp;
        sample = fltphp;

        // Phaser
        phaserBuffer[ipp & 1023] = sample;
        sample += phaserBuffer[(ipp - iphase + 1024) & 1023];
        ipp = (ipp + 1) & 1023;

        // Final accumulation and envelope application
        ssample += sample * envVolume;
      }

      ssample = ssample / 8 * sampleCoefficient;

      // Clamp sample and emit to buffer
      if (ssample > 1.0f)
        ssample = 1.0f;
      else if (ssample < -1.0f)
        ssample = -1.0f;

        // printf("%d %f\n", sampleCount, ssample);
#if SINGLE_FORMAT == 1
      *buffer++ = (uint8_t)(ssample * 127.0f + 128.0f);
#elif SINGLE_FORMAT == 2
      *buffer++ = (int16_t)(ssample * 32767.0f);
#elif SINGLE_FORMAT == 3
      *buffer++ = ssample;
#else
      switch (synth->sampleFormat) {
        case SFX_U8:
          ((uint8_t*)buffer)[sampleCount] = (uint8_t)(ssample * 127.0f + 128.0f);
          break;
        case SFX_I16:
          ((int16_t*)buffer)[sampleCount] = (int16_t)(ssample * 32767.0f);
          break;
        case SFX_F32:
          buffer[sampleCount] = ssample;
          break;
      }
#endif
    }
  }

  return sampleCount;
}

/**
 * Load params from disk
 */
bool pntr_app_sfx_load_params(SfxParams* params, const char* fileName) {
  if (fileName == NULL) {
    pntr_set_error(PNTR_ERROR_INVALID_ARGS);
    return false;
  }

  unsigned int bytesRead;
  unsigned char* fileData = pntr_load_file(fileName, &bytesRead);
  if (fileData == NULL) {
    return false;
  }

  if (fileData[0] != 'r' || fileData[1] != 'F' || fileData[2] != 'X' || fileData[3] != ' ') {
    pntr_set_error(PNTR_ERROR_FAILED_TO_OPEN);
    return false;
  }

  short int version = 0;
  PNTR_MEMCPY(&version, fileData + 4, 2);

  // only 200 is supported
  if (version != 200) {
    pntr_set_error(PNTR_ERROR_FAILED_TO_OPEN);
    return false;
  }

  short int len = 0;
  PNTR_MEMCPY(&len, fileData + 6, 2);

  // only 96 is supported
  if (len != 96) {
    pntr_set_error(PNTR_ERROR_FAILED_TO_OPEN);
    return false;
  }

  PNTR_MEMCPY(params, fileData + 8, 96);
  return true;
}

/**
 * Save params to disk
 */
bool pntr_app_sfx_save_params(SfxParams* params, const char* fileName) {
  if (fileName == NULL) {
    pntr_set_error(PNTR_ERROR_INVALID_ARGS);
    return false;
  }

  unsigned char fileData[104] = {0};

  char* signature = "rFX ";
  short int version = 200;
  short int len = 96;
  PNTR_MEMCPY(&fileData, signature, 4);
  PNTR_MEMCPY(fileData + 4, &version, sizeof(short int));
  PNTR_MEMCPY(fileData + 4 + sizeof(short int), &len, sizeof(short int));
  PNTR_MEMCPY(fileData + 4 + sizeof(short int) + sizeof(short int), &params, sizeof(SfxParams));

  return pntr_save_file(fileName, &fileData, 4 + sizeof(short int) * + sizeof(short int) + sizeof(SfxParams));
}

void pntr_app_sfx_gen_pickup_coin(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  sp->startFrequency = 0.4f + frnd(app, 0.5f);
  sp->attackTime = 0.0f;
  sp->sustainTime = frnd(app, 0.1f);
  sp->decayTime = 0.1f + frnd(app, 0.4f);
  sp->sustainPunch = 0.3f + frnd(app, 0.3f);

  if (sfx_random(app, 2)) {
    sp->changeSpeed = 0.5f + frnd(app, 0.2f);
    sp->changeAmount = 0.2f + frnd(app, 0.4f);
  }
}

void pntr_app_sfx_gen_laser_shoot(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  sp->waveType = sfx_random(app, 3);

  if ((sp->waveType == SFX_SINE) && sfx_random(app, 2)) {
    sp->waveType = sfx_random(app, 2);
  }

  sp->startFrequency = 0.5f + frnd(app, 0.5f);
  sp->minFrequency = sp->startFrequency - 0.2f - frnd(app, 0.6f);

  if (sp->minFrequency < 0.2f) {
    sp->minFrequency = 0.2f;
  }

  sp->slide = -0.15f - frnd(app, 0.2f);

  if (sfx_random(app, 3) == 0) {
    sp->startFrequency = 0.3f + frnd(app, 0.6f);
    sp->minFrequency = frnd(app, 0.1f);
    sp->slide = -0.35f - frnd(app, 0.3f);
  }

  if (sfx_random(app, 2)) {
    sp->squareDuty = frnd(app, 0.5f);
    sp->dutySweep = frnd(app, 0.2f);
  } else {
    sp->squareDuty = 0.4f + frnd(app, 0.5f);
    sp->dutySweep = -frnd(app, 0.7f);
  }

  sp->attackTime = 0.0f;
  sp->sustainTime = 0.1f + frnd(app, 0.2f);
  sp->decayTime = frnd(app, 0.4f);

  if (sfx_random(app, 2)) {
    sp->sustainPunch = frnd(app, 0.3f);
  }

  if (sfx_random(app, 3) == 0) {
    sp->phaserOffset = frnd(app, 0.2f);
    sp->phaserSweep = -frnd(app, 0.2f);
  }

  if (sfx_random(app, 2)) {
    sp->hpfCutoff = frnd(app, 0.3f);
  }
}

void pntr_app_sfx_gen_explosion(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  sp->waveType = SFX_NOISE;

  if (sfx_random(app, 2)) {
    sp->startFrequency = 0.1f + frnd(app, 0.4f);
    sp->slide = -0.1f + frnd(app, 0.4f);
  } else {
    sp->startFrequency = 0.2f + frnd(app, 0.7f);
    sp->slide = -0.2f - frnd(app, 0.2f);
  }

  sp->startFrequency *= sp->startFrequency;

  if (sfx_random(app, 5) == 0) {
    sp->slide = 0.0f;
  }

  if (sfx_random(app, 3) == 0) {
    sp->repeatSpeed = 0.3f + frnd(app, 0.5f);
  }

  sp->attackTime = 0.0f;
  sp->sustainTime = 0.1f + frnd(app, 0.3f);
  sp->decayTime = frnd(app, 0.5f);

  if (sfx_random(app, 2) == 0) {
    sp->phaserOffset = -0.3f + frnd(app, 0.9f);
    sp->phaserSweep = -frnd(app, 0.3f);
  }

  sp->sustainPunch = 0.2f + frnd(app, 0.6f);

  if (sfx_random(app, 2)) {
    sp->vibratoDepth = frnd(app, 0.7f);
    sp->vibratoSpeed = frnd(app, 0.6f);
  }

  if (sfx_random(app, 3) == 0) {
    sp->changeSpeed = 0.6f + frnd(app, 0.3f);
    sp->changeAmount = 0.8f - frnd(app, 1.6f);
  }
}

void pntr_app_sfx_gen_powerup(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  if (sfx_random(app, 2)) {
    sp->waveType = SFX_SAWTOOTH;
#ifdef SAWTOOTH_DUTY
    sp->squareDuty = 1.0f;
#endif
  } else {
    sp->squareDuty = frnd(app, 0.6f);
  }

  if (sfx_random(app, 2)) {
    sp->startFrequency = 0.2f + frnd(app, 0.3f);
    sp->slide = 0.1f + frnd(app, 0.4f);
    sp->repeatSpeed = 0.4f + frnd(app, 0.4f);
  } else {
    sp->startFrequency = 0.2f + frnd(app, 0.3f);
    sp->slide = 0.05f + frnd(app, 0.2f);

    if (sfx_random(app, 2)) {
      sp->vibratoDepth = frnd(app, 0.7f);
      sp->vibratoSpeed = frnd(app, 0.6f);
    }
  }

  sp->attackTime = 0.0f;
  sp->sustainTime = frnd(app, 0.4f);
  sp->decayTime = 0.1f + frnd(app, 0.4f);
}

void pntr_app_sfx_gen_hit_hurt(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  sp->waveType = sfx_random(app, 3);
  if (sp->waveType == SFX_SINE) {
    sp->waveType = SFX_NOISE;
  } else if (sp->waveType == SFX_SQUARE) {
    sp->squareDuty = frnd(app, 0.6f);
  }
#ifdef SAWTOOTH_DUTY
  else if (sp->waveType == SFX_SAWTOOTH) {
    sp->squareDuty = 1.0f;
  }
#endif

  sp->startFrequency = 0.2f + frnd(app, 0.6f);
  sp->slide = -0.3f - frnd(app, 0.4f);
  sp->attackTime = 0.0f;
  sp->sustainTime = frnd(app, 0.1f);
  sp->decayTime = 0.1f + frnd(app, 0.2f);

  if (sfx_random(app, 2)) {
    sp->hpfCutoff = frnd(app, 0.3f);
  }
}

void pntr_app_sfx_gen_jump(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  sp->waveType = SFX_SQUARE;
  sp->squareDuty = frnd(app, 0.6f);
  sp->startFrequency = 0.3f + frnd(app, 0.3f);
  sp->slide = 0.1f + frnd(app, 0.2f);
  sp->attackTime = 0.0f;
  sp->sustainTime = 0.1f + frnd(app, 0.3f);
  sp->decayTime = 0.1f + frnd(app, 0.2f);

  if (sfx_random(app, 2)) {
    sp->hpfCutoff = frnd(app, 0.3f);
  }

  if (sfx_random(app, 2)) {
    sp->lpfCutoff = 1.0f - frnd(app, 0.6f);
  }
}

void pntr_app_sfx_gen_blip_select(pntr_app* app, SfxParams* sp) {
  pntr_app_sfx_reset_params(sp);

  sp->waveType = sfx_random(app, 2);
  if (sp->waveType == SFX_SQUARE) {
    sp->squareDuty = frnd(app, 0.6f);
  }
#ifdef SAWTOOTH_DUTY
  else {
    sp->squareDuty = 1.0f;
  }
#endif
  sp->startFrequency = 0.2f + frnd(app, 0.4f);
  sp->attackTime = 0.0f;
  sp->sustainTime = 0.1f + frnd(app, 0.1f);
  sp->decayTime = frnd(app, 0.2f);
  sp->hpfCutoff = 0.1f;
}

void pntr_app_sfx_gen_synth(pntr_app* app, SfxParams* sp) {
  static const float synthFreq[3] = {
      0.27231713609, 0.19255692561, 0.13615778746};
  static const float arpeggioMod[7] = {
      0, 0, 0, 0, -0.3162, 0.7454, 0.7454};

  pntr_app_sfx_reset_params(sp);

  sp->waveType = sfx_random(app, 2);
  sp->startFrequency = synthFreq[sfx_random(app, 3)];
  sp->attackTime = sfx_random(app, 5) > 3 ? frnd(app, 0.5) : 0;
  sp->sustainTime = frnd(app, 1.0f);
  sp->sustainPunch = frnd(app, 1.0f);
  sp->decayTime = frnd(app, 0.9f) + 0.1f;
  sp->changeAmount = arpeggioMod[sfx_random(app, 7)];
  sp->changeSpeed = frnd(app, 0.5f) + 0.4f;
  sp->squareDuty = frnd(app, 1.0f);
  sp->dutySweep = (sfx_random(app, 3) == 2) ? frnd(app, 1.0f) : 0.0f;
  sp->lpfCutoff = (sfx_random(app, 2) == 1) ? 1.0f : 0.9f * frnd(app, 1.0f) * frnd(app, 1.0f) + 0.1f;
  sp->lpfCutoffSweep = rndNP1(app);
  sp->lpfResonance = frnd(app, 1.0f);
  sp->hpfCutoff = (sfx_random(app, 4) == 3) ? frnd(app, 1.0f) : 0.0f;
  sp->hpfCutoffSweep = (sfx_random(app, 4) == 3) ? frnd(app, 1.0f) : 0.0f;
}

void pntr_app_sfx_gen_randomize(pntr_app* app, SfxParams* sp, int waveType) {
  pntr_app_sfx_reset_params(sp);

  sp->waveType = waveType;

  sp->startFrequency = PNTR_POW(rndNP1(app), 2.0f);

  if (sfx_random(app, 1)) {
    sp->startFrequency = PNTR_POW(rndNP1(app), 3.0f) + 0.5f;
  }

  sp->minFrequency = 0.0f;
  sp->slide = PNTR_POW(rndNP1(app), 5.0f);

  if ((sp->startFrequency > 0.7f) && (sp->slide > 0.2f)) {
    sp->slide = -sp->slide;
  }
  if ((sp->startFrequency < 0.2f) && (sp->slide < -0.05f)) {
    sp->slide = -sp->slide;
  }

  sp->deltaSlide = PNTR_POW(rndNP1(app), 3.0f);
  sp->squareDuty = rndNP1(app);
  sp->dutySweep = PNTR_POW(rndNP1(app), 3.0f);
  sp->vibratoDepth = PNTR_POW(rndNP1(app), 3.0f);
  sp->vibratoSpeed = rndNP1(app);
  // sp->vibratoPhaseDelay = rndNP1(app);
  sp->attackTime = PNTR_POW(rndNP1(app), 3.0f);
  sp->sustainTime = PNTR_POW(rndNP1(app), 2.0f);
  sp->decayTime = rndNP1(app);
  sp->sustainPunch = PNTR_POW(frnd(app, 0.8f), 2.0f);

  if (sp->attackTime + sp->sustainTime + sp->decayTime < 0.2f) {
    sp->sustainTime += 0.2f + frnd(app, 0.3f);
    sp->decayTime += 0.2f + frnd(app, 0.3f);
  }

  sp->lpfResonance = rndNP1(app);
  sp->lpfCutoff = 1.0f - PNTR_POW(frnd(app, 1.0f), 3.0f);
  sp->lpfCutoffSweep = PNTR_POW(rndNP1(app), 3.0f);

  if (sp->lpfCutoff < 0.1f && sp->lpfCutoffSweep < -0.05f) {
    sp->lpfCutoffSweep = -sp->lpfCutoffSweep;
  }

  sp->hpfCutoff = PNTR_POW(frnd(app, 1.0f), 5.0f);
  sp->hpfCutoffSweep = PNTR_POW(rndNP1(app), 5.0f);
  sp->phaserOffset = PNTR_POW(rndNP1(app), 3.0f);
  sp->phaserSweep = PNTR_POW(rndNP1(app), 3.0f);
  sp->repeatSpeed = rndNP1(app);
  sp->changeSpeed = rndNP1(app);
  sp->changeAmount = rndNP1(app);
}

void pntr_app_sfx_mutate(pntr_app* app, SfxParams* sp, float range, uint32_t mask) {
  float* valPtr = &sp->attackTime;
  float half = range * 0.5f;
  float val, low;
  uint32_t rmod, bit;
  int i;

  rmod = 1 + sfx_random(app, 0xFFFFFF);
  for (i = 0; i < 22; ++i) {
    bit = 1 << i;
    if ((rmod & bit) & mask) {
      low = (SFX_NEGATIVE_ONE_MASK & bit) ? -1.0f : 0.0f;
      val = *valPtr + frnd(app, range) - half;
      if (val > 1.0f)
        val = 1.0f;
      else if (val < low)
        val = low;
      *valPtr = val;
    }
    ++valPtr;
  }
}

pntr_sound* pntr_app_sfx_sound(pntr_app* app, SfxParams* params) {
  RIFF_header wav_header = {
      .riff_header = "RIFF",
      .wave_header = "WAVE",
      .fmt_header = "fmt ",
      .data_header = "data",
      .fmt_chunk_size = 16,
      .audio_format = 1,
      .num_channels = 1,
      .sample_rate = 44100,
      .byte_rate = 44100,
      .sample_alignment = 1,
      .bit_depth = 8,

      .wav_size = 0,
      .data_bytes = 0};

  SfxSynth* synth = pntr_app_sfx_alloc_synth(SFX_U8, 44100, 10);
  int sampleCount = pntr_app_sfx_generate_wave(app, synth, params);

  wav_header.wav_size = sampleCount;
  wav_header.data_bytes = sampleCount;

  unsigned char* w = PNTR_MALLOC(sampleCount + sizeof(wav_header));
  PNTR_MEMCPY(w, &wav_header, sizeof(wav_header));
  PNTR_MEMCPY((void*)((int64_t)w + sizeof(wav_header)), synth->samples.u8, sampleCount);

  pntr_sound* s = pntr_load_sound_from_memory(PNTR_APP_SOUND_TYPE_WAV, w, sampleCount + sizeof(wav_header));
  if (synth != NULL) {
    PNTR_FREE(synth);
  }
  return s;
}

#endif  // PNTR_APP_SFX_IMPLEMENTATION_ONCE
#endif  // PNTR_APP_SFX_IMPLEMENTATION
