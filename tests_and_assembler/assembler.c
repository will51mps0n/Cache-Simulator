#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);


int getOpcodeValue(const char *opcode)
{

    if (strcmp(opcode, "add") == 0)
        return 0b000;
    if (strcmp(opcode, "nor") == 0)
        return 0b001;
    if (strcmp(opcode, "lw") == 0)
        return 0b010;
    if (strcmp(opcode, "sw") == 0)
        return 0b011;
    if (strcmp(opcode, "beq") == 0)
        return 0b100;
    if (strcmp(opcode, "jalr") == 0)
        return 0b101;
    if (strcmp(opcode, "halt") == 0)
        return 0b110;
    if (strcmp(opcode, "noop") == 0)
        return 0b111;
    if (strcmp(opcode, ".fill") == 0)
        return -1;
    return 8; // Error case
}

static inline int convertNum(int32_t);
 //convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int32_t num)
{
    return num - ((num & (1 << 15)) ? 1 << 16 : 0);
}

bool validRegisterBounds(int reg)
{
    if (reg < 0 || reg > 7)
    {
        return false;
    }
    return true;
}
void errorChecks(int opcodeInt, char *arg0, char *arg1, char *arg2)
{
    // printf("errorChecks\n");
    if (opcodeInt == 8)
    {
        printf("Error: invalid opcode\n");
        exit(1);
    }
    if ((opcodeInt >= 0) && (opcodeInt <= 5))
    {
        // printf("entered 0 - 5 check\n");
        if ((!isNumber(arg0)) || (!isNumber(arg1)))
        {
            printf("error: registers 0 and 1 must be int\n");
            exit(1);
        }
        if (!validRegisterBounds(atoi(arg0)) || !validRegisterBounds(atoi(arg1)))
        {
            printf("error: registers 0 or 1 out of bounds\n");
            exit(1);
        }

        if (opcodeInt == 0 || opcodeInt == 1)
        {
            if (!isNumber(arg2))
            {
                printf("error: register 2 must be int\n");
                exit(1);
            }
            if (!validRegisterBounds(atoi(arg2)))
            {
                printf("error: register 2 out of bounds\n");
                exit(1);
            }
        }
    }
}
int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL)
    {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // change:: from here below
    // first a loop to read in labels and their variables into an array
    // array of
    // first pass counts the number of lines to initialize our array
    int labelCount = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (label[0] != '\0')
        {
            labelCount++;
        }
    }
    // assuming there is at most 1 label corresponding to every command
    char labelArr[labelCount][7];
    int labelLocationArr[labelCount];
    int commandIndx = 0;
    int labelIndx = 0;
    // rewind for our second pass
    rewind(inFilePtr);

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        if (label[0] != '\0')
        {
            strcpy(labelArr[labelIndx], label);
            labelLocationArr[labelIndx] = commandIndx;
            int count = 0;
            while (count != labelIndx)
            {
                if (!strcmp(labelArr[labelIndx], labelArr[count]))
                {
                    printf("error, duplicate label encountered. \n");
                    exit(1);
                }
                count++;
            }
            // printf("Debug: Label '%s' added at index %d\n", labelArr[labelIndx], commandIndx);
            labelIndx++;
        }
        commandIndx++;
    }

    // rewind for third and final pass
    rewind(inFilePtr);
    // getOpcodeValue(const char* opcode)
    int currentPC = 0;

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        int opcodeInt = getOpcodeValue(opcode);
        errorChecks(opcodeInt, arg0, arg1, arg2);

        int regA = 0;
        int regB = 0;

        // R-type instructions (add, nor)
        if (opcodeInt == 0 || opcodeInt == 1)
        {   
            regA = atoi(arg0);
            regB = atoi(arg1);
            int regC = atoi(arg2);

            opcodeInt <<= 22;
            regA <<= 19;
            regB <<= 16;
            int machineCode = opcodeInt + regA + regB + regC;
            fprintf(outFilePtr, "%d\n", machineCode);
            currentPC++;
            continue;
        }

        // J-type instruction (jalr)
        else if (strcmp(opcode, "jalr") == 0)
        {
            regA = atoi(arg0);
            regB = atoi(arg1);

            opcodeInt <<= 22;
            regA <<= 19;
            regB <<= 16;

            int machineCode = opcodeInt + regA + regB;
            fprintf(outFilePtr, "%d\n", machineCode);
            currentPC++;
            continue;
        }

        // I-type instructions (lw, sw, beq) and .fill
        else if ((strcmp(opcode, "lw") == 0) || (strcmp(opcode, "sw") == 0) || (strcmp(opcode, "beq") == 0) || (strcmp(opcode, ".fill") == 0))
        {
            if ((strcmp(opcode, ".fill") == 0)) {
                    if (!isNumber(arg0)){
                    strcpy(arg2, arg0);
                    //printf("%s\n",arg2);
                    } 
                    else{
                    fprintf(outFilePtr,"%s\n", arg0);
                    currentPC++;
                    continue; 
                    }
             }
            regA = atoi(arg0);
            regB = atoi(arg1);

            int offset = isNumber(arg2) ? atoi(arg2) : -1;
                bool foundLabel = false;
            // case that offset is a label
            if (isNumber(arg2) == 0)
            {
                for (int i = 0; i < labelCount; i++)
                {
                    if (!strcmp(arg2, labelArr[i]))
                    {
                        offset = labelLocationArr[i];
                        foundLabel = true;
                        break;
                    }
                }
                if (!foundLabel)
                {
                    printf("error, label not found\n");
                    exit(1);
                }
                if (strcmp(opcode, "beq") == 0)
                { //&& regA == regB
                    offset = offset - 1 - currentPC;
                } 
            }
            else if ( offset < -32768 || offset > 32767) {
                printf("error, offset not 16 bit");
                exit(1);
            }

            if (offset < 0)
            {
                offset = convertNum(offset);
                int32_t bitmask = 0xFFFF;
                offset = offset & bitmask;
            }
            
            opcodeInt <<= 22;
            regA <<= 19;
            regB <<= 16;

            if (strcmp(opcode, ".fill") == 0)
            {
                fprintf(outFilePtr, "%d\n", offset);
            }
            else
            {
                
                int machineCode = opcodeInt + regA + regB + offset;
                fprintf(outFilePtr, "%d\n", machineCode);
            }
            currentPC++;
        }

        // O-type instructions (halt, noop)
        else if ((strcmp(opcode, "halt") == 0) || (strcmp(opcode, "noop") == 0))
        {
            opcodeInt <<= 22;
            fprintf(outFilePtr, "%d\n", opcodeInt);
            currentPC++;
        }
    }
    // printf("%d", currentPC);
    exit(0);
    return (0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line)
{
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for (int line_idx = 0; line_idx < strlen(line); ++line_idx)
    {
        int line_char_is_whitespace = 0;
        for (int whitespace_idx = 0; whitespace_idx < 4; ++whitespace_idx)
        {
            if (line[line_idx] == whitespace[whitespace_idx])
            {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if (!line_char_is_whitespace)
        {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr)
{
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for (int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address)
    {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH - 1)
        {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if (lineIsBlank(line))
        {
            if (!blank_line_encountered)
            {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        }
        else
        {
            if (blank_line_encountered)
            {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}

/*
 * NOTE: The code defined below is not to be modifed as it is implimented correctly.
 */

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {
        /* reached end of file */
        return (0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH - 1)
    {
        printf("error: line too long\n");
        exit(1);
    }

    // Ignore blank lines at the end of the file.
    if (lineIsBlank(line))
    {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label))
    {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);

    return (1);
}

static inline int

isNumber(char *string)
{
    int num;
    char c;
    return ((sscanf(string, "%d%c", &num, &c)) == 1);
}
