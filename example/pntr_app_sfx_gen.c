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

#ifdef EMSCRIPTEN
EM_JS(void, download_file, (char* filenamePtr, unsigned char* dataPtr, int size, char* mimeTypePtr), {
  const a = document.createElement('a');
  a.style = 'display:none';
  document.body.appendChild(a);
  const blob = new Blob([new Uint8Array(Module.HEAPU8.buffer, dataPtr, size)], { type: UTF8ToString(mimeTypePtr) });
  a.href = window.URL.createObjectURL(blob);
  a.download = UTF8ToString(filenamePtr);
  a.click();
  window.URL.revokeObjectURL(a.href);
  document.body.removeChild(a);
});
#endif  // EMSCRIPTEN

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

#ifdef EMSCRIPTEN

  // Play
  if (nk_begin(ctx, "Play", nk_rect(screen->width / 3, 0, (screen->width / 3), screen->height / 6), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 0, 1);
    if (nk_button_label(ctx, "Play")) {
      pntr_app_sfx_gen_play(app);
    }
  }
  nk_end(ctx);

  // Save
  if (nk_begin(ctx, "Save", nk_rect((screen->width / 3) + (screen->width / 3), 0, (screen->width / 3), screen->height / 6), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 0, 1);
    if (nk_button_label(ctx, "Save")) {
      unsigned char fileData[104] = {0};

      char* signature = "rFX ";
      short int version = 200;
      short int len = 96;
      PNTR_MEMCPY(&fileData, signature, 4);
      PNTR_MEMCPY(fileData + 4, &version, 2);
      PNTR_MEMCPY(fileData + 6, &len, 2);
      PNTR_MEMCPY(fileData + 8, &appData->sfx_params, 96);

      download_file("sound.rfx", fileData, 104, "octet/stream");
    }
  }
  nk_end(ctx);

#else

  // Play
  if (nk_begin(ctx, "Play", nk_rect(screen->width / 3, 0, (screen->width / 3) * 2, screen->height / 6), NK_WINDOW_NO_SCROLLBAR)) {
    nk_layout_row_dynamic(ctx, 0, 1);
    if (nk_button_label(ctx, "Play")) {
      pntr_app_sfx_gen_play(app);
    }
  }
  nk_end(ctx);

#endif

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
