#include <inttypes.h> 
#include "onewire.h"

#include "digital.h"
#include "debug.h"


class DS18B20 {

private:
	ONEWIRE *onewire;
	char CRC (char end[]);
	void capturaBit (int posicao, char v[], int valor);
public:
	DS18B20 (gpio_num_t pino);
	float readTemp (void);
	float readTargetTemp (char end[]);
	
	void init (void);
	void init (char v[]);
	void fazScanProfessor(void);
	void fazScanNosso(void);
};
