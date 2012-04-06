
// C doesn't have boolean data types... lets improvise with our own
typedef enum { false, true } bool;
typedef uint16_t	Word;		// 16-bit integers for our words
typedef uint32_t	BigWord;	// 32-bit integers for long words... kind of a hack really.

#define MEM_SIZE	0x100000	// Thats 128Kb of RAM!
#define REG_SIZE	0x08		// And 8 registers: A, B, C, X, Y, Z, I and J


// Shorthand memory location definitions
#define A		0x00
#define B		0x01
#define C		0x02
#define X		0x03
#define Y		0x04
#define Z		0x05
#define I		0x06
#define J		0x07

// Basic opcode definitions
#define OP_SET	0x01
#define OP_ADD	0x02
#define OP_SUB	0x03
#define OP_MUL	0x04
#define OP_DIV	0x05
#define OP_MOD	0x06
#define OP_SHL	0x07
#define OP_SHR	0x08
#define OP_AND	0x09
#define OP_BOR	0x10
#define OP_XOR	0x11
#define OP_IFE	0x12
#define OP_IFN	0x13
#define OP_IFG	0x14
#define OP_IFB	0x15

// Extended opcode definitions
#define EOP_JSR	0x01