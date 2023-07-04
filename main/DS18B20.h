#include <inttypes.h>
#include "onewire.h"

#include "digital.h"
#include "debug.h"

class DS18B20
{

private:
	ONEWIRE *onewire;
	char CRC(char end[]);
	void capturaBit(int posicao, char v[], int valor);

	// global search state Ow_Search();
	unsigned char ROM_NO[8];
	int LastDiscrepancy;
	int LastFamilyDiscrepancy;
	int LastDeviceFlag;
	unsigned char crc8;
	unsigned char docrc8(unsigned char value);

public:
	DS18B20(gpio_num_t pino);
	float readTemp(void);
	float readTargetTemp(char end[]);

	void init(void);
	void init(char v[]);
	void fazScanProfessor(void);
	void fazScanNosso(void);
};
