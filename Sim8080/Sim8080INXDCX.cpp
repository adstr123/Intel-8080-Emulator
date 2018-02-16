// Sim8080.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "13014222"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.


#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   500



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];




//////////////////////////
// Intel 8080 Registers //
//////////////////////////

#define REGISTER_B	0
#define REGISTER_C	1
#define REGISTER_D	2
#define REGISTER_E	3
#define REGISTER_H	4
#define REGISTER_L	5
#define REGISTER_M	6
#define REGISTER_A	7

#define FLAG_S	0x80
#define FLAG_Z	0x40
#define FLAG_A	0x10
#define FLAG_P	0x04
#define FLAG_C	0x01

BYTE Registers[8];
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;

////////////
// Memory //
////////////

#define K_1			1024
#define MEMORY_SIZE	K_1

BYTE Memory[MEMORY_SIZE];

///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;

///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][12] =
{
	"NOP        ", 
	"LXI B,data ", 
	"STAX B     ", 
	"INX B      ", 
	"INR B      ", 
	"DCR B      ", 
	"MVI B,data ", 
	"RLC        ", 
	".BYTE 0x08 ", 
	"DAD B      ", 
	"LDAX B     ", 
	"DCX B      ", 
	"INR C      ", 
	"DCR C      ", 
	"MVI C,data ", 
	"RRC        ", 
	".BYTE 0x10 ", 
	"LXI D,data ", 
	"STAX D     ", 
	"INX D      ", 
	"INR D      ", 
	"DCR D      ", 
	"MVI D,data ", 
	"RAL        ", 
	".BYTE 0x18 ", 
	"DAD D      ", 
	"LDAX D     ", 
	"DCX D      ", 
	"INR E      ", 
	"DCR E      ", 
	"MVI E,data ", 
	"RAR        ", 
	"RIM        ", 
	"LXI H,data ", 
	"SHLD       ", 
	"INX H      ", 
	"INR H      ", 
	"DCR H      ", 
	"MVI H,data ", 
	"DAA        ", 
	".BYTE 0x28 ", 
	"DAD H      ", 
	"LHLD       ", 
	"DCX H      ", 
	"INR L      ", 
	"DCR L      ", 
	"MVI L,data ", 
	"CMA        ", 
	"SIM        ", 
	"LXI SP,data", 
	"STA        ", 
	"INX SP     ", 
	"INR M      ", 
	"DCR M      ", 
	"MVI M,data ", 
	"STC        ", 
	".BYTE 0x38 ", 
	"DAD SP     ", 
	"LDA        ", 
	"DCX SP     ", 
	"INR A      ", 
	"DCR A      ", 
	"MVI A,data ", 
	"CMC        ", 
	"MOV B,B    ", 
	"MOV B,C    ", 
	"MOV B,D    ", 
	"MOV B,E    ", 
	"MOV B,H    ", 
	"MOV B,L    ", 
	"MOV B,M    ", 
	"MOV B,A    ", 
	"MOV C,B    ", 
	"MOV C,C    ", 
	"MOV C,D    ", 
	"MOV C,E    ", 
	"MOV C,H    ", 
	"MOV C,L    ", 
	"MOV C,M    ", 
	"MOV C,A    ", 
	"MOV D,B    ", 
	"MOV D,C    ", 
	"MOV D,D    ", 
	"MOV D,E    ", 
	"MOV D,H    ", 
	"MOV D,L    ", 
	"MOV D,M    ", 
	"MOV D,A    ", 
	"MOV E,B    ", 
	"MOV E,C    ", 
	"MOV E,D    ", 
	"MOV E,E    ", 
	"MOV E,H    ", 
	"MOV E,L    ", 
	"MOV E,M    ", 
	"MOV E,A    ", 
	"MOV H,B    ", 
	"MOV H,C    ", 
	"MOV H,D    ", 
	"MOV H,E    ", 
	"MOV H,H    ", 
	"MOV H,L    ", 
	"MOV H,M    ", 
	"MOV H,A    ", 
	"MOV L,B    ", 
	"MOV L,C    ", 
	"MOV L,D    ", 
	"MOV L,E    ", 
	"MOV L,H    ", 
	"MOV L,L    ", 
	"MOV L,M    ", 
	"MOV L,A    ", 
	"MOV M,B    ", 
	"MOV M,C    ", 
	"MOV M,D    ", 
	"MOV M,E    ", 
	"MOV M,H    ", 
	"MOV M,L    ", 
	"HLT        ", 
	"MOV M,A    ", 
	"MOV A,B    ", 
	"MOV A,C    ", 
	"MOV A,D    ", 
	"MOV A,E    ", 
	"MOV A,H    ", 
	"MOV A,L    ", 
	"MOV A,M    ", 
	"MOV A,A    ", 
	"ADD B      ", 
	"ADD C      ", 
	"ADD D      ", 
	"ADD E      ", 
	"ADD H      ", 
	"ADD L      ", 
	"ADD M      ", 
	"ADD A      ", 
	"ADC B      ", 
	"ADC C      ", 
	"ADC D      ", 
	"ADC E      ", 
	"ADC H      ", 
	"ADC L      ", 
	"ADC M      ", 
	"ADC A      ", 
	"SUB B      ", 
	"SUB C      ", 
	"SUB D      ", 
	"SUB E      ", 
	"SUB H      ", 
	"SUB L      ", 
	"SUB M      ", 
	"SUB A      ", 
	"SBB B      ", 
	"SBB C      ", 
	"SBB D      ", 
	"SBB E      ", 
	"SBB H      ", 
	"SBB L      ", 
	"SBB M      ", 
	"SBB A      ", 
	"ANA B      ", 
	"ANA C      ", 
	"ANA D      ", 
	"ANA E      ", 
	"ANA H      ", 
	"ANA L      ", 
	"ANA M      ", 
	"ANA A      ", 
	"XRA B      ", 
	"XRA C      ", 
	"XRA D      ", 
	"XRA E      ", 
	"XRA H      ", 
	"XRA L      ", 
	"XRA M      ", 
	"XRA A      ", 
	"ORA B      ", 
	"ORA C      ", 
	"ORA D      ", 
	"ORA E      ", 
	"ORA H      ", 
	"ORA L      ", 
	"ORA M      ", 
	"ORA A      ", 
	"CMP B      ", 
	"CMP C      ", 
	"CMP D      ", 
	"CMP E      ", 
	"CMP H      ", 
	"CMP L      ", 
	"CMP M      ", 
	"CMP A      ", 
	"RNZ        ", 
	"POP B      ", 
	"JNZ        ", 
	"JMP        ", 
	"CNZ        ", 
	"PUSH B     ", 
	"ADI        ", 
	"RST 0      ", 
	"RZ         ", 
	"RET        ", 
	"JZ         ", 
	".BYTE 0xCB ", 
	"CZ         ", 
	"CALL       ", 
	"ACI        ", 
	"RST 1      ", 
	"RNC        ", 
	"POP D      ", 
	"JNC        ", 
	"OUT        ", 
	"CNC        ", 
	"PUSH D     ", 
	"SUI        ", 
	"RST 2      ", 
	"RC         ", 
	".BYTE 0xD9 ", 
	"JC         ", 
	"IN         ", 
	"CC         ", 
	".BYTE 0xDD ", 
	"SBI        ", 
	"RST 3      ", 
	"RPO        ", 
	"POP H      ", 
	"JPO        ", 
	"XTHL       ", 
	"CPO        ", 
	"PUSH H     ", 
	"ANI        ", 
	"RST 4      ", 
	"RPE        ", 
	"PCHL       ", 
	"JPE        ", 
	"XCHG       ", 
	"CPE        ", 
	".BYTE 0xED ", 
	"XRI        ", 
	"RST 5      ", 
	"RP         ", 
	"POP PSW    ", 
	"JP         ", 
	"DI         ", 
	"CP         ", 
	"PUSH PSW   ", 
	"ORI        ", 
	"RST 6      ", 
	"RM         ", 
	"SPHL       ", 
	"JM         ", 
	"EI         ", 
	"CM         ", 
	".BYTE 0xFD ", 
	"CPI        ", 
	"RST 7      "
};







////////////////////////////////////////////////////////////////////////////////
//                      Intel 8080 Simulator/Emulator (Start)                 //
////////////////////////////////////////////////////////////////////////////////


BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}


// Add any instruction implementing routines here...



void set_flags(BYTE inReg)
{
	BYTE regA;
	int  bit_set_count;

	regA = inReg;

	if (regA == 0)			// set to zero (Zero flag)
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}

	if ((regA & 0x80) != 0)		// msbit set (Sign flag)
	{
		Flags = Flags | FLAG_S;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_S);
	}

	bit_set_count = 0; // Parity flag. Add up all bits that are set to 1
	if ((regA & 0x01) != 0)
		bit_set_count++;
	if ((regA & 0x02) != 0)
		bit_set_count++;
	if ((regA & 0x04) != 0)
		bit_set_count++;
	if ((regA & 0x08) != 0)
		bit_set_count++;
	if ((regA & 0x10) != 0)
		bit_set_count++;
	if ((regA & 0x20) != 0)
		bit_set_count++;
	if ((regA & 0x40) != 0)
		bit_set_count++;
	if ((regA & 0x80) != 0)
		bit_set_count++;

	switch (bit_set_count)
	{
	case 0:
	case 2:
	case 4:
	case 6:
	case 8:
		Flags = Flags | FLAG_P; // Set flag
		break;

	default:
		Flags = Flags & (0xFF - FLAG_P); // Clear flag
		break;
	}
}



void block_00_instructions(BYTE opcode)
{
	BYTE ls3bits;
	BYTE destination;
	WORD address;
	BYTE lb;
	BYTE hb;
	long temp;
	long hl;
	WORD temp_word;
	BYTE temp_byte;

	ls3bits = opcode & 0x07;

	switch (ls3bits)
	{
	case 0x00:	// NOP
		break;

	case 0x01:	// LXI and DAD
		if ((opcode & 0x08) == 0) // Check if bit 3 is 0 or 1. If 0 then LXI
		{
			lb = fetch(); // Read in the two immediate bytes
			hb = fetch();

			switch (opcode)
			{
			case 0x01:	// LXI BC
						Registers[REGISTER_B] = hb;
						Registers[REGISTER_C] = lb;

			break;
			case 0x11: // LXI DE
						Registers[REGISTER_D] = hb;
						Registers[REGISTER_E] = lb;
			break;
			case 0x21: // LXI HL
						Registers[REGISTER_H] = hb;
						Registers[REGISTER_L] = lb;
			break;
			default: // LXI SP
						StackPointer = ((WORD)hb << 8) + (WORD)lb;
						break;
			}
		}
		else // DAD
		{
			lb = fetch();
			hb - fetch();

			switch (opcode)
			{
			case 0x09: // DAD B (Add BC to HL)
				// ADD BC to HL
				// Store result in H and L
				// Set carry flag if appropriate
				break;

			case 0x19: // DAD D (Add DE to HL)
				break;

			case 0x29: // DAD H (Add HL to HL)
				break;

			case 0x39: // DAD SP (Add stack pointer to HL)
				break;
			}
		}
		break;

	case 0x02:	// Load and Store instructions
		switch (opcode)
		{

		case 0x0A: // LDAX B (Load memory BC to A)
			// Load accumulator with copy of memory location addressed by BC
			break;

		case 0x1A: // LDAX D (Load memory DE to A)
			// Load accumulator with copy of memory location addressed by DE
			break;

		case 0x2A: // LHLD

			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if ((address >= 0) && (address < (MEMORY_SIZE-1)))
			{
					Registers[REGISTER_L] = Memory[address];
					Registers[REGISTER_H] = Memory[address+1];
			}
			else
			{
					Registers[REGISTER_L] = 0;
					Registers[REGISTER_H] = 0;
			}

			break;

		case 0x02: // STAX BC
			address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];
			if ((address >= 0) && (address < MEMORY_SIZE))
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0x3A: // LDA

			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;

			if ((address >= 0) && (address < MEMORY_SIZE))
			{
					Registers[REGISTER_A] = Memory[address];
			}
			else
			{
					Registers[REGISTER_A] = 0;
			}
			break;

		case 0x12: // STAX DE
			address = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];
			if ((address >= 0) && (address < MEMORY_SIZE))
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0x22: // SHLD (Store H and L direct)
			// Fetch high & low bytes
			// Store contents of H and L in memory location specified in bytes 2 and 3 of STA instruction
			break;

		case 0x32: // STA (Store accumulator direct)
			// Fetch high & low bytes
			// Store contents of A in memory location specified in bytes 2 and 3 of STA instruction
			break;

		default:
			break;
		}
		break;

	case 0x03:	// INX B (Increment BC) and DCX
		temp = (long)Registers[REGISTER_B] << 8 + (long)Registers[REGISTER_C];
		Registers[REGISTER_B] = (BYTE)((temp >> 8) & 0xFF);
		Registers[REGISTER_C] = (BYTE)(temp & 0xFF);
		break;

	case 0x04:	// INR B (Increment B)

		destination = (opcode >> 3) & 0x07; // Find out which is the destination register (Which register we need to increment)

		if (destination == REGISTER_M) // Check if it is register M
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // Construct address from registers H & L

			if ((address >= 0) && (address < MEMORY_SIZE)) // Check if address is valid
			{
				Memory[address]++; // Increment memory
				set_flags(Memory[address]);
			}
		}
		else
		{
			Registers[destination]++; // Register incremented is either A, B, C, D, E, H or L
			set_flags(Registers[destination]);
		} 
		break;

	case 0x0C: // INR C (Increment C)
		break;

	case 0x14: // INC D (Increment D)
		break;

	case 0x1C: // INC E (Increment E)
		break;

	case 0x24: // INC H (Increment H)
		break;

	case 0x2C: // INC L (Increment L)
		break;

	case 0x34: // INC M (Increment memory (HL))
		break;

	case 0x3C: // INC A (Increment A)
		break;

	case 0x05:	// DCR B (Decrement B)
		
		destination = (opcode >> 3) & 0x07; // Find out which is the destination register (Which register we need to increment)

		if (destination == REGISTER_M) // Check if it is register M
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // Construct address from registers H & L

			if ((address >= 0) && (address < MEMORY_SIZE)) // Check if address is valid
			{
				Memory[address]--; // Decrement memory
				set_flags(Memory[address]);
			}
		}
		else
		{
			Registers[destination]--; // Register decremented is either A, B, C, D, E, H or L
			set_flags(Registers[destination]);
		} 
		break;

	case 0x0D: // DCR C (Decrement C)
		break;

	case 0x15: // DCR D (Decrement D)
		break;

	case 0x1D: // DCR E (Decrement E)
		break;

	case 0x25: // DCR H (Decrement H)
		break;

	case 0x2D: // DCR L (Decrement L)
		break;

	case 0x35: // DCR M (Decrement memory (HL))
		break;

	case 0x3D: // DCR A (Decrement A)
		break;

	case 0x0B: // DCX B (Decrement BC)
		temp = (long)Registers[REGISTER_B] << 8 + (long)Registers[REGISTER_C];
		Registers[REGISTER_B] = (BYTE)((temp >> 8) & 0xFF);
		Registers[REGISTER_C] = (BYTE)(temp & 0xFF);
		break;

	case 0x1B: // DCX D (Decrement DE)
		temp = (long)Registers[REGISTER_D] << 8 + (long)Registers[REGISTER_E];
		Registers[REGISTER_D] = (BYTE)((temp >> 8) & 0xFF);
		Registers[REGISTER_E] = (BYTE)(temp & 0xFF);
		break;

	case 0x2B: // DCX H (Decrement HL)
		temp = (long)Registers[REGISTER_H] << 8 + (long)Registers[REGISTER_L];
		Registers[REGISTER_H] = (BYTE)((temp >> 8) & 0xFF);
		Registers[REGISTER_L] = (BYTE)(temp & 0xFF);
		break;

	case 0x3B: // DCX SP (Decrement stack pointer)
		StackPointer--;
		break;

	case 0x06:	// MVI
		destination = (opcode >> 3) & 0x07;
		Registers[destination] = fetch();
		if (destination == REGISTER_M) 
		{ 
				address = ((WORD)Registers[REGISTER_H] << 8) + 
								 (WORD)Registers[REGISTER_L]; 
 
				if ((address >= 0) && (address < MEMORY_SIZE)) 
				{ 
						Memory[address] = Registers[REGISTER_M]; 
				} 
		}
		break;

	case 0x13: // INX D (Increment DE)
		temp = (long)Registers[REGISTER_D] << 8 + (long)Registers[REGISTER_E];
		Registers[REGISTER_D] = (BYTE)((temp >> 8) & 0xFF);
		Registers[REGISTER_E] = (BYTE)(temp & 0xFF);
		break;

	case 0x23: // INX H (Increment HL
		temp = (long)Registers[REGISTER_H] << 8 + (long)Registers[REGISTER_L];
		Registers[REGISTER_H] = (BYTE)((temp >> 8) & 0xFF);
		Registers[REGISTER_L] = (BYTE)(temp & 0xFF);
		break;

	case 0x33: // INX SP (Increment SP)
		StackPointer++;
		break;

	default:	// Rotates,DAA and carry instructions
		break;
	}
}



void MOV_and_HLT_instructions(BYTE opcode)
{
	BYTE source;
	BYTE destination;
	BYTE temp;
	WORD address;

	if (opcode == 0x76)
	{
		halt = true;  // HLT - halt microprocessor
	}
	else
	{
		source = opcode & 0x07;
		destination = (opcode >> 3) & 0x07;

		if (source == REGISTER_M) // Check if we are using register indirect
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // If we are, create address from registers H & L (H means high byte, l means low byte

			if ((address >= 0) && (address < MEMORY_SIZE)) // Check it is a valid memory address
			{
				Registers[REGISTER_M] = Memory[address]; // If all is well write contents to register M
			}
			else
			{
				Registers[REGISTER_M] = 0;
			}
		}

		Registers[destination] = Registers[source]; // Code for copying one register to another

		if (destination == REGISTER_M) // Check if destination is Register M
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // If so, create address from registers H & L

			if ((address >= 0) && (address < MEMORY_SIZE)) // Check it is a valid memory address
			{
				Memory[address] = Registers[REGISTER_M]; // If all is well, write to address
			}
		}
	}
}



void block_10_instructions(BYTE opcode)
{
	BYTE  source;
	BYTE  instruction_type;
	short temp = 0;
	WORD  address;

	instruction_type = (opcode >> 3) & 0x07;
	source = opcode & 0x07;



	if (source == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // If we are, create address from registers H & L (H means high byte, l means low byte

			if ((address >= 0) && (address < MEMORY_SIZE)) // Check it is a valid memory address
			{
				Registers[REGISTER_M] = Memory[address]; // If all is well write contents to register M
			}
			else
			{
				Registers[REGISTER_M] = 0;
			}
		}



	switch(opcode)
	{

	case 0x80: // ADD B (Add B to A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);

		break;

	case 0x81: // ADD C (Add C to A)
			
		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x82: // ADD D (Add D to A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x83: // ADD E (Add E to A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x84: // ADD H (Add H to A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x85: // ADD L (Add L to A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x86: // ADD M (Add memory (HL) to A)
		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x87: // ADD A (Add A to A)
		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] + temp; // Actually do the add
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;


	case 0x88: // ADC B (Add B to A with carry)
		// Add B to A plus the setting of the carry flag
		// Store result in A
		// Set carry flag according to result
		break;

	case 0x89: // ADC C (Add C to A with carry)
		break;

	case 0x8A: // ADC D (Add D to A with carry)
		break;

	case 0x8B: // ADC E (Add E to A with carry)
		break;

	case 0x8C: // ADC H (Add H to A with carry)
		break;

	case 0x8D: // ADC L (Add L to A with carry)
		break;

	case 0x8E: // ADC M (Add memory (HL) to A with carry)
		break;

	case 0x8F: // ADC A (Add A to A with carry)
		break;


	case 0x90: // SUB B (Subtract B from A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x91: // SUB C (Subtract C from A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x92: // SUB D (Subtract D from A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x93: // SUB E (Subtract E from A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x94: // SUB H (Subtract H to A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x95: // SUB L (Subtract L to a)
		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0x96: // SUB M (Subtract memory (HL) from A)

		temp = (WORD)Registers[source];
		temp = (WORD)Registers[REGISTER_A] - temp; // Actually do the subtract
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;



	case 0x98: // SBB B (Subtract B from A with carry)
		// Subtracy B from A plus the setting of the carry flag
		// Store result in A
		// Set carry flag according to result
		break;

	case 0x99: // SBB C (Subtract C from A with carry)
		break;

	case 0x9A: // SBB D (Subtract D from A with carry)
		break;

	case 0x9B: // SBB E (Subtract E from A with carry)
		break;

	case 0x9C: // SBB H (Subtract H from A with carry)
		break;

	case 0x9D: // SBB L (Subtract L from A with carry)
		break;

	case 0x9E: // SBB M (Subtract memory (SP) from A with carry)
		break;

	case 0x9F: // SBB A (Subtract A from A with carry)
		break;



	case 0xA0: // ANA B
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA1: // ANA C
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA2: // ANA D
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA3: // ANA E
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA4: // ANA H
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA5: // ANA L
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA6: // ANA M
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xA7: // ANA A
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] & temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;


	case 0xA8: // XRA B (xor A with B)
		// Perform exclusive OR between B and A
		// Place result in accumulator
		break;

	case 0xA9: // XRA C (xor A with C)
		break;

	case 0xAA: // XRA D (xor A with D)
		break;

	case 0xAB: // XRA E (xor A with E)
		break;

	case 0xAC: // XRA H (xor A with H)
		break;

	case 0xAD: // XRA L (xor A with L)
		break;

	case 0xAE: // XRA M (xor A with memory (HL))
		break;

	case 0xAF: // XRA A (xor A with A)
		break;



	case 0xB0: // ORA B
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB1: // ORA C
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB2: // ORA D
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB3: // ORA E
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB4: // ORA H
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB5: // ORA L
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB6: // ORA M
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;

	case 0xB7: // ORA A
		temp = (WORD)Registers[source]; 
		temp = (WORD)Registers[REGISTER_A] | temp;
		Registers[REGISTER_A] = (BYTE)(temp & 0xFF);
		break;



	case 0xB8: // CMP B (Compare A with B)
		// Compare B and A
		// Zero flag indicates equality
		// No carry indicates A > B
		// Carry indicates A < B

	case 0xB9: // CMP C (Compare A with C)
		break;

	case 0xBA: // CMP D (Compare A with D)
		break;

	case 0xBB: // CMP E (Compare A with E)
		break;

	case 0xBC: // CMP H (Compare A with H)
		break;

	case 0xBD: // CMP L (Compare A with L)
		break;

	case 0xBE: // CMP M (Compare A with memory (HL)
		break;

	case 0xBF: // CMP A (Compare A with A)
		break;

	}



	if ((temp & 0x100) != 0)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		set_flags((BYTE)(temp & 0xFF));
}



void block_11_instructions(BYTE opcode)
{
	WORD address;
	BYTE lb;
	BYTE hb;
	BYTE temp;
	WORD temp_word;

	switch (opcode)
	{
	case 0xC0:	// RNZ (Return if not zero)
		// Test setting of FLAG_Z
		// if 0 (contents of A are other than zero), pop 2 bytes off stack and place them in PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xC1:	// POP B
			if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2))) // Check address held in SP is valid. Opposite of PUSH, removing 2 bytes.
		{
			Registers[REGISTER_C] = Memory[StackPointer];		// Pop C then B, opposite order to PUSH
			StackPointer++;										// Increment
			Registers[REGISTER_B] = Memory[StackPointer];
			StackPointer++;
		}
		break;

	case 0xC2:	// JNZ  (Jump if not zero)
		// Test setting of FLAG_Z
		// If 1 (contents of accumulator are other than 0), program execution resumes at address specified in JNZ instruction
		// else program continues with next sequential instruction
		break;

	case 0xC3:	// JMP    

		lb = fetch(); // Get address
		hb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		ProgramCounter = address; // Write it to the PC

		break;

	case 0xC4:	// CNZ  (Call if not zero)
		// Test setting of FLAG_Z
		// if 0 (contents of A are other than 0), push contents of PC onto stack
		// Jump to address specified in instruction's second and third bytes
		// else continue with next sequential instruction
		break;

	case 0xC5:	// PUSH B
		if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1))) // Check address held in SP is valid. Require 2 bytes, make sure there are enough free bytes below.
		{
			StackPointer--;										// Decrement SP
			Memory[StackPointer] = Registers[REGISTER_B];		// Copy data
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_C];
		}
		break;

	case 0xC6:	// ADI
		lb = fetch(); // fetch
		temp_word = (WORD)Registers[REGISTER_A]+(WORD)lb; // Could be 9 bits, therefore can use 16 bit WORDs when doing addition. Takes A, adds WORD value of lb. WORD defined as 16 bit variable.
		if (temp_word >= 0x100)							  // Carry flag set to 1 if addition creates 9 bit number. We need to test for this:
		{								                  
			Flags = Flags | FLAG_C; // Set carry flag using bitwise OR
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);				   // Clear carry flag using bitwise AND
		}													   // Now deal with parity and sign flags in void set_flags(BYTE inReg).
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);	   // Copy result back into Accumulator, but take into account result is currently 16-bit number!
		set_flags(Registers[REGISTER_A]);
		break; 
		

	case 0xC7:	// RST 0   
		break;

	case 0xC8:	// RZ (Return if zero)
		// Test setting of FLAG_Z
		// if 1 (contents of A are zero), pop 2 bytes off stack and place them in PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xC9:	// RET
		if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2))) // Check if the SP contains valid address
		{
			lb = Memory[StackPointer];								   // Pop address off the stack
			StackPointer++;
			hb = Memory[StackPointer];
			StackPointer++;
			ProgramCounter = ((WORD)hb << 8) + (WORD)lb;			   // Set up new PC address
		}
		break;

	case 0xCA:	// JZ  (Jump if zero)
		// Test setting of FLAG_Z
		// If 0 (contents of accumulator are 0), program execution resumes at address specified in JNZ instruction
		// else program continues with next sequential instruction
		break;

	case 0xCB:	// .BYTE 0xCB  
		break;

	case 0xCC:	// CZ (Call if zero)
		// Test setting of FLAG_Z
		// if 1 (contents of A are than 0), push contents of PC onto stack
		// Jump to address specified in instruction's second and third bytes
		// else continue with next sequential instruction
		break;

	case 0xCD:	// CALL
		lb = fetch();
		hb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;								// Get address of the function we are going to call (Same in STA & LDA)
		if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))		// Validate address in SP
		{																	// Push pragram counter onto stack
			StackPointer--;													// Decrement SP
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);	// 0xFF gives a whole byte of 1s
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			ProgramCounter = address;										// Set PC to its new value
		}
		break;

	case 0xCE:	// ACI         
		lb = fetch(); // fetch
		temp_word = (WORD)Registers[REGISTER_A]+(WORD)lb; // Could be 9 bits, therefore can use 16 bit WORDs when doing addition. Takes A, adds WORD value of lb. WORD defined as 16 bit variable.
		if ((Flags & FLAG_C) != 0)						  // Check if carry bit is set, so mask Flags with FLAG_C and see if result is non-zero.
		{
			temp_word++;
		}
		if (temp_word >= 0x100)							  // Carry flag set to 1 if addition creates 9 bit number. We need to test for this:
		{								                  
			Flags = Flags | FLAG_C; // Set carry flag using bitwise OR
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);				   // Clear carry flag using bitwise AND
		}													   // Now deal with parity and sign flags in void set_flags(BYTE inReg).
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);	   // Copy result back into Accumulator, but take into account result is currently 16-bit number!
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xCF:	// RST 1       
		break;

	case 0xD0:	// RNC (Return if no carry)
		// Test setting of FLAG_C
		// if 0 (has been no carry), pop 2 bytes off stack and place them in PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xD1:	// POP D
			if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2))) // Check address held in SP is valid. Opposite of PUSH, removing 2 bytes.
		{
			Registers[REGISTER_E] = Memory[StackPointer];		
			StackPointer++;										// Increment
			Registers[REGISTER_D] = Memory[StackPointer];
			StackPointer++;
		}
		break;

	case 0xD2:	// JNC (Jump if no carry)
		// Test setting of FLAG_C
		// If 0 (no carry), program execution resumes at address specified in JNC instruction
		// else program continues with next sequential instruction
		break;

	case 0xD3:	// OUT         
		break;

	case 0xD4:	// CNC   (Call if no carry)
		// Test setting of FLAG_C
		// if 0, push contents of PC onto stack
		// Jump to address specified in instruction's second and third bytes
		// else continue with next sequential instruction
		break;

	case 0xD5:	// PUSH D
			if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1))) // Check address held in SP is valid. Require 2 bytes, make sure there are enough free bytes below.
		{
			StackPointer--;										// Decrement SP
			Memory[StackPointer] = Registers[REGISTER_D];		// Copy data
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_E];
		}
		break;

	case 0xD6:	// SUI
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xD7:	// RST 2       
		break;

	case 0xD8:	// RC (Return if carry)
		// Test setting of FLAG_C
		// if 1 (has been a carry), pop 2 bytes off stack and place them in PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xD9:	// .BYTE 0xD9  
		break;

	case 0xDA:	// JC (Jump if carry)
		// Test setting of FLAG_C
		// If 1 (there is a carry), program execution resumes at address specified in JNZ instruction
		// else program continues with next sequential instruction
		break;

	case 0xDB:	// IN          
		break;

	case 0xDC:	// CC (Call if carry)
		// Test setting of FLAG_Z
		// if 1, push contents of PC onto stack
		// Jump to address specified in instruction's second and third bytes
		// else continue with next sequential instruction
		break;

	case 0xDD:	// .BYTE 0xDD  
		break;

	case 0xDE:	// SBI (Subtract immediate with borrow)
		// Fetch hb & lb
		// 16 bit
		// Subtract contents of second instruction byte and setting of FLAG_C from A
		// Store in A
		break;

	case 0xDF:	// RST 3       
		break;

	case 0xE0:	// RPO (Return if parity odd)
		// if byte in A has odd number of 1 bits, FLAG_P is set to 0
		// RPO tests carry flag
		// if 0, pop 2 bytes of stack and place them in PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xE1:	// POP H
			if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2))) // Check address held in SP is valid. Opposite of PUSH, removing 2 bytes.
		{
			Registers[REGISTER_L] = Memory[StackPointer];
			StackPointer++;										// Increment
			Registers[REGISTER_H] = Memory[StackPointer];
			StackPointer++;
		}
		break;

	case 0xE2:	// JPO  (Jump if parity is odd)
		// if byte in A has odd number of 1 bits, FLAG_P is set to 0
		// JPO tests carry flag
		// if 0, program execution resumes at address specified in JPO instruction
		// else continue with next sequential instruction
		break;

	case 0xE3:	// XTHL        
		break;

	case 0xE4:	// CPO (Call if parity odd)
		// if byte in A has odd number of 1 bits, FLAG_P is set to 0
		// CPO tests carry flag
		// if 0, push contents of PC onto stack
		// Jump to address specified in bytes two and 3 of the instruction
		// else continue with next sequential instruction
		break;

	case 0xE5:	// PUSH H
			if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1))) // Check address held in SP is valid. Require 2 bytes, make sure there are enough free bytes below.
		{
			StackPointer--;										// Decrement SP
			Memory[StackPointer] = Registers[REGISTER_H];		// Copy data
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_L];
		}
		break;

	case 0xE6:	// ANI (And immediate with accumulator)#
		// Fetch high & low bytes
		// Perform logical AND with second byte of the instruction and A.
		// Result placed in A
		// Reset FLAG_C to 0
		break;

	case 0xE7:	// RST 4       
		break;

	case 0xE8:	// RPE (Return if even)
		// if byte in A has even number of 1 bits, FLAG_P is set to 1
		// RPE tests carry flag
		// if 1, pop 2 bytes off stack and place them onto PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xE9:	// PCHL 
		break;

	case 0xEA:	// JPE (Jump if parity even)
		// if byte in A has even number of 1 bits, FLAG_P is set to 1
		// JPE tests carry flag
		// if 1, program execution resumes at address specified in JPO instruction
		// else continue with next sequential instruction
		break;

	case 0xEB:	// XCHG (Exchange H and L with D and E)
		// Save H and L
		// Load D and E new address into H and L
		// Load H and L into D and E
		break;

	case 0xEC:	// CPE (Call if parity even)
		// if byte in A has even number of 1 bits, FLAG_P is set to 1
		// CPO tests carry flag
		// if 1, push contents of PC onto stack
		// Jump to address specified in bytes two and 3 of the instruction
		// else continue with next sequential instruction
		break;

	case 0xED:	// .BYTE 0xED  
		break;

	case 0xEE:	// XRI (Exclusive or immediate with accumulator)
		// Fetch high & low bytes
		// Perform exclusive OR on second byte of instruction and A
		// Place result in A
		// Reset FLAG_C to 0
		// Reset FLAG_A to 0
		break;

	case 0xEF:	// RST 5       
		break;

	case 0xF0:	// RP (Return if positive)
		// Test FLAG_C
		// if 0 (positive data in A), pop 2 bytes off stack and place them into PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xF1:	// POP PSW
			if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2))) // Check address held in SP is valid. Opposite of PUSH, removing 2 bytes.
		{
			Flags = Memory[StackPointer];						// Flags
			StackPointer++;										// Increment
			Registers[REGISTER_A] = Memory[StackPointer];		// Accumulator
			StackPointer++;
		}
		break;

	case 0xF2:	// JP (Jump if positive)
		// Test FLAG_S
		// if 0 (positive data in A), program execution resumes at address specified in JP instruction
		// else continue with next sequential instruction
		break;

	case 0xF3:	// DI          
		break;

	case 0xF4:	// CP (Call if positive)
		// Test setting of FLAG_S
		// if 0 (contents of A are positive), push contents of PC onto stack
		// Jump to address specified in instruction's second and third bytes
		// else continue with next sequential instruction
		break;

	case 0xF5:	// PUSH PSW. If carry flag is 0, do the push.
			if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1))) // Check address held in SP is valid. Require 2 bytes, make sure there are enough free bytes below.
		{
			StackPointer--;										// Decrement SP
			Memory[StackPointer] = Registers[REGISTER_A];		// Copy data
			StackPointer--;
			Memory[StackPointer] = Flags; // S, Z, 0, AC, 0, P, 1, CY
		}
		break;

	case 0xF6:	// ORI (Inclusive or immediate)
		// Fetch high & low bytes
		// Perform inclusive OR on second byte of instruction and A
		// Place result in A
		// Reset FLAG_C to 0
		// Reset FLAG_A to 0
		break;

	case 0xF7:	// RST 6       
		break;

	case 0xF8:	// RM  (Return if minus)
		// Test FLAG_C
		// if 1 (data in A is minus), pop 2 bytes off stack and place them into PC
		// Program execution resumes at new address in PC
		// else continue with next sequential instruction
		break;

	case 0xF9:	// SPHL (Move H and L to SP)
		// Load SP with stack's beginning address
		// SP increment
		// Load H and L contents into SP
		break;

	case 0xFA:	// JM (Jump if minus)
		// Test FLAG_S
		// if 1 (data in A is minus), program execution resumes at address specified in JM instruction
		// else continue with next sequential instruction
		break;

	case 0xFB:	// EI          
		break;

	case 0xFC:	// CM (Call if minus)
		// Test setting of FLAG_S
		// if 1 (contents of the accumulator are minus), push contents of PC onto stack
		// Jump to address specified in instruction's second and third bytes
		// else continue with next sequential instruction
		break;

	case 0xFD:	// .BYTE 0xFD  
		break;

	case 0xFE:	// CPI (Carries out instruction, but does not write that back to the accumulator)
		lb = fetch(); // fetch
		temp_word = (WORD)Registers[REGISTER_A]-(WORD)lb; // Carry flag set to 1 if borrow occurs (when number you are subtracting > number you are subtracting from)
		if (temp_word >= 0x100)
		{								                  
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag		   
		}													   
		break;

	case 0xFF:	// RST 7      
		break;

	default:
		break;
	}
}



void execute(BYTE opcode)
{
	BYTE ms2bits;

	ms2bits = (opcode >> 6) & 0x03;

	switch (ms2bits)
	{
	case 0:
		block_00_instructions(opcode);
		break;

	case 1:		// MOV and HALT
		MOV_and_HLT_instructions(opcode);
		break;

	case 2:
		block_10_instructions(opcode);
		break;

	default:
		block_11_instructions(opcode);
		break;
	}
}


void emulate_8080()
{
	BYTE opcode;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                  A  B  C  D  E  H  L   SP\n");

	while ((!halt) && (memory_in_range))
	{
		printf("%04X ", ProgramCounter);           // Print current address

		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);     // Print Accumulator
		printf("%02X ", Registers[REGISTER_B]);     // Print Register B
		printf("%02X ", Registers[REGISTER_C]);     // Print Register C
		printf("%02X ", Registers[REGISTER_D]);     // Print Register D
		printf("%02X ", Registers[REGISTER_E]);     // Print Register E
		printf("%02X ", Registers[REGISTER_H]);     // Print Register H
		printf("%02X ", Registers[REGISTER_L]);     // Print Register L

		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_S) == FLAG_S)	            // Print Sign Flag
		{
			printf("S=1 ");
		}
		else
		{
			printf("S=0 ");
		}

		if ((Flags & FLAG_Z) == FLAG_Z)	            // Print Zero Flag
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}

		if ((Flags & FLAG_A) == FLAG_A)	            // Print Auxillary Carry Flag
		{
			printf("A=1 ");
		}
		else
		{
			printf("A=0 ");
		}

		if ((Flags & FLAG_P) == FLAG_P)	            // Print Parity Flag
		{
			printf("P=1 ");
		}
		else
		{
			printf("P=0 ");
		}

		if ((Flags & FLAG_C) == FLAG_C)	            // Print Carry Flag
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}




////////////////////////////////////////////////////////////////////////////////
//                      Intel 8080 Simulator/Emulator (End)                   //
////////////////////////////////////////////////////////////////////////////////













void initialise_filenames()
{
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++)
	{
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename)
{
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		if (chr == '.') 
		{
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename)
{
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename)
{
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) 
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename)
{
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) 
	{
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);
		
		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}





void load_and_run()
{
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	ln = 0;
	chr = '\0';
	while (chr != '\n')
	{
		chr = getchar();

		switch(chr)
		{
		case '\n':
				break;
		default:
				if (ln < MAX_FILENAME_SIZE)
				{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
		}
	}

	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else
	{
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else
	{
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file))
	{
		// Clear Registers and Memory

		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;
		for (i=0; i<8; i++)
		{
			Registers[i] = 0;
		}
		for (i=0; i<MEMORY_SIZE; i++)
		{
			Memory[i] = 0;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) 
		{
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer))
			{
				if (sscanf(InputBuffer, "L=%x", &address) == 1)
				{
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1)
				{
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
					{
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else
				{
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate the 8080

		emulate_8080();
	}
	else
	{
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}




void test_and_mark()
{
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
	
	while (!testing_complete)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 0)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1)
			{
				testing_complete = true;
				printf("Current mark = %d\n", mark);
			}
			else if (strcmp(buffer, "Error") == 0)
			{
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else
			{
				// Clear Registers and Memory

				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<8; i++)
				{
					Registers[i] = 0;
				}
				for (i=0; i<MEMORY_SIZE; i++)
				{
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				while (!end_of_program)
				{
					chr = buffer[i];
					switch (chr)
					{
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1)
						{
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1)
						{
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
							{
								Memory[load_at] = (BYTE)code;
							}
							load_at++;
						}
						else
						{
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				// Emulate the 8080

				if (load_at > 1)
				{
					emulate_8080();

					// Send results

					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %04X", Registers[REGISTER_A], Flags, Registers[REGISTER_B], Registers[REGISTER_C], Registers[REGISTER_D], Registers[REGISTER_E], Registers[REGISTER_H], Registers[REGISTER_L], StackPointer);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Intel 8080 Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1 (2013-14)\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
	 // Creation failed!
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock, (SOCKADDR *)&client_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//   //printf("Bind failed! \n");  // Bind failed!
	//}



	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run();
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}

