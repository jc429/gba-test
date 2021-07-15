#ifndef GAMEDATA_H
#define GAMEDATA_H

void gamedata_init();
void gamedata_write_byte(unsigned char data, int mem_offset);
unsigned char gamedata_read_byte(int mem_offset);

#define SRAM_OFFSET_KEY		0x00
#define SRAM_LENGTH_KEY		0x08
#define SRAM_OFFSET_LEVEL	0x10

#endif //GAMEDATA_H