#include		"buffio.hpp"


int iBuffInit(struct sBuffer *bPtr, int bSize)
	/* initialize a circular buffer given the address of the buffer
		and its size. */
{
	if (bSize > MAX_BUFF_SIZE)
		return(ERR_BUFFER);
	else
	{
		bPtr->get = bPtr->data;
      bPtr->put = bPtr->data;
		bPtr->end = bPtr->data + bSize;
		bPtr->space = bSize;
		return(0);
	}
}

int iBuffPut(struct sBuffer *bPtr, char cc)
	/* add a byte to a circular buffer given the address of the buffer
		and the character. */
{
	if (bPtr->space)
	{
		*(bPtr->put) = cc;
		if ( ++bPtr->put >= bPtr->end )
			bPtr->put = bPtr->data;
		return(--(bPtr->space));
	}
	else
		return(ERR_BUFFER);
}

int iBuffGet(struct sBuffer *bPtr)
	/* add a byte to a circular buffer given the address of the buffer
		and the character. */
{

	int	cc;

	if (bPtr->space && (bPtr->get == bPtr->put) )
		return(ERR_BUFFER);
	else
	{
		cc = *(bPtr->get);
		if ( ++bPtr->get >= bPtr->end )
			bPtr->get = bPtr->data;
		++(bPtr->space);
		return(cc);
	}
}
