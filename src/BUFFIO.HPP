#ifndef BUFFIOHPP
#define BUFFIOHPP

#define  MAX_BUFF_SIZE     32000       /* max. buffer size allowed */
#define	ERR_BUFFER			-1				/* general buffer error */

#define	BUFF_ALLOC(size)	(struct sBuffer *)new unsigned char[(size+1+sizeof(struct sBuffer))]


struct sBuffer
{
	unsigned char	*get, *put, *end;
	unsigned int	space;
	unsigned char	data[0];
};

int  iBuffInit(struct sBuffer *bPtr, int bSize);
int  iBuffPut(struct sBuffer *bPtr, char cc);
int  iBuffGet(struct sBuffer *bPtr );

#endif

