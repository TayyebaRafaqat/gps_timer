#include "audio.h"

int pcm_init(double Fs){
	int err;

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
                printf("Playback open error: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
		return -1;
        }
	
	if ((err = snd_pcm_set_params(handle,
                                      SND_PCM_FORMAT_S16_LE,
                                      SND_PCM_ACCESS_RW_INTERLEAVED,
                                      1,
                                      Fs,
                                      1,
                                      10000)) < 0)
	{   /* 5msec */
                printf("Playback open error: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
		return -1;
        }

	return 0;
}

void generate_signal(int16_t *buffer, double Fs){
	unsigned int i;

	for (i = 0; i < 44100; i++)
	                buffer[i] = 16383.0 * sin(i*1000.0*2.0*M_PI/Fs);

}

int pcm_play(int16_t *buffer, int size){
	frames = snd_pcm_writei(handle, buffer, size/2);
	printf("frames: %li\n", frames);
	if (frames < 0)
		{
			printf("Error: %s\n", snd_strerror(frames));
                        frames = snd_pcm_recover(handle, frames, 0);
                        printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
                        return -1;
                }
	if (frames > 0 && frames < (long)size){
                printf("Short write (expected %li, wrote %li)\n", (long)size, frames);
		return -1;
	}
	return 0;
}