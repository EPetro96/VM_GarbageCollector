#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>


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
// #define CONS 0x30
// #define HD 0x31
// #define TL 0x32

#define MAX_ALLOCS (1 << 16)
#define BLOCK_SIZE 64

//define some assisting functions
#define push(STACK, TOP, ELEM) (STACK[++TOP] = ELEM)
#define pop(STACK, TOP) (STACK[TOP--])
#define get1byte(PC) byte_program[PC+1]
#define get2bytes(PC) byte_program[PC+1] + (byte_program[PC+2] << 8)
#define get4bytes(PC) byte_program[PC+1] + (byte_program[PC+2] << 8) + (byte_program[PC+3] << 16) + (byte_program[PC+4] << 24)

struct heap_node_t {
	int64_t hd;
	bool hd_type;
	int64_t tail;
	bool tail_type;
	uint8_t mark;
	uint8_t old;
	struct heap_node_t *next;
};

typedef struct heap_node_t heap_node;

inline heap_node* new(int64_t hd, bool hd_type, int64_t tail, bool tail_type, heap_node** clear, heap_node** new_node){
	heap_node* node;
	if(*clear) {
		node = (*clear);
		(*clear) = (*clear)->next;
	}
	else {
		node = (*new_node);
		(*new_node)++;
	}
	node->hd = hd;
	node->hd_type = hd_type;
	node->tail = tail;
	node->tail_type = tail_type;
	node->mark = 0;
	node->old = 0;
	return node;
};

void mark_cons(heap_node* node){
	if (node->old) return;
	node->mark++;
	if (node->hd_type) {
		heap_node* hd = (heap_node*)node->hd;
		if(!hd->mark) mark_cons(hd);
	}
	if (node->tail_type) {
		heap_node* tail = (heap_node*)node->tail;
		if (!tail->mark) mark_cons(tail);
	}
	return;
}

void mark(int64_t *stack, bool *types, int32_t top){
	for (int i = 0; i <= top; i++)
		if (types[i]) {
			heap_node* node = (heap_node*)stack[i];
			if(!node->mark) mark_cons(node);
		}
		return;
}

heap_node* sweep(heap_node* head, heap_node** last, heap_node** clear, uint32_t *number_of_cons){
	heap_node* heap_head = head;
	heap_node* current = head, *previous = head;
	while(current){
		if (current->mark) {
			current->mark--;
			current->old++;
			previous = current;
			current = current->next;
		}
		else {
			if (current == heap_head) {
				heap_node* temp = current;
				current = current->next;
				previous = current;
				heap_head = current;
				temp->next = (*clear);
				(*clear) = temp;
			}
			else {
				heap_node* temp = current;
				current = current->next;
				previous->next = current;
				temp->next = (*clear);
				(*clear) = temp;
			}
			(*number_of_cons)--;
		}
	}
	*last = previous;
	return heap_head;
}

#define gc() mark(&stack[0], &types[0], top); heap_head = sweep(heap_head, &last, &clear, &number_of_cons);

#define gc_old() mark(&stack[0], &types[0], top); old_heap_head = sweep(old_heap_head, &last, &clear, &number_of_old_cons);


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
		&&clock_label,
		&&none,
		&&none,
		&&none,
		&&none,
		&&none,
		&&cons_label,
		&&hd_label,
		&&tl_label,
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
		0,
		0,
		0,
		0,
		0,
		0,
		0,	//cons
		0,	//hd
		0	//tl
	};

	FILE *fin = fopen(argv[1], "rb");

	register int32_t top = -1;
	int64_t stack[1 << 16];
	bool types[1 << 16];
	uint64_t byte_program[1 << 16];
	uint8_t opcode_byte;

	memset(&types, 0, sizeof(types));

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
	register heap_node* heap_head = NULL;

	register heap_node* old_heap_head = NULL;
	
	heap_node* clear = NULL;
	heap_node* last = NULL;

	uint32_t max_allocs = (MAX_ALLOCS) >> 2;
	uint32_t number_of_cons = 0;
	uint32_t number_of_old_cons = 0;
	heap_node* new_nodes;
	register bool first_time = true;
	heap_node* last_address = NULL;

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

	cons_label:
	{
		//print("CONS\n");
		if (first_time) {
			new_nodes = malloc(sizeof(heap_node) * BLOCK_SIZE);
			first_time = false;
			last_address = &new_nodes[BLOCK_SIZE - 1];
		}
		pc+=1;
		heap_node* node = new(stack[top-1], types[top-1], stack[top], types[top], &clear, &new_nodes);
		if (last_address == new_nodes) first_time = true;
		number_of_cons++;
		top--;
		node->next = heap_head;
		heap_head = node;
		stack[top] = (intptr_t)node;
		types[top] = true;
		if (number_of_cons + max_allocs >= MAX_ALLOCS){
			//printf("Collecting some garbage...\n");	//for debugging purposes
			gc();
			last->next = old_heap_head;
			old_heap_head = heap_head;
			heap_head = NULL;
			number_of_old_cons += number_of_cons;
			number_of_cons = 0;
			if (number_of_old_cons >= max_allocs) {
				//printf("Collecting old garbage...\n");	//for debugging purposes
				gc_old();
			}
		}
		NEXT_INSTRUCTION;
	}
	
	hd_label:
	{
		//print("HD\n");
		pc+=1;
		heap_node* node = (heap_node*)stack[top];
		stack[top] = node->hd;
		types[top] = node->hd_type;
		NEXT_INSTRUCTION;
	}

	tl_label:
	{
		//print("TL\n");
		pc+=1;
		heap_node* node = (heap_node*)stack[top];
		stack[top] = node->tail;
		types[top] = node->tail_type;
		NEXT_INSTRUCTION;
	}

	none:
	{
		printf("Non-valid opcode! VM will terminate!\n");
		return 0;
	}
	
	return 0;
}