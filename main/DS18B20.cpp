#ifndef __DS18B20__
#define __DS18B20__

#include "DS18B20.h"
#include "delay.h"
#include <vector>


void DS18B20::capturaBit(int posicao, char v[], int valor)
{
	unsigned char pbyte = posicao / 8;
	unsigned char pbit = posicao % 8;

	if (valor == 1)
		v[pbyte] |= (1 << pbit);
	else
		v[pbyte] &= ~(1 << pbit);
}

void DS18B20::fazScanProfessor(void)
{
	char v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t normal, complemento;

	onewire->reset();
	onewire->writeByte(SEARCH_ROM);

	for (int x = 0; x < 64; x++)
	{
		normal = onewire->readBit();

		complemento = onewire->readBit();
		if (normal == 0 && complemento == 0)
		{
			// bits conflitantes na posicao

			onewire->escreve_bit(0);
			capturaBit(x, v, 0);
		}
		if (normal == 0 && complemento == 1)
		{
			// o bit é 0

			capturaBit(x, v, 0);
			onewire->escreve_bit(0);
		}
		if (normal == 1 && complemento == 0)
		{
			// o bit é 1

			capturaBit(x, v, 1);
			onewire->escreve_bit(1);
		}
		if (normal == 1 && complemento == 1)
		{
			// nao existem escravos no barramento
			printf("Nao existem escravos no barramento\n");
			return;
		}
	}
	printf("Codigo da Familia: %d\n", v[0]);
	printf("Numero de Serie  : %d %d %d %d %d %d\n", v[6], v[5], v[4], v[3], v[2], v[1]);
	printf("CRC=             : %d\n", v[7]);

	printf("Endereco completo: %d %d %d %d %d %d %d %d\n", v[7], v[6], v[5], v[4], v[3], v[2], v[1], v[0]);
}

void DS18B20::fazScanNosso(void)
{
    char searchDirection = 0;
    char lastZero = 0;
    char lastDiscrepancy = 0;
    bool doneFlag = false;
    std::vector<std::vector<char>> addresses;

    while (!doneFlag)
    {
        char v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        uint8_t normal, complemento;

		// checa se há dispositivos 1-Wire. Reset() retorna 0 se o(s) dispositivo(s) responderam, 1 se nenhum dispositivo respondeu
		if (onewire->reset())
		{
			printf("Nao existem escravos no barramento\n");
			return;
		}

        //onewire->reset();
        onewire->writeByte(SEARCH_ROM);

        int newLastDiscrepancy = 0;

        for (int x = 0; x < 64; x++)
        {
            normal = onewire->readBit();
            complemento = onewire->readBit();

            if (normal == 0 && complemento == 0)
            {
                if (x < lastDiscrepancy)
                    searchDirection = ((v[x / 8] >> (x % 8)) & 1);
                else if (x == lastDiscrepancy)
                    searchDirection = 1;
                else
                    searchDirection = 0;

                if (searchDirection == 0)
                    newLastDiscrepancy = x;
            }
            else if (normal == 0 && complemento == 1)
            {
                searchDirection = 0;
            }
            else if (normal == 1 && complemento == 0)
            {
                searchDirection = 1;
            }

            capturaBit(x, v, searchDirection);
            onewire->escreve_bit(searchDirection);
        }

        lastDiscrepancy = newLastDiscrepancy;

        if (lastDiscrepancy == 0)
            doneFlag = true;

        addresses.push_back(std::vector<char>(v, v + sizeof(v) / sizeof(char)));
    }

    int i = 1;
    for (auto &addr : addresses)
    {
        printf("Endereco sensor #%d completo: ", i);
        for (auto it = addr.rbegin(); it != addr.rend(); ++it)
        {
            printf("%d ", *it);
        }
        printf("\n");
        i++;
    }
}

void DS18B20::init(char v[])
{

	delay_ms(500);

	printf("INIT\n");
	if (onewire->reset() == 0)
		printf("Detectou escravo na linha\n");
	else
		printf("Nao detectou escravo\n");

	onewire->writeByte(READ_ROM);
	v[7] = onewire->readByte(); // family number
	printf("Codigo da Familia: %d\n", v[7]);

	for (uint8_t x = 6; x >= 1; x--)
		v[x] = onewire->readByte(); // serial

	printf("Numero de Serie  : %d %d %d %d %d %d\n", v[1], v[2], v[3], v[4], v[5], v[6]);

	v[0] = onewire->readByte();
	printf("CRC=             : %d\n", v[0]);

	delay_ms(1000);
}

// Usa o pino GPIO16 para fazer a comunicacao
DS18B20::DS18B20(gpio_num_t pino)
{
	DEBUG("DS18B20:Construtor\n");
	onewire = new ONEWIRE(pino);
}

char DS18B20::CRC(char end[])
{
	// calcula o CRC e retorna o resultado
	return 0;
}
float DS18B20::readTargetTemp(char vetor_64bits[])
{
	float temperatura;

	uint8_t a, b, inteira;
	float frac;

	onewire->reset();
	onewire->writeByte(MATCH_ROM);
	for (int x = 7; x >= 0; x--)
	{
		onewire->writeByte(vetor_64bits[x]);
	}
	onewire->writeByte(INICIA_CONVERSAO_TEMP);
	delay_ms(1000);
	onewire->reset();
	onewire->writeByte(MATCH_ROM);
	for (int x = 7; x >= 0; x--)
	{
		onewire->writeByte(vetor_64bits[x]);
	}
	onewire->writeByte(READ_TEMP_MEMORY);
	a = onewire->readByte();
	b = onewire->readByte();
	inteira = (b << 4) | (a >> 4);

	frac = ((a & 1) * (0.0625)) + (((a >> 1) & 1) * (0.125)) + (((a >> 2) & 1) * (0.25)) + (((a >> 3) & 1) * (0.5));

	temperatura = inteira + frac;
	return temperatura;
}
float DS18B20::readTemp(void)
{
	float temperatura;

	uint8_t a, b, inteira;
	float frac;

	onewire->reset();
	onewire->writeByte(SKIP_ROM);
	onewire->writeByte(INICIA_CONVERSAO_TEMP);
	delay_ms(1000);
	onewire->reset();
	onewire->writeByte(SKIP_ROM);
	onewire->writeByte(READ_TEMP_MEMORY);
	a = onewire->readByte();
	b = onewire->readByte();
	inteira = (b << 4) | (a >> 4);

	frac = ((a & 1) * (0.0625)) + (((a >> 1) & 1) * (0.125)) + (((a >> 2) & 1) * (0.25)) + (((a >> 3) & 1) * (0.5));

	temperatura = inteira + frac;
	return temperatura;
}

#endif
