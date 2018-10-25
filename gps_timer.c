
#include "adafruitUltimateGPS.h"
#include "__pruFn.h"
#include "iolib.h"
#include "audio.h"

RingBuffer rb_GPS;
struct GPSDATA data;
#define N 44100
double Fs = 44100.0;  // sampling frequency
int16_t buffer[N];

int main()
{
	printf("TASK 5\n");
	iolib_init();	// Initialize Gpio library "iolib"
	iolib_setdir(8,26, DIR_OUT);	// Set GPIO Pin P8_26 as output currently in pull-up mode
	pin_low(8,26);

	/* initialize the library, PRU and interrupt; launch our PRU program */
	if(pru_setup("./gps_timer.bin")) {
		pru_cleanup();
		return -1;
    	}

	// Initialize pcm
	pcm_init(Fs);

	// Initialize GPS
	initGPS(&rb_GPS);

	bool timeFlag = false;
	const char id = '4';

	while (!timeFlag)
	{
		GPSIntHandler(&rb_GPS);		// Reads string form GPS and store in buffer
		RingBuffer_Print(&rb_GPS);	// Prints Buffer 
		memset(data.Time, 0, sizeof(data.Time));	// Clear data.Time
		GPSGetData(&rb_GPS, &data);	// Parse Buffer String and stores time in data.Time

		if (strlen(data.Time) != 0 && data.Time[5] == id){
			timeFlag = true;
			printf("Time: %s\n", data.Time);
		}
		RingBuffer_Clear(&rb_GPS);	// Clear Buffer
	}

	// Sends signal to pru to sync with pps and start timer
	prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
	printf("Pru Code Execution Started.... ARM_PRU0_INTERRUPT\n");
	prussdrv_pru_wait_event (PRU_EVTOUT_0);		// PRU acknowledges after syncing with PPS
	printf("PPS Arrived\n");
	pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);	// Clears Event and Interrupt

	unsigned int ret;
	int i=0;
	printf("Buffer Size: %d\n", sizeof(buffer));
	while(i++ < 5){
		/* Function readies Signal */
		generate_signal(buffer, Fs);

		// Wait for PRU interrurt to audio Out
		ret = prussdrv_pru_wait_event (PRU_EVTOUT_0); //Waiting for this instruction: MOV r31.b0, PRU0_ARM_INTERRUPT+16
		printf("PRU Event_0 No: %d\n",ret);
		//snd_pcm_reset(handle);
		snd_pcm_pause(handle, 0);
		/* Audio OUT */
		pin_high(8,26);
		pcm_play(buffer, sizeof(buffer));
		pin_low(8,26);
		pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);	// Clears Event and Interrupt
		snd_pcm_pause(handle, 1);
	}

	// Send Event to PRU to stop 
	prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

	// Pru PRU_EVTOUT_1 to acknowledge that program is halted
	printf("PRU Event_1 No: %d\n", prussdrv_pru_wait_event (PRU_EVTOUT_1));
	pru_clear_event(PRU_EVTOUT_1, PRU1_ARM_INTERRUPT);
	printf("Pru Halted\n");

	// Disable PRU and release driver
	pru_cleanup();

	// Close pcm
	snd_pcm_close(handle);

	return 0;
}
