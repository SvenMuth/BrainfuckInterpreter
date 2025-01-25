#include <stdio.h>
#include <string.h>

#include "main.h"
#include "log.h"

void log_program(const char* message)
{
    static int counter = 1;
    printf("(%d): %s\n", counter, message);

    counter++;
}

void process_instructions(const char instruction, node_t** current_pos)
{
    static char buffer[50];
    static char* type;
    switch (instruction)
    {
    case TOKEN_MOVE_RIGHT:
        move_right(current_pos);
        type = "array pos now: ";
        break;

    case TOKEN_MOVE_LEFT:
        move_left(current_pos);
        type = "array pos now: ";
        break;

    case TOKEN_DISPLAY:
        display(*current_pos);
        type = "output: ";
        break;

    case TOKEN_READ:
        read(*current_pos);
        type = "read in: ";
        break;

    case TOKEN_ADD_ONE:
        add(*current_pos);
        type = "add: ";
        break;

    case TOKEN_SUBTRACT_ONE:
        subtract(*current_pos);
        type = "subtract: ";
        break;

    case TOKEN_JUMP_IF_ZERO:
        jump_if_zero(current_pos);
        type = "jump to: ";
        break;

    case TOKEN_JUMP_IF_NOT_ZERO:
        jump_if_not_zero(current_pos);
        type = "jump to: ";
        break;

    default:
        break;
    }

    snprintf(buffer, sizeof(buffer),
        "  %c  |  %s %d", instruction, type, (*current_pos)->value);
    log_program(buffer);
}

void clear_stdin()
{
    while (true)
    {
        int c = getchar();
        if (c == EOF || c == '\n')
        {
            break;
        }
    }
}

void move_right(node_t** current_pos)
{
    if ((*current_pos)->next == nullptr)
    {
        node_t* tmp = create_new_node(0);
        insert_after_node(*current_pos, tmp);
    }
    *current_pos = (*current_pos)->next;
}

void move_left(node_t** current_pos)
{
    if ((*current_pos)->prev == nullptr)
    {
        node_t* tmp = create_new_node(0);
        insert_at_head(current_pos, tmp);
    }
    *current_pos = (*current_pos)->prev;
}

void display(const node_t* current_pos)
{
    const char value_char = (char)current_pos->value;
}

void read(node_t* current_pos)
{
    printf("Input value: ");
    current_pos->value = getchar();

    clear_stdin();
}

void add(node_t* current_pos)
{
    current_pos->value += 1;
}

void subtract(node_t* current_pos)
{
    current_pos->value -= 1;
}

void jump_if_zero(node_t** current_pos)
{
    if ((*current_pos)->value == 0)
    {
        node_t* tmp = *current_pos;
        while (tmp != nullptr)
        {
            if (tmp->value == TOKEN_JUMP_IF_ZERO)
            {
                break;
            }
            tmp = tmp->next;
        }
        *current_pos = tmp;
    }
}

void jump_if_not_zero(node_t** current_pos)
{
    if ((*current_pos)->value != 0)
    {
        node_t* tmp = *current_pos;
        while (tmp != nullptr)
        {
            if (tmp->value == TOKEN_JUMP_IF_NOT_ZERO)
            {
                break;
            }
            tmp = tmp->prev;
        }
        *current_pos = tmp;
    }
}


int main(void)
{
    node_t* head = nullptr;
    node_t* tmp = create_new_node(0);

    insert_at_head(&head, tmp);
    node_t** current_node = &head;

    constexpr char instructions[] = ", > , > ,. <. <.";
    for (int i = 0; i < strlen(instructions); i++)
    {
        if (instructions[i] == SPACE_KEY)
        {
            continue;
        }

        //printf("%c\n", instructions[i]);
        process_instructions(instructions[i], current_node);
    }
}
