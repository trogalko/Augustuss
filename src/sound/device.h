#ifndef SOUND_DEVICE_H
#define SOUND_DEVICE_H

typedef enum {
    SOUND_TYPE_MIN = 0,
    SOUND_TYPE_SPEECH = 0,
    SOUND_TYPE_EFFECTS,
    SOUND_TYPE_CITY,
    SOUND_TYPE_MUSIC,
    SOUND_TYPE_MAX
} sound_type;

void sound_device_open(void);
void sound_device_close(void);

void sound_device_init_channels(void);
int sound_device_is_file_playing_on_channel(const char *filename, sound_type type);

void sound_device_set_music_volume(int volume_pct);
void sound_device_set_volume_for_type(sound_type type, int volume_pct);

int sound_device_play_music(const char *filename, int volume_pct, int loop);
int sound_device_play_file_on_channel_panned(const char *filename, sound_type type,
    int volume_pct, int left_pct, int right_pct);
int sound_device_play_file_on_channel(const char *filename, sound_type type, int volume_pct);
void sound_device_stop_music(void);
void sound_device_stop_type(sound_type type);

void sound_device_on_audio_finished(void (*callback)(sound_type));
void sound_device_fadeout_music(int milisseconds);

/**
 * Use a custom music player, for external music data (e.g. videos)
 * @param bitdepth Bitdepth, either 8 or 16
 * @param num_channels Number of channels, 1 = mono, 2 = stereo
 * @param rate Frequency, usually 22050 or 44100
 * @param audio_data First chunk of music data
 * @param len Length of data
 */
void sound_device_use_custom_music_player(int bitdepth, int num_channels, int rate, const void *audio_data, int len);

/**
 * Writes custom music data
 * @param audio_data Music data
 * @param len Length
 */
void sound_device_write_custom_music_data(const void *audio_data, int len);

void sound_device_use_default_music_player(void);

#endif // SOUND_DEVICE_H
