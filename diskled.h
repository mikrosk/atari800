#define DISKLED_FONT_WIDTH		5
#define DISKLED_FONT_HEIGHT		7
#define DISKLED_FONT_CHARSIZE	(DISKLED_FONT_WIDTH * DISKLED_FONT_HEIGHT)

static unsigned long ulDiskState = 0;
static unsigned long ulDiskUnit = LED_NO_UNIT;
static unsigned char DiskLED[]= {
172,172,172,172,172,
172,172,000,172,172,
172,000,000,172,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,172,172,172,
		
172,172,172,172,172,
172,000,000,172,172,
172,172,172,000,172,
172,172,000,172,172,
172,000,172,172,172,
172,000,000,000,172,
172,172,172,172,172,
		
172,172,172,172,172,
172,000,000,172,172,
172,172,172,000,172,
172,172,000,172,172,
172,172,172,000,172,
172,000,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,172,172,000,172,
172,172,000,000,172,
172,000,172,000,172,
172,000,000,000,172,
172,172,172,000,172,
172,172,172,172,172,

172,172,172,172,172,
172,000,000,000,172,
172,000,172,172,172,
172,000,000,000,172,
172,172,172,000,172,
172,000,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,172,000,172,172,
172,000,172,172,172,
172,000,000,172,172,
172,000,172,000,172,
172,172,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,000,000,000,172,
172,172,172,000,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,000,172,172,
172,172,172,172,172,

172,172,172,172,172,
172,172,000,172,172,
172,000,172,000,172,
172,172,000,172,172,
172,000,172,000,172,
172,172,000,172,172,
172,172,172,172,172,

172,172,172,172,172,		
172,172,000,172,172,
172,000,172,000,172,
172,000,172,000,172,
172,000,172,000,172,
172,172,000,172,172,
172,172,172,172,172,	/* End of read LEDs	*/

053,053,053,053,053,	/* Start of write LEDs */
053,053,000,053,053,
053,000,000,053,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,000,000,053,053,
053,053,053,000,053,
053,053,000,053,053,
053,000,053,053,053,
053,000,000,000,053,
053,053,053,053,053,

053,053,053,053,053,
053,000,000,053,053,
053,053,053,000,053,
053,053,000,053,053,
053,053,053,000,053,
053,000,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,053,000,053,
053,053,000,000,053,
053,000,053,000,053,
053,000,000,000,053,
053,053,053,000,053,
053,053,053,053,053,


053,053,053,053,053,
053,000,000,000,053,
053,000,053,053,053,
053,000,000,000,053,
053,053,053,000,053,
053,000,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,000,053,053,
053,000,053,053,053,
053,000,000,053,053,
053,000,053,000,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,000,000,000,053,
053,053,053,000,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,000,053,053,
053,000,053,000,053,
053,053,000,053,053,
053,000,053,000,053,
053,053,000,053,053,
053,053,053,053,053,

053,053,053,053,053,
053,053,000,053,053,
053,000,053,000,053,
053,000,053,000,053,
053,000,053,000,053,
053,053,000,053,053,	
053,053,053,053,053	/* End of write LEDs	*/
};
