#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <assert.h>
#include <string.h> /* memset */

// Include our header file
// It includes some definitions to try and keep the code pretty
#include "dcpu16.h"

Word *lookup(struct CPU *Instance, Word Address) {

	// Make sure we are within the addressable range (Up to 0x3F)
	assert( Address < 0x40 );
	
	switch (Address) {
		// If we are within 0x00 to 0x07, Simply return the addressed register
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			return &Instance->REG[Address];
		
		// If we are within 0x08 to 0x0F, return the memory using the address
		// that is stored in the addressed register
		case 0x08: case 0x09: case 0x0A: case 0x0B:
		case 0x0C: case 0x0D: case 0x0E: case 0x0F:
			// We are 0x08 offset, so subtract 0x08 first.
			return &Instance->MEM[ Instance->REG[Address - 0x08] ]; 
		
		// If we are in the 0x10 to 0x17 range, 
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			// We are 0x10 offset, so subtract 0x10 first.
			return &Instance->MEM[ Instance->REG[Address - 0x10] + Instance->MEM[Instance->PC++] ];
			
		case 0x18:
			// Return the NEXT piece of memory as pointed by the stack pointer
			return &Instance->MEM[ Instance->SP++ ];
			
		case 0x19:
			// Return the piece of memory as pointed by the stack pointer
			return &Instance->MEM[ Instance->SP ];
			
		case 0x1a:
			// Return the PREVIOUS piece of memory as pointed by the stack pointer
			return &Instance->MEM[ --Instance->SP ];
			
		case 0x1b:
			// Return the stack pointer
			return &Instance->SP;
			
		case 0x1c:
			// Return the program counter
			return &Instance->PC;
			
		case 0x1d:
			// Return the overflow
			return &Instance->O;
			
		case 0x1e:
			// Return the next instruction
			return &Instance->MEM[ Instance->MEM[Instance->PC++] ];
			
		case 0x1f:
			// Return the address of the next instruction
			return &Instance->MEM[Instance->PC++];
			
		default:
			// Return a literal between 0x00 and 0x20.
			// We are 0x20 offset already by this point, so subtract 0x20 first.
			return Address - 0x20;
	}
}

void cpu_step(struct CPU *Instance) {
	// This is pretty much one big state machine
	
	// Check if we are supposed to be skipping
	if(Instance->skip == true) {
		Instance->skip = false;		// Make sure we don't skip the next as well!
		return;	
	}
	
	// Remember, the instruction format is like so: bbbbbbaaaaaaoooo
	
	// Firstly, lets grab the entire instruction
	Word Instruction 	= Instance->MEM[ Instance->PC++ ];
	// Now lets strip just the Opcode, by ANDing it with 0xF
	Word Opcode			= Instruction & 0xF;
	// Now lets shift right 4 bits and AND with 0x3F to get the 6 bits of argument A
	Word ArgA			= (Instruction >> 4 ) & 0x3F;
	// Do the same again, (shifting 10 bits this time) to get the 6 bits of argument B
	Word ArgB			= (Instruction >> 10) & 0x3F;
	// Lookup the values held in the memory at both arguments!
	Word *MemA			= lookup(Instance, ArgA);
	Word *MemB			= lookup(Instance, ArgB);
	
	// Non-basic opcodes
	if (Opcode == 0x0000) {
		// Using extended opcodes, the instruction format is slightly different
		// The new opcode is actually where A should be.
		// New format: aaaaaaoooooo0000
		// So lets redefine our opcode
		Opcode = (Instruction >> 4) & 0x3F;
		ArgA = (Instruction >> 10) & 0x3F;
		
		switch (Opcode) {
			case 0x01:
				Instance->MEM[ --Instance->SP ] = Instance->PC;
				Instance->PC = *lookup( Instance, ArgA );
				break;
			default: // Undefined opcode? Lets just say its a halt instruction.
				printf("Halt Detected\n");
				Instance->halt = true;
		}
		// Done executing, finish step
		return;
	}
	
	// By this point, we are sure we don't have an extended opcode,
	// So lets go right ahead and do some processing.
	// We do a little bitwise magic to catch overflows,
	// and to ensure we only get the bits we want.
	// Apologies for the unexplained magic happening here!
	switch (Opcode) {
		case OP_SET:
			*MemA = *MemB; 
			break;
			
		case OP_ADD:
			*MemA = (*MemA + *MemB) & 0xFFFF;
			Instance->O = (*MemA >> 16) & 0xFFFF;
			break;
			
		case OP_SUB:
			*MemA = (*MemA - *MemB) & 0xFFFF;
			Instance->O = (*MemA >> 16) & 0xFFFF;
			break;
			
		case OP_MUL:
			*MemA = (*MemA * *MemB) & 0xFFFF;
			Instance->O = (*MemA >> 16) & 0xFFFF;
			break;
			
		case OP_DIV:
			if (*MemB == 0) {
				*MemA = 0;
				Instance->O = 0;
			} else {
				Instance->O = ((*MemA << 16) / *MemB) & 0xFFFF;
				*MemA = (*MemA / *MemB) * 0xFFFF;
			}
			break;
			
		case OP_MOD:
			if (*MemB == 0) {
				*MemA = 0;
			} else {
				*MemA = *MemA % *MemB;
			}
			break;
			
		case OP_SHL:
			Instance->O = (((*MemA << *MemB) >> 16) & 0xFFFF);
			*MemA = *MemA << *MemB;
			break;
			
		case OP_SHR:
			Instance->O = (((*MemA << 16) >> *MemB) & 0xFFFF);
			*MemA = *MemA >> *MemB;
			break;
			
		case OP_AND:
			*MemA = *MemA & *MemB;
			break;
		
		case OP_BOR:
			*MemA = *MemA | *MemB;
			break;	
		
		case OP_XOR:
			*MemA = *MemA ^ *MemB;
			break;
			
		case OP_IFE:
			if ((*MemA == *MemB) == false) {
				Instance->skip = true;
			}
			break;
			
		case OP_IFN:
			if ((*MemA != *MemB) == false) {
				Instance->skip = true;
			}
			break;
			
		case OP_IFG:
			if ((*MemA > *MemB) == false) {
				Instance->skip = true;
			}
			break;
			
		case OP_IFB:
			if (((*MemA & *MemB) != 0) == false) {
				Instance->skip = true;
			}
			break;
			
		default:	// Didin't find the opcode? That shouldn't have happened!
			printf("Unexpected opcode encountered.\n");	// Screw it, lets continue anyways.
			break;
	}
}


void cpu_init(struct CPU *Instance) {
	// Initialise the stack pointer to 0xFFFF
	Instance->SP = 0xFFFF;
	// Everything else should be zeros
	Instance->PC = 0x0000;
	Instance->O  = 0x0000;
	memset( Instance->MEM, 0, sizeof( Instance->MEM ) );
	memset( Instance->REG, 0, sizeof( Instance->REG ) );

	// We don't want to halt or skip an instruction
	Instance->skip = false;
	Instance->halt = false;
	
	// Ensure that the sub-cycle state is at zero
	Instance->state = 0x0000;

	// I don't think anything else needs to be initialised.
}

int main() {
	
	// Instantiate our CPU! Call it Instance.
	// Because we are using a struct, we could actually have multiple CPUs...
	// Multi core DCPU-16s? Challenge accepted... someday.
	struct CPU Instance;

	cpu_init( &Instance );
	printf("Initialised. Press any key to continue...");
	
	PAUSE;
	
	
	
	while (Instance.halt == false) {
		cpu_step( &Instance );
	}

}