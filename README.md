This will let you load SFX made with [rfxgen](https://raylibtech.itch.io/rfxgen) in [pntr_app](https://github.com/robloach/pntr_app).

It allows you add & generate very small sound-effects to your game, with a [single header](pntr_app.sfx.h).


Basic usage looks like this:

```c
#define PNTR_APP_SFX_IMPLEMENTATION
#include "pntr_app_sfx.h"

// generate a "jump" sound
SfxParams params = {0};
pntr_app_sfx_gen_jump(app, &params);

// output those params as a pntr_sound
pntr_sound* jumpy = pntr_app_sfx_sound(app, &params);
```

## API

```c
// load a SfxParams as a pntr_sound
pntr_sound* pntr_app_sfx_sound(pntr_app* app, SfxParams* params);

// Load/Save file functions (for rfx files)
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

// utils for messing with SfxParams
void pntr_app_sfx_reset_params(SfxParams* params);
SfxSynth* pntr_app_sfx_alloc_synth(int format, int sampleRate, int maxDuration);
int pntr_app_sfx_generate_wave(pntr_app* app, SfxSynth*, const SfxParams* params);
```