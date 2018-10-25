#include "adafruitUltimateGPS.h"

#define BAUDRATE B9600   // Change as needed, keep B
#define MODEMDEVICE "/dev/ttyO1" //Beaglebone Black serial port
#define _POSIX_SOURCE 1 /* POSIX compliant source */

int fd;
void initGPS(RingBuffer *temp)
{
	RingBuffer_Init(temp);
	// int c;
    	struct termios newtio;
    	//char buf[255];
    	// Load the pin configuration
    	// int ret = system("echo uart1 > /sys/devices/bone_capemgr.9/slots");
    	
    	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    	if (fd < 0) { perror(MODEMDEVICE); exit(-1); }

    	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

    	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

    	newtio.c_iflag = IGNPAR;

    	newtio.c_oflag = 0;

    	newtio.c_lflag = ICANON;
    	tcflush(fd, TCIFLUSH);
    	tcsetattr(fd,TCSANOW,&newtio);
    	// NMEA command to ouput all sentences
    	// Note that this code & format values in manual are hexadecimal
    	write(fd, "$PTNLSNM,273F,01*27\r\n", 21);
    	
    	//tcsetattr(fd, TCSANOW, &oldtio);
}

void GPSIntHandler(RingBuffer *temp)
{
	int res;
	char buf[LENGTH_BUFFER];
	
        res = read(fd, buf, LENGTH_BUFFER);
        buf[res] = 0;             /* set end of string, so we can printf */
        //printf("%s", buf, res);
	//printf("res: %d\n", res);
	int i;
	for (i=0; buf[i] != '\0'; i++)
	{
		RingBuffer_Push(buf[i],temp);
	}
}


void GPSGetData(RingBuffer *temp, struct GPSDATA *data)
{
	int commagps=0;
	char gpsheader[5];
	bool garbageflaggps=false;
	char alpha;
	int j;
	while (((temp->tail) != (temp->head)) && !garbageflaggps)
	{
		switch (temp->state)
		{
		case 'A':
			alpha = RingBuffer_PopChar(temp);
			//searching for $
			if (alpha == '$')
			{
				temp->state = 'B';
			}
			break;
		case 'B':
			//Obtain the header and comparing it
			for (j = 0; j <= 4; j++)
			{
				gpsheader[j] = RingBuffer_PopChar(temp);
			}
			if (gpsheader[0] == 'G' && gpsheader[1] == 'P')
			{
				if(gpsheader[2]=='R' && gpsheader[3]=='M' && gpsheader[4]=='C')
				{
					temp->state='C';
					commagps=0;
				}
				else
				{
					garbageflaggps = true;
				}
			}
			else
			{
				garbageflaggps = true;
			}
			break;

		case 'C':
			//Check whether the data after specified commas is correct
			alpha = RingBuffer_PopChar(temp);
			if (alpha == ',')
			{
				commagps++;
				if (commagps == 1)
				{
					for (j=0;;j++)
					{	
						data->Time[j]= RingBuffer_PopChar(temp);
						if(data->Time[j]==',')
						{
							commagps++;
							data->Time[j]='\0';
							break;
						}
					}
				}	
				if (commagps == 11)
				{
					commagps=0;
					temp->state = 'D';
				}
	    		}
			break;
		case 'D':
			//Check for invalid data and \n:
			alpha = RingBuffer_PopChar(temp);			

			if (alpha == '\n')
			{
				temp->sentence_flag = true;
				temp->state = 'A';
				commagps=0;
						
			}
			else
			{
				garbageflaggps = true;
				temp->sentence_flag = false;
			}
			break;
		}
	}
}

void RingBuffer_Init(RingBuffer *temp)
{
	temp->head=0;
	temp->tail=0;
	temp->state='A';
	temp->sentence_flag=false;
	temp->size=LENGTH_BUFFER;
}

void RingBuffer_Push(char letter,RingBuffer *temp)
{
	temp->buffer[temp->head]=letter;
	temp->head=((temp->head)+1)%(temp->size);
	if(temp->head==temp->tail)
	{
		temp->tail=((temp->tail)+1)%temp->size;
	}
}

char RingBuffer_PopChar(RingBuffer *temp)
{
	char out_letter;
	out_letter=temp->buffer[temp->tail];
	temp->tail=((temp->tail)+1)%temp->size;
	return out_letter;
}

void RingBuffer_TransBuffer(RingBuffer *source, RingBuffer *destination)
{
	while((source->tail)!=(source->head))
	{
		RingBuffer_Push(RingBuffer_PopChar(source),destination);
	}
}
void RingBuffer_Clear(RingBuffer *temp)
{
	int k=0;
	for(k=0;k<temp->size;k++)
	{
		temp->buffer[k]='0';
	}
	temp->head=0;
	temp->tail=0;
	temp->sentence_flag=false;
	temp->state='A';
}


void RingBuffer_Print(RingBuffer *temp)
{
	int i=0;
	int o_head=temp->head;
	int o_tail=temp->tail;
	while((temp->tail)!=(temp->head))
	{
		printf("%c",temp->buffer[temp->tail]);
		temp->tail=((temp->tail)+1)%(temp->size);
		i++;
	}
	printf("\n");
	temp->tail=o_tail;
	temp->head=o_head;
}

void RingBuffer_PrintIndex(RingBuffer *curr)
{
	printf("\nHead: %d\t Tail: %d\n",curr->head,curr->tail);
}
