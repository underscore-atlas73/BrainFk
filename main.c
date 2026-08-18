#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

unsigned char MEMORY[65536] = {0};
uint16_t memPointer = 0;

uint16_t codeLength = 0;
union {
	FILE* fp; //file pointer
	uint16_t ip; //instruction pointer
} codePointer;
char* codeArgument = NULL;

uint16_t inputMode = 0;; //bit 15 = file mode; bit 14 = argument mode; bits 0 - 9 argument length
union {
	FILE* fp; //file pointer
	uint16_t ip; //input pointer 
} inputPointer;
char* inputArgument = NULL;

void interpret(char c);
char getInput(void);
void fileParse(void);
void argParse(void);

void main(int argc, char* argv[]) {
	if (argc == 1) { //requires arguments in all cases
			puts("Invalid usage. Please refer to documentation.");
			return;
	}
	
	setvbuf(stdout, NULL, _IONBF, 0); //immediately output all characters
	
	if (argc >= 3) { //these flags are only acceptable in cases where 2 or more arguments have been passed
		if (strcmp(argv[argc - 2], "-i") == 0) inputMode = 1 << 14 | strlen(argv[argc - 1]);
		if (inputMode & 0b0011110000000000) { //in most environments, the length of one argument wouldn't be greater than 16KB, destroying bits 15 and 14
			puts("Input string too long. Please defer through file.");
		}
		
		inputArgument = argv[argc - 1];
		
		if (strcmp(argv[argc - 2], "-if") == 0) inputMode = 1 << 15; //"-if": Input file flag; used to pass input through file
		
		if (strcmp(argv[1], "-f") == 0) { //"-f" : File flag; used to pass program through file
				codePointer.fp = fopen(argv[2], "rb"); //Due to Windows' god awful translation implementation, I've opted to read raw bytes instead. It was royally screwing up ftell.
				if(!codePointer.fp) { fprintf(stderr, "fopen failed to open code file \"%s\": %s\n", argv[2], strerror(errno)); return; }
				fileParse();
				return;
		}
	}
	
	codeArgument = argv[1];
	codeLength = strlen(argv[1]);
	if (codeLength > 1024) {
		puts("Argument code too long. Please defer through file.");
	}
	argParse();
	
	return;
}

void interpret(char c) {
	//printf("Code FPos: %ld\n", ftell(codePointer.fp));
	int32_t fpos;
	switch (c) {
		case '#': //memdump
			FILE* memdump = fopen("mem_dump.txt", "w");
			if (!memdump) { printf("Memory dump failed on fopen with: %s", strerror(errno)); return;}
			
			uint32_t i;
			for (i = 0; i < (1 << 16); i++) {
				fprintf(memdump, "%02X ", MEMORY[i]);
				if ((i % 8 == 7) && (i + 1 != (1 << 16))) fprintf(memdump, "\n");
			}
			fclose(memdump);
			
		case '<':
			memPointer--;
			break;
		case '>':
			memPointer++;
			break;
		case '+':
			MEMORY[memPointer]++;
			break;
		case '-':
			MEMORY[memPointer]--;
			break;
		case '.':
			putchar(MEMORY[memPointer]);
			break;
		case ',':
			MEMORY[memPointer] = getInput();
			break;
		case '[':
			if(!MEMORY[memPointer]) {
				if (!codeLength) {
					while ( (c = fgetc(codePointer.fp)) != EOF && c != ']');
					if (c == EOF) {
						printf("Unexpected EOF at %i, expected ']'.\n", ftell(codePointer.fp));
						exit(0);
					}
					return;
				}

				while (++codePointer.ip != codeLength && codeArgument[codePointer.ip] != ']');
				if (codePointer.ip == codeLength) {
					printf("Unexpected end of input at %i, expected ']'.\n", codePointer.ip);
					exit(0);
				}
				return;
			}
			
			fpos = (!codeLength) ? ftell(codePointer.fp) : codePointer.ip;
			parse_loop:
			
			if (!codeLength) {
				while ( (c = fgetc(codePointer.fp)) != EOF && c != ']') {
					interpret(c);
				}
				
				if (c == EOF) {
						printf("Unexpected EOF at %i, expected ']'.\n", ftell(codePointer.fp));
						exit(0);
				}
			}
			if (codeLength) {
				while (++codePointer.ip != codeLength && codeArgument[codePointer.ip] != ']')
					interpret(codeArgument[codePointer.ip]);
				
				if (codePointer.ip == codeLength) {
					printf("Unexpected end of input at %i, expected ']'.\n", codePointer.ip);
					exit(0);
				}
			}
			
			if(MEMORY[memPointer] && !codeLength) { fseek(codePointer.fp, fpos, SEEK_SET); goto parse_loop; }
			if(MEMORY[memPointer] &&  codeLength) { codePointer.ip = fpos; goto parse_loop; }
			return;
		case ']':
			printf("Stray bracket at: %ld.", ftell(codePointer.fp));
			exit(0);
		default:
			break;
	}
}

char getInput() {
	if (inputMode & (1 << 14)) {
		if (inputPointer.ip == (inputMode & 0x3FF)) return '\0'; //if input pointer is at end of argument + 1, return null
		return inputArgument[inputPointer.ip++];
	}
	if (inputMode & (1 << 15)) {
		int c;
		if (!inputPointer.fp) {
			inputPointer.fp = fopen(inputArgument, "rb");
			if(!inputPointer.fp) { fprintf(stderr, "Initial fopen of input file \"%s\" failed: %s\n", inputArgument, strerror(errno)); exit(0); }
		}
		c = fgetc(inputPointer.fp);
		return (c + 1) ? (char)c : '\0';
	}
	return '\0';
}

void validate(uint8_t ptype, char* program) {
	uint32_t openCount = 0;
	uint32_t closeCount = 0;
	
	if (ptype) {
		int c;
		while ( (c = fgetc(codePointer.fp)) != EOF) {
			if (c == '[') openCount++;
			if (c == ']') closeCount++;
		}
		
		if (openCount != closeCount) {
			printf("Preliminary program validation returned mismatched open/close bracket counts (%i/%i).\n", openCount, closeCount);
			exit(0);
		}
		rewind(codePointer.fp);
		return;
	}
	
	while (codePointer.ip != codeLength) {
		if (program[codePointer.ip] == '[') openCount++;
		if (program[codePointer.ip] == ']') closeCount++;
		codePointer.ip++;
	}
	if (openCount != closeCount) {
		printf("Preliminary program validation returned mismatched open/close bracket counts (%i/%i).\n", openCount, closeCount);
		return;
	}
	codePointer.ip = 0;
	return;
}

void fileParse(void) {
	int c;
	
	validate(1, NULL);
	
	while ( (c = fgetc(codePointer.fp)) != EOF) {
		interpret(c);
	}
}

void argParse() {
	validate(0, codeArgument);

	while (codePointer.ip != codeLength) {
		interpret(codeArgument[codePointer.ip]);
		codePointer.ip++;
	}
}