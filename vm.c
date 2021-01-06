#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>


#define NEXT_INSTRUCTION goto *byte_program[pc]

//define opcodes
// #define HALT 0x00
// #define JUMP 0x01
// #define JNZ 0x02
// #define DUP 0x03
// #define SWAP 0x04
// #define DROP 0x05
// #define PUSH4 0x06
// #define PUSH2 0x07
// #define PUSH1 0x08
// #define ADD 0x09
// #define SUB 0x0a
// #define MUL 0x0b
// #define DIV 0x0c
// #define MOD 0x0d
// #define EQ 0x0e
// #define NE 0x0f
// #define LT 0x10
// #define GT 0x11
// #define LE 0x12
// #define GE 0x13
// #define NOT 0x14
// #define AND 0x15
// #define OR 0x16
// #define INPUT 0x17
// #define OUTPUT 0x18
// #define CLOCK 0x2a

//define some assisting functions
#define push(STACK, TOP, ELEM) (STACK[++TOP] = ELEM)
#define pop(STACK, TOP) (STACK[TOP--])
#define get1byte(PC) byte_program[PC+1]
#define get2bytes(PC) byte_program[PC+1] + (byte_program[PC+2] << 8)
#define get4bytes(PC) byte_program[PC+1] + (byte_program[PC+2] << 8) + (byte_program[PC+3] << 16) + (byte_program[PC+4] << 24)


int main (int argc, char const *argv[]){

	static void *label_tab[] = {
		&&halt_label,
		&&jump_label,
		&&jnz_label,
		&&dup_label,
		&&swap_label,
		&&drop_label,
		&&push4_label,
		&&push2_label,
		&&push1_label,
		&&add_label,
		&&sub_label,
		&&mul_label,
		&&div_label,
		&&mod_label,
		&&eq_label,
		&&ne_label,
		&&lt_label,
		&&gt_label,
		&&le_label,
		&&ge_label,
		&&not_label,
		&&and_label,
		&&or_label,
		&&input_label,
		&&output_label,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&clock_label
	};

	static uint8_t skip_bytes[] = {
		0,	//halt
		2,	//jump
		2,	//jnz
		1,	//dup
		1,	//swap
		0,	//drop
		4,	//push4
		2,	//push2
		1,	//push1
		0,	//add
		0,	//sub
		0,	//mul
		0,	//div
		0,	//mod
		0,	//eq
		0,	//ne
		0,	//lt
		0,	//gt
		0,	//le
		0,	//ge
		0,	//not
		0,	//and
		0,	//or
		0,	//input
		0,	//output
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,	//clock
	};

	FILE *fin = fopen(argv[1], "rb");

	register int32_t top = -1;
	int32_t stack[1 << 16];
	uint64_t byte_program[1 << 16];
	uint8_t opcode_byte;

	uint16_t length = 0;
	while (fscanf(fin, "%c", &opcode_byte) == 1){
		byte_program[length++] = (uint64_t)label_tab[opcode_byte];
		for (uint8_t byte = 0; byte < skip_bytes[opcode_byte]; byte++){
			uint8_t opcode_byte_temp;
			if (fscanf(fin, "%c", &opcode_byte_temp) != 1){
				printf("Error: Wrong bytecode\n");
				return -1;
			}
			else byte_program[length++] = (uint64_t)opcode_byte_temp;
		}
	}
	fclose(fin);

	
	register uint32_t pc = 0;
	clock_t start_time = clock();
	NEXT_INSTRUCTION;

	halt_label:
	{
		// printf("HALT\n");
		return 0;
	}

	jump_label:
	{
		//printf("JUMP\n");
	    pc = get2bytes(pc);
	    NEXT_INSTRUCTION;
	}

	jnz_label:
	{
		//printf("JNZ\n");
	    int32_t stack_top = pop(stack, top);
	    pc = (stack_top != 0) ? get2bytes(pc) : (pc + 3);
	    NEXT_INSTRUCTION;
	}

	dup_label:
	{
		//printf("DUP\n");
		uint8_t i = get1byte(pc);
		pc +=2;
		int32_t elem = stack[top - i];
		push(stack, top, elem);
	    NEXT_INSTRUCTION;
	}


	swap_label:
	{
		//printf("SWAP\n");
		uint8_t i = get1byte(pc);
		pc +=2;
		int32_t elem = stack[top - i];
		int32_t stack_top = pop(stack,top);
		push(stack, top, elem);
		stack[top-i] = stack_top;
		NEXT_INSTRUCTION;
	}


	drop_label:
	{
		//printf("DROP\n");
		pc +=1;
		top--;
		NEXT_INSTRUCTION;
	}


	push4_label:
	{
		//printf("PUSH4\n");
		int32_t num = get4bytes(pc);
		pc +=5;
		push(stack, top, num);
	    NEXT_INSTRUCTION;
	}

	push2_label:
	{
		//printf("PUSH2\n");
		int16_t num = get2bytes(pc);
	    pc += 3;
	    push(stack, top, num);
	    NEXT_INSTRUCTION;
	}

	push1_label:
	{
		//printf("PUSH1\n");
		int8_t num = get1byte(pc);
	    pc += 2;
	    push(stack, top, num);
	    NEXT_INSTRUCTION;
	}

	add_label:
	{
		//printf("ADD\n");
		pc +=1;
		stack[top - 1] += stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}
	
	sub_label:
	{
		//printf("SUB\n");
		pc +=1; 
		stack[top - 1] -= stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	mul_label:
	{
		//printf("MUL\n");
		pc +=1;
		stack[top - 1] *= stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	div_label:
	{
		//printf("DIV\n");
		pc +=1;
		stack[top - 1] /= stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	mod_label:
	{
		//printf("MOD\n");
	    pc += 1;
	    stack[top - 1] %= stack[top];
	    top--;
	    NEXT_INSTRUCTION;
	}

	eq_label:
	{
		//printf("EQ\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] == stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}


	ne_label:
	{
		//printf("NE\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] != stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	lt_label:
	{
		//printf("LT\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] < stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}


	gt_label:
	{
		//printf("GT\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] > stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}


	le_label:
	{
		//printf("LE\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] <= stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}


	ge_label:
	{
		//printf("GE\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] >= stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	not_label:
	{
		//printf("NOT\n");
		pc +=1;
		stack[top] = !stack[top];
	    NEXT_INSTRUCTION;
	}

	and_label:
	{
		//printf("AND\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] && stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	or_label:
	{
		//printf("OR\n");
		pc +=1;
		stack[top - 1] = stack[top - 1] || stack[top];
		top--;
	    NEXT_INSTRUCTION;
	}

	input_label:
	{
		//printf("INPUT\n");
		pc +=1;
		char ch;
		if (scanf("%c", &ch) != 1){
			printf("Error: Wrong input. VM will terminate!\n");
			return -1;
		}
		push(stack, top, (int32_t)ch);
	    NEXT_INSTRUCTION;
	}

	output_label:
	{
		//printf("OUTPUT\n");
		pc +=1;
		int32_t ch = pop(stack, top);
		printf("%c", (char)ch);
	    NEXT_INSTRUCTION;
	}

	clock_label:
	{
		//printf("CLOCK\n");
		pc +=1;
		double time_spent = (double)(clock() - start_time) / CLOCKS_PER_SEC;
		printf("Time spent: %0.61f secs \n", time_spent);
		NEXT_INSTRUCTION;
	}
	
	none:
	{
		printf("Non-valid opcode! VM will terminate!\n");
		return 0;
	}
	
	return 0;
}