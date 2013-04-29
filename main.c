/*
 * bf: a brainfuck interpreter
 * v1.0, by Henry Weiss
 * 10/28/08
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SLOTS 30000

/* Data storage */
unsigned char memory[MEM_SLOTS];
unsigned int ptr = 0;

/* Function defs */
void parsechar(char ch, FILE *file);
void handleloop(FILE *file);

int main(int argc, const char *argv[])
{
    /* Check for valid args */
    if (argc < 2)
    {
        printf("Usage: bf [source file]\n");
        exit(1);
    }
    
    /* Open the file for reading */
    FILE *file = fopen(argv[1], "r");
    
    if (!file)
    {
        fprintf(stderr, "File \"%s\" could not be opened.\n", argv[1]);
        exit(1);
    }
    
    /* Initialize the memory slots */
    int i;
    
    for (i = 0; i < MEM_SLOTS; i++)
        memory[i] = 0;
    
    /* Loop through chars and parse the tokens */
    char ch;
    
    while ((ch = fgetc(file)) != EOF)
        parsechar(ch, file);
    
    /* Clean up */
    fclose(file);
    
    return 0;
}

/* Parses one character and takes appropriate action if it's a token */
void parsechar(char ch, FILE *file)
{
    switch (ch)
    {
        case '>':
            ptr++;
            break;
            
        case '<':
            ptr--;
            break;
            
        case '+':
            memory[ptr]++;
            break;
            
        case '-':
            memory[ptr]--;
            break;
            
        case '.':
            putchar(memory[ptr]);
            break;
            
        case ',':
            memory[ptr] = getchar();
            break;
            
        case '[':
            handleloop(file);
            break;
    }
}

/* Handles all execution inside a loop. Nested loops are handled recursively,
   since parsechar() will call this again if another loop token is found. */
void handleloop(FILE *file)
{
    char ch;
    fpos_t loopstart;
    
    /* Check if we should even enter the loop in the first place */
    if (memory[ptr])
    {
        /* Save the position of the loop start */
        fgetpos(file, &loopstart);
        
        /* Set up the loop execution */
        while (memory[ptr])
        {
            /* Go to the top of the loop again */
            fsetpos(file, &loopstart);
            
            /* Actually execute the loop body now */
            while ((ch = fgetc(file)) != ']')
                parsechar(ch, file);
        }
    }
    else
    {
        while ((ch = fgetc(file)) != ']')
            ;  /* Skip over the loop */
    }
}