#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED
#define SETTINGS_KEY 1

typedef struct ClaySettings {
  int Language;
} ClaySettings;

const char* get_day_name(int dayOfWeek);

void handle_settings_changed(DictionaryIterator *iter, void *context);
#endif