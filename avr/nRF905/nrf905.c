/**
 *    Copyright 2010 Daine Mamacos
 *
 *    This file is part of the nRF905 driver for AVR micro controllers.
 *
 *    The nRF905 driver is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License.
 *
 *    The nRF905 driver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <avr/io.h>
#include "SPIDefs.h"
#include "nRF905_conf.h"
#include "nrf905.h"

unsigned char initData[10] =
{
	0b01101100,
	0b00001100,
	0b01000100,
	0b00100000,
	0b00100000,

	//Devices Address
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000001,

	0b11011000,
};

void SPI_MasterInit()
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1 << DD_MOSI) | (1 << DD_SCK) | (1 << DD_SS);

	//SPI Master Mode
	SPCR = (1 << MSTR);

	//Set Speed (Fosc/4) (16 / 4 = 4Mhz)
	SPCR &= ~((1 << SPR1) | (1 << SPR0) | (1 << SPI2X));

	//set MSB
	SPCR &= ~(1 << DORD);

	//Enable SPI
	SPCR |= (1 << SPE);


}
void SPI_MasterStart()
{
	//Pull down the line for the correct slave
	NRF905_SS_PORT &= ~(1 << NRF905_SS);
}

void SPI_MasterEnd()
{
	//pull up the line again.
	NRF905_SS_PORT |= (1 << NRF905_SS);
}
unsigned char SPI_MasterTransmit(unsigned char cData)
{
	unsigned char data;

	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1 << SPIF)));

	data = SPDR;

	return data;
}


void nRF905SetFreq(uint16_t freqKKhz, uint8_t HFREQ_PLL, uint8_t power)
{
	uint16_t channel = 0;
	if(HFREQ_PLL == 0)
	{
		 channel = freqKKhz - 4224;
	}
	if(HFREQ_PLL == 1)
	{
		 channel = ((freqKKhz / 2) - 4224);
	}

	channel |= (power & 2) << 10 | (HFREQ_PLL & 1) << 9;

	SPI_MasterStart();
	SPI_MasterTransmit(0x80 | (uint8_t)((channel >> 8) & 0x0F));
	SPI_MasterTransmit((uint8_t)channel);
	SPI_MasterEnd();
}

void nRF905SetTxRxAddWidth(uint8_t txAddWidth, uint8_t rxAddWidth)
{
	uint8_t addLenWidth = (txAddWidth << 4) | (rxAddWidth & 0x0F);
	SPI_MasterStart();
	SPI_MasterTransmit(0x02);
	SPI_MasterTransmit(addLenWidth);
	SPI_MasterEnd();
}

void nRF905SetRxAddress(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t address4)
{
	SPI_MasterStart();
	SPI_MasterTransmit(0x05);
	SPI_MasterTransmit(address1);
	SPI_MasterTransmit(address2);
	SPI_MasterTransmit(address3);
	SPI_MasterTransmit(address4);
	SPI_MasterEnd();
}

void nRF905SetRxPayloadWidth(uint8_t width)
{
	//a small safety check to make sure, our payload isn't bigger than 32 bytes
	if(width > 32) width = 32;
	SPI_MasterStart();
	SPI_MasterTransmit(0x03);
	SPI_MasterTransmit(width);
	SPI_MasterEnd();
}

void nRF905SetTxPayloadWidth(uint8_t width)
{
	//a small safety check to make sure, our payload isn't bigger than 32 bytes
	if(width > 32) width = 32;
	SPI_MasterStart();
	SPI_MasterTransmit(0x04);
	SPI_MasterTransmit(width);
	SPI_MasterEnd();
}

void nRF905SetTxAddress(uint8_t address1, uint8_t address2, uint8_t address3, uint8_t address4)
{
	SPI_MasterStart();
	SPI_MasterTransmit(0x22);
	SPI_MasterTransmit(address1);
	SPI_MasterTransmit(address2);
	SPI_MasterTransmit(address3);
	SPI_MasterTransmit(address4);
	SPI_MasterEnd();
}

void nRF905GetTxAddress(uint8_t *buffer)
{
	SPI_MasterStart();
	SPI_MasterTransmit(0x23);
	buffer[0] = SPI_MasterTransmit(0x00);
	buffer[1] = SPI_MasterTransmit(0x00);
	buffer[2] = SPI_MasterTransmit(0x00);
	buffer[3] = SPI_MasterTransmit(0x00);
	SPI_MasterEnd();
}

void nRF905SetTxPayload(char *payload, uint8_t payloadWidth)
{

	SPI_MasterStart();
	SPI_MasterTransmit(0x20);
	for(uint8_t i = 0; i < payloadWidth; ++ i)
	{
		SPI_MasterTransmit(payload[i]);
	}
	SPI_MasterEnd();

}

void nRF905GetTxPayload(char *payload, uint8_t payloadWidth)
{
	SPI_MasterStart();
	SPI_MasterTransmit(0x21);
	for(uint8_t i = 0; i < payloadWidth; ++ i)
	{
		payload[i] = SPI_MasterTransmit(0x00);
	}
	SPI_MasterEnd();
}

void nRF905SendPacket()
{
	NRF905_CONTRL_PORT |= (1 << NRF905_TXEN) | (1 << NRF905_TRX_CE);
	//once we start sending the packet, we wait for the DR pin to go high
	//indicating that the data is finished sending
	while(!(NRF905_DR_PORT & (1 << NRF905_DR)));
	NRF905_CONTRL_PORT &= ~((1 << NRF905_TXEN) | (1 << NRF905_TRX_CE));
}

void nRF905DeviceSleep()
{
	NRF905_CONTRL_PORT &= ~(1 << NRF905_PWR_UP);
}

void nRF905GetConfig(uint8_t from, uint8_t count, char *buffer)
{
		SPI_MasterStart();
		SPI_MasterTransmit(0x10 | (from & 0x0F));
		for(int i = 0; i < count; i ++)
		{
			buffer[i] = SPI_MasterTransmit(0x00);
		}
		SPI_MasterEnd();
}

void nRF905SetConfig(uint8_t from, uint8_t count, char *buffer)
{
	SPI_MasterStart();
	SPI_MasterTransmit(0x00 | (from & 0x0F));
	for(int i = 0; i < count; i ++)
	{
		//send dummy bytes to read off the data
		SPI_MasterTransmit(buffer[i]);
	}
	SPI_MasterEnd();

}

void nRF905Init()
{
	//nRF905 power down
	NRF905_CONTRL_DDR = (1 << NRF905_TXEN) | (1 << NRF905_TRX_CE) | (1 << NRF905_PWR_UP);
	NRF905_CONTRL_PORT &= ~((1 << NRF905_TXEN) | (1 << NRF905_TRX_CE));
	NRF905_CONTRL_PORT |= (1 << NRF905_PWR_UP);

	//NRF905_DR_DDR &= ~((1 << NRF905_AM) | (1 << NRF905_CD) | (1 << NRF905_DR));

	//read the config data
	nRF905SetConfig(0, 10, initData);
	SPI_MasterEnd();
}

void nRF905EnableRecv()
{
	NRF905_CONTRL_PORT &= ~(1 << NRF905_TXEN);
	NRF905_CONTRL_PORT |= 1 << NRF905_TRX_CE;
}
void nRF905DisableRecv()
{
	NRF905_CONTRL_PORT &= ~(1 << NRF905_TXEN);
	NRF905_CONTRL_PORT &= ~(1 << NRF905_TRX_CE);
}



void nRF905GetRxPayload(char *payload, uint8_t payloadWidth)
{
	SPI_MasterStart();
	SPI_MasterTransmit(0x24);
	for(uint8_t i = 0; i < payloadWidth; ++ i)
	{
		payload[i] = SPI_MasterTransmit(0x00);
	}
	SPI_MasterEnd();
}

void nRF905RecvPacket(char *buffer, uint8_t payloadWidth, uint8_t stayInRecvMode)
{
	//we only want to continue while AM is high (in case of a CRC failure)
	while(NRF905_DR_PORT & (1 << NRF905_AM))
	{
		//our data is ready for clocking out the RX buffer
		if(NRF905_DR_PORT & (1 << NRF905_DR))
		{
			if(!stayInRecvMode) NRF905_CONTRL_PORT &= ~(1 << NRF905_TRX_CE);
			nRF905GetRxPayload(buffer, payloadWidth);
		}
	}
}

void nRF905RecvPacketWODL(char *buffer, uint8_t payloadWidth, uint8_t stayInRecvMode)
{
	//we only want to continue while AM is high (in case of a CRC failure)
	while(AM_IN_STATUS_REG(nRF905GetStatusReg()))
	{
		//our data is ready for clocking out the RX buffer
		if(DR_IN_STATUS_REG(nRF905GetStatusReg()))
		{
			if(!stayInRecvMode) NRF905_CONTRL_PORT &= ~(1 << NRF905_TRX_CE);
			nRF905GetRxPayload(buffer, payloadWidth);
		}
	}
}

void nRF905SetReduceRxPwr(uint8_t rxPwrRed)
{
	char byte1Config;
	nRF905GetConfig(1, 1, &byte1Config);
	if(rxPwrRed)
	{
		byte1Config |= 0x10;
	}
	else
	{
		byte1Config &= ~(0x10);
	}
	nRF905SetConfig(1, 1, &byte1Config);
}

void nRF905SetAutoRetransmit(uint8_t autoRetransmit)
{
	char byte1Config;
	nRF905GetConfig(1, 1, &byte1Config);
	if(autoRetransmit)
	{
		byte1Config |= 0x20;
	}
	else
	{
		byte1Config &= ~(0x20);
	}
	nRF905SetConfig(1, 1, &byte1Config);
}

void nRF905SetOuputClock(uint8_t outClockFreq, uint8_t enable)
{
	outClockFreq = (outClockFreq > 3) ? 3 : outClockFreq;
	enable = (enable) ? 1 : 0;
	char byte9Config;
	nRF905GetConfig(9, 1, &byte9Config);
	byte9Config = (byte9Config & 0b11111000) | outClockFreq | enable << 2;
	nRF905SetConfig(9, 1, &byte9Config);
}

void nRF905SetCRC(uint8_t CRCMode, uint8_t enable)
{
	CRCMode = (CRCMode) ? 1 : 0;
	enable = (enable) ? 1 : 0;
	char byte9Config;
	nRF905GetConfig(9, 1, &byte9Config);
	byte9Config = (byte9Config & 0b00111111) | CRCMode << 7 | enable << 6;
	nRF905SetConfig(9, 1, &byte9Config);
}


void nRF905SetOscFreq(uint8_t freq)
{
	freq = (freq > 4) ? 4 : freq;
	char byte9Config;
	nRF905GetConfig(9, 1, &byte9Config);
	byte9Config = (byte9Config & 0b11000111) | freq << 3;
	nRF905SetConfig(9, 1, &byte9Config);
}

uint8_t nRF905GetStatusReg()
{
	SPI_MasterStart();
	uint8_t statusReg = SPI_MasterTransmit(0x00);
	SPI_MasterEnd();
	return statusReg;
}