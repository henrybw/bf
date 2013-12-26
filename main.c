/*
 * bf: a simple 8-bit brainfuck interpreter
 * v2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SLOTS 30000
#define STACK_PAGE 32  /* Number of stack levels to track before reallocating */

/* Data storage */
char *instrs;
size_t *stack;  /* For handling loops. Array of offsets into instrs. */
unsigned char memory[MEM_SLOTS];

/* Runtime state */
size_t ip;  /* Points to next instr to be executed */
size_t sp;  /* Points to next free stack slot (top of stack is at sp-1) */
size_t stacksize;
unsigned short dp;

/* Function defs */
void init(const char *filename);
char *loadinstrs(FILE *file);
void interpret(char ch);
void handlejump(char jumpinst);
void cleanup();

int main(int argc, const char *argv[])
{
    char inst;

    if (argc < 2)
    {
        printf("Usage: bf [source file]\n");
        exit(1);
    }

    init(argv[1]);
    
    /* Main execution loop: fetch next instruction from the instruction
       buffer and interpret it. */
    while ((inst = instrs[ip++]) != 0)
        interpret(inst);
    
    return 0;
}

/* Initialize runtime state and load instructions from the given source file */
void init(const char *filename)
{
    FILE *file;

    file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "File \"%s\" could not be opened.\n", filename);
        exit(1);
    }

    instrs = loadinstrs(file);
    ip = 0;
    fclose(file);

    stack = (size_t *)calloc(STACK_PAGE, sizeof(size_t));
    if (!stack)
    {
        fprintf(stderr, "Failed to allocate the interpreter stack.\n");
        exit(1);
    }

    stacksize = STACK_PAGE;
    sp = 0;

    memset(memory, 0, MEM_SLOTS);
    dp = 0;
}

/* Loads instructions from the given source file. Allocates and returns a
   buffer containing instructions read from the file. */
char *loadinstrs(FILE *file)
{
    long filesize;
    char *buff;
    char *buffptr;
    char ch;

    /* Calculate file size to determine the size of the instruction buffer */
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    
    /* Make the instruction buffer null-terminated by calloc'ing one more
       byte than necessary for the instructions. */
    buff = (char *)calloc(filesize + 1, sizeof(char));
    if (!buff)
    {
        fprintf(stderr, "Failed to allocate the instruction buffer.\n");
        exit(1);
    }

    /* Read instructions from the file */
    buffptr = buff;
    while ((ch = fgetc(file)) != EOF)
        *buffptr++ = ch;

    return buff;
}

/* Parses one instruction */
void interpret(char inst)
{
    switch (inst)
    {
        case '>':
            dp++;
            break;
            
        case '<':
            dp--;
            break;
            
        case '+':
            memory[dp]++;
            break;
            
        case '-':
            memory[dp]--;
            break;
            
        case '.':
            putchar(memory[dp]);
            break;
            
        case ',':
            memory[dp] = getchar();
            break;
            
        case '[':
        case ']':
            handlejump(inst);
            break;
    }
}

/* Manipulate control flow based on the given jump instruction */
void handlejump(char jumpinst)
{
    if (jumpinst == '[')
    {
        if (memory[dp])
        {
            /* We are going to execute the loop body: save the loop start
               onto the stack for when we encounter the loop end. */
            stack[sp++] = ip;

            if (sp >= stacksize)
            {
                stacksize *= 2;
                stack = (size_t *)realloc(stack, stacksize);
                if (!stack)
                {
                    fprintf(stderr, "Stack overflow.\n");
                    exit(1);
                }
            }
        }
        else
        {
            /* We are skipping this loop: find the bracket that matches the
               bracket we are currently processing, accounting for nesting. */
            int nestdepth = 0;
            int currentip = ip - 1;  /* So we process the original '[' first */

            do
            {
                char inst = instrs[currentip++];

                if (inst == '[')
                    nestdepth++;
                else if (inst == ']')
                    nestdepth--;
            }
            while (nestdepth > 0);

            ip = currentip;
        }
    }
    else if (jumpinst == ']')
    {
        if (memory[dp])
        {
            /* Jump back to the beginning of the loop body */
            ip = stack[sp - 1];
        }
        else
        {
            /* Exit loop body and pop this loop from the stack */
            sp--;
        }
    }
}

/* Frees all globally allocated resources */
void cleanup()
{
    free(instrs);
    free(stack);
}

