#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#define LENGTH_BUFFER 550

typedef struct{
	char buffer[LENGTH_BUFFER];
	int size;
	int head;
	int tail;
	char state;
	bool sentence_flag;
}RingBuffer;

void RingBuffer_Init(RingBuffer *temp);
void RingBuffer_Push(char letter,RingBuffer *temp);
char RingBuffer_PopChar(RingBuffer *temp);
void RingBuffer_TransBuffer(RingBuffer *source, RingBuffer *destination);
void RingBuffer_Clear(RingBuffer *temp);
void RingBuffer_Print(RingBuffer *temp);
void RingBuffer_PrintIndex(RingBuffer *curr);

struct GPSDATA				//structure for storing GPS data acquired by NMEA responses.
{
	char Time[30];			//to store time given by GPS
	char Date[30];			//to store date
	char Lat[30];			//to store latitude
	char Long[30];			//to store longitude
	char Alt[30];			//to store altitude
	char Quality[13];		//to store signal quality
	char Sat[15];			//to store number of satellites from which response was generated
	char Speed[15];			//to store speed
	char Dop[15];			//to store dilusion of precision
	char Bearing[15];		//to store bearing
};
//constant pointer of struct GPSDATA, declared so that user can READ (only) in the main application
extern struct GPSDATA const * const GPSData;

void initGPS(RingBuffer *temp);
void GPSGetData(RingBuffer *temp, struct GPSDATA *data);
bool isGPSSentenceComplete(void);
bool isGPSDataUpdated(void);
void GPSIntHandler(RingBuffer *temp);
