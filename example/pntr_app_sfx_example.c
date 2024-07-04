#define PNTR_APP_IMPLEMENTATION
#define PNTR_ENABLE_DEFAULT_FONT
#define PNTR_ENABLE_VARGS
#define PNTR_DISABLE_MATH
#include "pntr_app.h"

#define PNTR_APP_SFX_IMPLEMENTATION
#include "pntr_app_sfx.h"

typedef struct AppData {
  pntr_sound* sfx;
  pntr_font* font;
  SfxParams sfx_params;
} AppData;

bool Init(pntr_app* app) {
  AppData* appData = pntr_load_memory(sizeof(AppData));
  pntr_app_set_userdata(app, appData);

  pntr_app_sfx_gen_jump(app, &appData->sfx_params);
  appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);

  appData->font = pntr_load_font_default();

  return true;
}

bool Update(pntr_app* app, pntr_image* screen) {
  AppData* appData = (AppData*)pntr_app_userdata(app);
  pntr_clear_background(screen, PNTR_RAYWHITE);

  pntr_draw_text(screen, appData->font, "1 - Pickup Coin", 10, 10, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "2 - Laser Shoot", 10, 20, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "3 - Explosion", 10, 30, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "4 - Powerup", 10, 40, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "5 - Hit/Hurt", 10, 50, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "6 - Jump", 10, 60, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "7 - Blip/Select", 10, 70, PNTR_DARKGRAY);
  pntr_draw_text(screen, appData->font, "8 - Synth", 10, 80, PNTR_DARKGRAY);

  return true;
}

void Close(pntr_app* app) {
  AppData* appData = (AppData*)pntr_app_userdata(app);
  pntr_unload_sound(appData->sfx);
  pntr_unload_font(appData->font);
  pntr_unload_memory(appData);
}

void Event(pntr_app* app, pntr_app_event* event) {
  AppData* appData = (AppData*)pntr_app_userdata(app);

  if (event->type == PNTR_APP_EVENTTYPE_KEY_DOWN) {
    if (event->key == PNTR_APP_KEY_1) {
      pntr_app_sfx_gen_pickup_coin(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_2) {
      pntr_app_sfx_gen_laser_shoot(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_3) {
      pntr_app_sfx_gen_explosion(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_4) {
      pntr_app_sfx_gen_powerup(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_5) {
      pntr_app_sfx_gen_hit_hurt(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_6) {
      pntr_app_sfx_gen_jump(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_7) {
      pntr_app_sfx_gen_blip_select(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    } else if (event->key == PNTR_APP_KEY_8) {
      pntr_app_sfx_gen_synth(app, &appData->sfx_params);
      pntr_unload_sound(appData->sfx);
      appData->sfx = pntr_app_sfx_sound(app, &appData->sfx_params);
      pntr_play_sound(appData->sfx, false);
    }
  }
}

pntr_app Main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  return (pntr_app){
      .width = 400,
      .height = 225,
      .title = "pntr_app_sfx: Example",
      .init = Init,
      .update = Update,
      .close = Close,
      .event = Event,
      .fps = 0};
}
