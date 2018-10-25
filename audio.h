#include <stdint.h>
#include <math.h>
#include <alsa/asoundlib.h>


static char *device = "plughw:0,0";                        /* playback device */
snd_pcm_t *handle;
snd_pcm_sframes_t frames;


int pcm_init(double Fs);
void generate_signal(int16_t *, double);
int pcm_play(int16_t *buffer, int size);