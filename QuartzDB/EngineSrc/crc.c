#define SHT3X_CRC8_POLYNOMIAL  0x31

static char _gTable[256];

static void __MakeTable(char polynomial)
{
	int i, j;
	const char msbit = 0x80;
	char       t = msbit;

	_gTable[0] = 0;

	for (i = 1; i <= 255; i *= 2) {

		t = (t << 1) ^ (t & msbit ? polynomial : 0);

		for (j = 0; j < i; j++)
			_gTable[i+j] = _gTable[j] ^ t;
	}
}

char _GetCrc(char *pdata, int nbytes, char crc)
{
	static char bInit = 0;

	if (!bInit) {
		__MakeTable(SHT3X_CRC8_POLYNOMIAL);
		bInit = 1;
	}

	/* loop over the buffer data */
	while (nbytes-- > 0)
		crc = _gTable[(crc ^ *pdata++) & 0xff];

	return crc;
}