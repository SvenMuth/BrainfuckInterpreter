//
// Created by sven on 25.01.25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter.h"

#ifndef DEBUG
#include "colors.h"
#endif


void int_to_char_array(int_array_t file_data, data_t* exec_data)
{
    exec_data->instructions = malloc(sizeof(char) * file_data.length);
    exec_data->instructions_length = file_data.length;

    if (exec_data->instructions == nullptr)
    {
        ERROR_PRINT("Memory allocation failed!");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < file_data.length; i++)
    {
        exec_data->instructions[i] = (char)file_data.instructions[i];
    }
}

void calculate_size(int_array_t* file_data)
{
    file_data->size = file_data->step * file_data->multiplicator;
    file_data->multiplicator++;
}

void allocate_space(int_array_t* file_data)
{
    calculate_size(file_data);

    file_data->instructions = malloc(sizeof(int) * file_data->size);
    if (file_data->instructions == nullptr)
    {
        ERROR_PRINT("Memory allocation failed!");
        exit(EXIT_FAILURE);
    }
}

void realloc_space(int_array_t* file_data)
{
    calculate_size(file_data);

    file_data->instructions = realloc(file_data->instructions,sizeof(int) * file_data->size);
    if (file_data->instructions == nullptr)
    {
        ERROR_PRINT("Realloc failed!");
        exit(EXIT_FAILURE);
    }
}

void read_file_in_array(FILE* fp, data_t* exec_data)
{
    int_array_t file_data = {
        .length = 0,
        .multiplicator = 1,
        .step = STEP_SIZE,
    };
    allocate_space(&file_data);

    int c;
    while ((c = fgetc(fp)) != EOF)
    {
        // When writing directly to a char array the file pointer was getting corrupted
        file_data.instructions[file_data.length] = c;
        file_data.length++;

        if ((file_data.length + 1) % STEP_SIZE == 0)
        {
            realloc_space(&file_data);
        }
    }

    // That is why conversion is done here
    int_to_char_array(file_data, exec_data);
    free(file_data.instructions);
}

void set_array_zero(const data_t* data)
{
    for (int i = 0; i < data->tape_length; i++)
    {
        data->tape[i] = 0;
    }
}

void log_execution(const char instruction, const char* message, const int value)
{
#if defined(DEBUG) || defined(NO_LOG)
    //Dont print log while debugging or no_log
#else
    //TODO: Implement better format specifiers
    static int counter = 1;
    if (instruction == TOKEN_ADD_ONE || instruction == TOKEN_MOVE_RIGHT)
    {
        printf("(%d) \t \'%c\' %-18s [%d -> %d]\n",
            counter, instruction, message, (value - 1), value);
    }
    else if (instruction == TOKEN_SUBTRACT_ONE || instruction == TOKEN_MOVE_LEFT)
    {
        printf("(%d) \t \'%c\' %-18s [%d -> %d]\n",
            counter, instruction, message, (value + 1), value);
    }
    else if (instruction == TOKEN_DISPLAY)
    {
        printf(HI_MAGENTA "(%d) \t \'%c\' %-18s [%d -> \'%c\']\n" RESET_COLOR,
            counter, instruction, message, value, (char)value);
    }
    else
    {
        printf("(%d) \t \'%c\' %-18s %d\n",
            counter, instruction, message, value);
    }

    counter++;
#endif
}

bool process_instruction(data_t* data, const bool is_jump_active)
{
    switch (data->current_instruction)
    {
    case TOKEN_MOVE_RIGHT:
        move_right(data);
        log_execution(TOKEN_MOVE_RIGHT,
            "[moved right]", data->tape[data->pos_tape]);
        break;

    case TOKEN_MOVE_LEFT:
        move_left(data);
        log_execution(TOKEN_MOVE_LEFT,
            "[moved left]", data->tape[data->pos_tape]);
        break;

    case TOKEN_DISPLAY:
        log_execution(TOKEN_DISPLAY,
            "[output]", data->tape[data->pos_tape]);
        break;

    case TOKEN_READ:
        read(data);
        log_execution(TOKEN_READ,
            "[read in]", data->tape[data->pos_tape]);
        break;

    case TOKEN_ADD_ONE:
        add(data);
        log_execution(TOKEN_ADD_ONE,
            "[add]", data->tape[data->pos_tape]);
        break;

    case TOKEN_SUBTRACT_ONE:
        subtract(data);
        log_execution(TOKEN_SUBTRACT_ONE,
            "[subtract]", data->tape[data->pos_tape]);
        break;

    case TOKEN_JUMP_IF_ZERO:
        const bool result_if_zero = jump_if_zero(data, is_jump_active);
        if (result_if_zero)
        {
            log_execution(TOKEN_JUMP_IF_ZERO,
            "[jump start]", data->tape[data->pos_tape]);
            return true;
        }
        if (!result_if_zero && is_jump_active)
        {
            log_execution(TOKEN_JUMP_IF_ZERO,
            "[r jump end]", data->tape[data->pos_tape]);
        }

        break;

    case TOKEN_JUMP_IF_NOT_ZERO:
        const bool result_if_not_zero = jump_if_not_zero(data, is_jump_active);
        if (result_if_not_zero)
        {
            log_execution(TOKEN_JUMP_IF_NOT_ZERO,
            "[r jump start]", data->tape[data->pos_tape]);
            return true;
        }
        if (!result_if_not_zero && is_jump_active)
        {
            log_execution(TOKEN_JUMP_IF_NOT_ZERO,
            "[jump end]", data->tape[data->pos_tape]);
        }
        break;

    default:
        ERROR_PRINT("Invalid instruction occurred! -> \'%d\'", data->current_instruction);
        exit(EXIT_FAILURE);
        break;
    }

    return false;
}

void move_right(data_t* data)
{
    if (data->pos_tape == data->tape_length)
    {
        ERROR_PRINT("Tape is not large enough! [INDEX: %d]", data->pos_tape);
        exit(EXIT_FAILURE);
    }
    data->pos_tape += 1;
}

void move_left(data_t* data)
{
    if (data->pos_tape == 0)
    {
        ERROR_PRINT("Index is 0! [INDEX: %d]", data->pos_tape);
        exit(EXIT_FAILURE);
    }
    data->pos_tape -= 1;
}

void read(const data_t* data)
{
    printf("Input value: ");
    data->tape[data->pos_tape] = getchar() - '0';

    fseek(stdin,0,SEEK_END);
}

void add(const data_t* data)
{
    data->tape[data->pos_tape] += 1;
}

void subtract(const data_t* data)
{
    data->tape[data->pos_tape] -= 1;
}

bool jump_if_zero(const data_t* data, const bool is_jump_active)
{
    if (is_jump_active)
    {
        return false;
    }

    if (data->tape[data->pos_tape] == 0)
    {
        return true;
    }
    return false;
}

bool jump_if_not_zero(const data_t* data, const bool is_jump_active)
{
    if (is_jump_active)
    {
        return false;
    }

    if (data->tape[data->pos_tape] != 0)
    {
        return true;
    }
    return false;
}

FILE* open_file(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        perror("File opening failed!");
        exit(EXIT_FAILURE);
    }

    return fp;
}

void print_log_header()
{
#if !defined(DEBUG) && !defined(NO_LOG)
    printf("STEP\t INSTRUCTION\t\t\tVALUE\n");
#endif
}

void initialize_exec_data(data_t* data)
{
    *data = (data_t) {
        .tape = malloc(sizeof(int) * TAPE_SIZE),
        .pos_tape = 0,
        .tape_length = TAPE_SIZE - 1,
        .is_jump_if_zero = false,
        .is_jump_if_not_zero = false,
        .instructions = nullptr,
        .pos_instructions = 0,
        .instructions_length = 0,
        .current_instruction = 0,
    };

    if (data->tape == nullptr)
    {
        ERROR_PRINT("Memory allocation failed!");
        exit(EXIT_FAILURE);
    }

    set_array_zero(data);
}

bool run_jump_if_zero(const data_t* data)
{
    if (data->is_jump_if_zero && data->current_instruction != TOKEN_JUMP_IF_NOT_ZERO)
    {
        return true;
    }
    return false;
}

void run_jump_if_not_zero(data_t* data)
{
    if (data->is_jump_if_not_zero)
    {
        while (data->current_instruction != TOKEN_JUMP_IF_ZERO)
        {
            data->pos_instructions--;
            data->current_instruction = data->instructions[data->pos_instructions];
        }
        // Subtract one, otherwise right token is skipped
        data->pos_instructions--;
    }
}

void set_is_jump(data_t* data, const bool status_jump_execution)
{
    if (status_jump_execution)
    {
        if (data->current_instruction == TOKEN_JUMP_IF_ZERO)
        {
            data->is_jump_if_zero = true;
        }
        if (data->current_instruction == TOKEN_JUMP_IF_NOT_ZERO)
        {
            data->is_jump_if_not_zero = true;
        }
    }
}

void reset_is_jump(data_t* data, const bool status_jump_execution)
{
    // Case for "jump if zero"
    if (!status_jump_execution &&
        data->current_instruction == TOKEN_JUMP_IF_NOT_ZERO)
    {
        data->is_jump_if_zero = false;
    }
    // Case for "jump if not zero"
    if (!status_jump_execution &&
        data->current_instruction == TOKEN_JUMP_IF_ZERO)
    {
        data->is_jump_if_not_zero = false;
    }
}














