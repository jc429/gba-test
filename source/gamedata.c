#include <tonc.h>
#include "gamedata.h"
#include "debug.h"


// tonc SRAM isn't marked volatile and that causes issues lol
#define v_SRAM ((volatile unsigned char *)0x0E000000)
// (basically my answer to not knowing what SRAM first initializes with, but also can ensure data compatibility with version changes)
#define SRAM_KEY_COUNT	8
const byte sram_keys[SRAM_KEY_COUNT] = {0xEE, 0xAB, 0xB0, 0x54, 0x66, 0xAA, 0xFF, 0x02};	// "unique" keys || if first byte of SRAM is not set to these, erase everything 


void gamedata_clear_all_sram();
void gamedata_write_keys();
bool gamedata_check_keys();


void gamedata_init()
{
	if(DEBUG_CLEAR_SRAM || !gamedata_check_keys())
	{
		gamedata_clear_all_sram();
	}
	gamedata_write_keys();
}


void gamedata_clear_all_sram()
{
	for(int i = 0; i < SRAM_SIZE; i++)
	{
		v_SRAM[i] = 0;
	}
}



void gamedata_write_keys()
{
	for(int i = 0; i < SRAM_KEY_COUNT; i++)
	{
		gamedata_write_byte(sram_keys[i], SRAM_OFFSET_KEY + i);
		
	}
}

bool gamedata_check_keys()
{
	for(int i = 0; i < SRAM_KEY_COUNT; i++)
	{
		byte b = gamedata_read_byte(SRAM_OFFSET_KEY + i);
		if(b != sram_keys[i])
			return false;
	}
	return true;
}

///////////////////
/// Save & Load ///
///////////////////

// 0x0E00_0000 to 0x0FFF_FFFF
// byte r/w only


void gamedata_write_byte(byte data, int mem_offset)
{ 
	v_SRAM[mem_offset] = data;
}

byte gamedata_read_byte(int mem_offset)
{
	return (byte)v_SRAM[mem_offset];
}
