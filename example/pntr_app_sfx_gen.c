#include <string.h>

#define PNTR_APP_IMPLEMENTATION
#define PNTR_ENABLE_DEFAULT_FONT
#define PNTR_ENABLE_VARGS
#define PNTR_ENABLE_MATH
#include "pntr_app.h"

#define PNTR_APP_SFX_IMPLEMENTATION
#include "pntr_app_sfx.h"

#define PNTR_NUKLEAR_IMPLEMENTATION
#include "pntr_nuklear.h"

typedef struct AppData {
  pntr_sound* sfx;
  pntr_font* font;
  SfxParams sfx_params;
  struct nk_context* ctx;
} AppData;

// this will allow user to select a save-file and download
#ifdef EMSCRIPTEN
EM_ASYNC_JS(void, download_rfx_file, (unsigned char* dataPtr, int size, char* suggestedNamePtr, char* startInPtr, char* mimeTypePtr), {
  try {
    const f = await showSaveFilePicker({suggestedName : UTF8ToString(suggestedNamePtr), startIn : UTF8ToString(startInPtr)});
    const writableStream = await f.createWritable();
    const blob = new Blob([new Uint8Array(Module.HEAPU8.buffer, dataPtr, size)], { type: UTF8ToString(mimeTypePtr) });
    await writableStream.write(blob);
    await writableStream.close();
  } catch (e) {
    console.log('rfx file-save aborted.');
  }
});
#endif  // EMSCRIPTEN

// output the code on the console
void save_code(pntr_app* app, SfxParams sfx_params) {
  char code[1024];
  snprintf(code, 1024,
      "#define PNTR_APP_SFX_IMPLEMENTATION\n\
#include \"pntr_app_sfx.h\"\n\
\n\
SfxParams gen_sound_effect() {\n\
  return (SfxParams) {\n\
    .randSeed=%lu,\n\
    .waveType=%d,\n\
    .attackTime=%ff,\n\
    .sustainTime=%ff,\n\
    .sustainPunch=%ff,\n\
    .decayTime=%ff,\n\
    .startFrequency=%ff,\n\
    .minFrequency=%ff,\n\
    .slide=%ff,\n\
    .deltaSlide=%ff,\n\
    .vibratoDepth=%ff,\n\
    .vibratoSpeed=%ff,\n\
    .changeAmount=%ff,\n\
    .changeSpeed=%ff,\n\
    .squareDuty=%ff,\n\
    .dutySweep=%ff,\n\
    .repeatSpeed=%ff,\n\
    .phaserOffset=%ff,\n\
    .phaserSweep=%ff,\n\
    .lpfCutoff=%ff,\n\
    .lpfCutoffSweep=%ff,\n\
    .lpfResonance=%ff,\n\
    .hpfCutoff=%ff,\n\
    .hpfCutoffSweep=%ff };\n\
}\n",
      (unsigned long)sfx_params.randSeed, sfx_params.waveType, sfx_params.attackTime, sfx_params.sustainTime, sfx_params.sustainPunch, sfx_params.decayTime, sfx_params.startFrequency, sfx_params.minFrequency, sfx_params.slide, sfx_params.deltaSlide, sfx_params.vibratoDepth, sfx_params.vibratoSpeed, sfx_params.changeAmount, sfx_params.changeSpeed, sfx_params.squareDuty, sfx_params.dutySweep, sfx_params.repeatSpeed, sfx_params.phaserOffset, sfx_params.phaserSweep, sfx_params.lpfCutoff, sfx_params.lpfCutoffSweep, sfx_params.lpfResonance, sfx_params.hpfCutoff, sfx_params.hpfCutoffSweep);

  // Export it to the clipboard and the logs.
  pntr_app_set_clipboard(app, code, 0);
  printf("%s\n", code);
}

SfxParams gen_sound_effect() {
  return (SfxParams){
      .randSeed = 0.000000f,
      .waveType = 0,
      .attackTime = 0.204667f,
      .sustainTime = 0.000000f,
      .sustainPunch = 0.255497f,
      .decayTime = 0.487662f,
      .startFrequency = 0.000000f,
      .minFrequency = 0.209907f,
      .slide = 0.000000f,
      .deltaSlide = 0.000000f,
      .vibratoDepth = 0.000000f,
      .vibratoSpeed = 0.000000f,
      .changeAmount = 0.000000f,
      .changeSpeed = 0.437060f,
      .squareDuty = 0.000000f,
      .dutySweep = 0.000000f,
      .repeatSpeed = 0.000000f,
      .phaserOffset = 0.000000f,
      .phaserSweep = 1.000000f,
      .lpfCutoff = 0.000000f,
      .lpfCutoffSweep = 0.000000f,
      .lpfResonance = 0.194383f,
      .hpfCutoff = 0.000000f,
      .hpfCutoffSweep = 0.000000f};
}

bool Init(pntr_app* app) {
  AppData* appData = pntr_load_memory(sizeof(AppData));
  pntr_app_set_userdata(app, appData);

  pntr_app_sfx_gen_jump(app, &appData->sfx_params);
  appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);

  appData->font = pntr_load_font_default();

  appData->ctx = pntr_load_nuklear(appData->font);

  return true;
}

void pntr_app_sfx_gen_play(pntr_app* app) {
  AppData* appData = (AppData*)pntr_app_userdata(app);
  pntr_unload_sound(appData->sfx);
  appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
  pntr_play_sound(appData->sfx, false);
}

bool Update(pntr_app* app, pntr_image* screen) {
  AppData* appData = (AppData*)pntr_app_userdata(app);
  pntr_clear_background(screen, PNTR_RAYWHITE);

  // Nuklear GUI Code
  struct nk_context* ctx = appData->ctx;
  pntr_nuklear_update(ctx, app);

  // Load Preset
  if (nk_begin(ctx, "Load Preset", nk_rect(0, 0, screen->width / 3, screen->height), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 0, 1);

    if (nk_button_label(ctx, "Pickup Coin")) {
      pntr_app_sfx_gen_pickup_coin(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Laser Shoot")) {
      pntr_app_sfx_gen_laser_shoot(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Explosion")) {
      pntr_app_sfx_gen_explosion(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Powerup")) {
      pntr_app_sfx_gen_powerup(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Hit Hurt")) {
      pntr_app_sfx_gen_hit_hurt(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Jump")) {
      pntr_app_sfx_gen_jump(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Blip Select")) {
      pntr_app_sfx_gen_blip_select(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }

    if (nk_button_label(ctx, "Synth")) {
      pntr_app_sfx_gen_synth(app, &appData->sfx_params);
      pntr_app_sfx_gen_play(app);
    }
  }
  nk_end(ctx);

  // Play
  if (nk_begin(ctx, "Play", nk_rect(screen->width / 3, 0, (screen->width / 3), screen->height / 6), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 0, 1);
    if (nk_button_label(ctx, "Play")) {
      pntr_app_sfx_gen_play(app);
    }
  }
  nk_end(ctx);

  // Save
  if (nk_begin(ctx, "Save / Copy to Clipboard", nk_rect((screen->width / 3) + (screen->width / 3), 0, (screen->width / 3), screen->height / 6), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 0, 1);
    if (nk_button_label(ctx, "Save")) {
      save_code(app, appData->sfx_params);

#ifdef EMSCRIPTEN
      // build file-string
      unsigned char fileData[104] = {0};
      char* signature = "rFX ";
      short int version = 200;
      short int len = 96;
      PNTR_MEMCPY(&fileData, signature, 4);
      PNTR_MEMCPY(fileData + 4, &version, 2);
      PNTR_MEMCPY(fileData + 6, &len, 2);
      PNTR_MEMCPY(fileData + 8, &appData->sfx_params, 96);

      download_rfx_file(fileData, 104, "sound.rfx", "downloads", "octet/stream");
#endif  // EMSCRIPTEN
    }
  }
  nk_end(ctx);

  // Configure
  if (nk_begin(ctx, "Configure", nk_rect(screen->width / 3, screen->height / 6, (screen->width / 3) * 2, screen->height - (screen->height / 6)), 0)) {
    nk_layout_row_dynamic(ctx, 0, 2);

    nk_label(ctx, "Wave Type:", NK_TEXT_LEFT);
    nk_combobox_separator(ctx, "Square;Sawtooth;Sine;Noise", ';', &appData->sfx_params.waveType, 4, 30, nk_vec2((screen->width / 4), (screen->height / 3) * 2));

    nk_label(ctx, "Attack Time:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.attackTime, 1.0f, 0.05f);

    nk_label(ctx, "Sustain Time:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.sustainTime, 1.0f, 0.05f);

    nk_label(ctx, "Sustain Punch:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.sustainPunch, 1.0f, 0.05f);

    nk_label(ctx, "Decay Time:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.decayTime, 1.0f, 0.05f);

    nk_label(ctx, "Start Frequency:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.startFrequency, 1.0f, 0.05f);

    nk_label(ctx, "Minimum Frequency:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.minFrequency, 1.0f, 0.05f);

    nk_label(ctx, "Slide:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.slide, 1.0f, 0.05f);

    nk_label(ctx, "Delta Slide:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.deltaSlide, 1.0f, 0.05f);

    nk_label(ctx, "Vibrato Depth:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.vibratoDepth, 1.0f, 0.05f);

    nk_label(ctx, "Vibrato Speed:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.vibratoSpeed, 1.0f, 0.05f);

    nk_label(ctx, "Change Amount:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.changeAmount, 1.0f, 0.05f);

    nk_label(ctx, "Change Speed:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.changeSpeed, 1.0f, 0.05f);

    nk_label(ctx, "Square Duty:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.squareDuty, 1.0f, 0.05f);

    nk_label(ctx, "Duty Sweep:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.dutySweep, 1.0f, 0.05f);

    nk_label(ctx, "Repeat Speed:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.repeatSpeed, 1.0f, 0.05f);

    nk_label(ctx, "Phaser Offset:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.phaserOffset, 1.0f, 0.05f);

    nk_label(ctx, "Phaser Sweep:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.phaserSweep, 1.0f, 0.05f);

    nk_label(ctx, "lpfCutoff:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.lpfCutoff, 1.0f, 0.05f);

    nk_label(ctx, "lpfCutoffSweep:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.lpfCutoffSweep, 1.0f, 0.05f);

    nk_label(ctx, "lpfResonance:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.lpfResonance, 1.0f, 0.05f);

    nk_label(ctx, "hpfCutoff:", NK_TEXT_LEFT);
    nk_slider_float(ctx, 0.0f, &appData->sfx_params.hpfCutoff, 1.0f, 0.05f);

    nk_label(ctx, "hpfCutoffSweep:", NK_TEXT_LEFT);
    nk_slider_float(ctx, -1.0f, &appData->sfx_params.hpfCutoffSweep, 1.0f, 0.05f);
  }
  nk_end(ctx);

  pntr_draw_nuklear(screen, ctx);

  return true;
}

void Close(pntr_app* app) {
  AppData* appData = (AppData*)pntr_app_userdata(app);
  pntr_unload_nuklear(appData->ctx);
  pntr_unload_sound(appData->sfx);
  pntr_unload_font(appData->font);
  pntr_unload_memory(appData);
}

void Event(pntr_app* app, pntr_app_event* event) {
  AppData* appData = (AppData*)pntr_app_userdata(app);

  if (event->type == PNTR_APP_EVENTTYPE_KEY_DOWN) {
    switch (event->key) {
      case PNTR_APP_KEY_SPACE:
      case PNTR_APP_KEY_ENTER:
        pntr_app_sfx_gen_play(app);
        break;
    }
  }
}

pntr_app Main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  return (pntr_app){
      .width = 400,
      .height = 225,
      .title = "pntr_app_sfx: Generator",
      .init = Init,
      .update = Update,
      .close = Close,
      .event = Event,
      .fps = 0};
}
