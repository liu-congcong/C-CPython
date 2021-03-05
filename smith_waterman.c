#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GAP_OPEN -10
#define GAP_EXTENSION -10

typedef struct path
{
    char pair[2];
    struct path *next;
} PATH;

int string2integers(const char *, size_t, unsigned short *);
int get_source(int *, short *);
int get_path(int **, short **, const size_t, const size_t, const char *, const char *, PATH **);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: smith_waterman string1 string2.\n");
        exit(EXIT_FAILURE);
    };
    size_t row_index, column_index;
    const size_t rows = strlen(argv[2]) + 1;
    const size_t columns = strlen(argv[1]) + 1;
    const int score_hash[5][5] = {
        5, -4, -4, -4, -4,
        -4, 5, -4, -4, -4,
        -4, -4, 5, -4, -4,
        -4, -4, -4, 5, -4,
        -4, -4, -4, -4, -4}; // ACGTN
    int scores[4];                // max score, left score, left & top score, top score
    short source;                    // left -1, left & top 0, top 1

    unsigned short *row = malloc(sizeof(unsigned short) * columns);
    unsigned short *column = malloc(sizeof(unsigned short) * rows);

    string2integers(argv[1], columns - 1, row); // row string
    string2integers(argv[2], rows - 1, column); // column string

    // printf("rows: %lu\ncolumns: %lu\n", rows, columns);

    int **score_table = (int **)malloc(sizeof(int *) * rows);
    short **path_table = (short **)malloc(sizeof(short *) * rows);
    for (row_index = 0; row_index < rows; row_index++)
    {
        score_table[row_index] = malloc(sizeof(int) * columns);
        memset(score_table[row_index], 0, columns);
        path_table[row_index] = malloc(sizeof(short) * columns);
        memset(score_table[row_index], 0, columns);
    };

    for (row_index = 1; row_index < rows; row_index++)
    {
        for (column_index = 1; column_index < columns; column_index++)
        {
            scores[1] = score_table[row_index][column_index - 1] + (path_table[row_index][column_index - 1] ? GAP_EXTENSION : GAP_OPEN); // gap
            scores[2] = score_table[row_index - 1][column_index - 1] + score_hash[row[column_index]][column[row_index]];                 // match or mismatch
            scores[3] = score_table[row_index - 1][column_index] + (path_table[row_index - 1][column_index] ? GAP_EXTENSION : GAP_OPEN); // gap
            get_source(scores, &source);                                                                                                 // save max score to scores[0] and get source -1|0|1
            score_table[row_index][column_index] = scores[0];
            path_table[row_index][column_index] = source;
            // printf("scores: %f, %f, %f, max: %f, max: %f\n", scores[1], scores[2], scores[3], scores[0], score_table[row_index][column_index]);
        };
    };
    PATH *start_node = NULL;
    int score = get_path(score_table, path_table, rows, columns, argv[1], argv[2], &start_node);
    printf("alignment score: %d\n", score);
    PATH *node = start_node;
    while (node)
    {
        fputc(node->pair[0], stdout);
        node = node->next;
    }
    node = start_node;
    fputc('\n', stdout);
    while (node)
    {
        fputc(node->pair[1], stdout);
        node = node->next;
    }
    fputc('\n', stdout);

    /* release */
    free(row);
    free(column);
    for (row_index = 0; row_index < rows; row_index++)
    {
        free(score_table[row_index]);
        free(path_table[row_index]);
    };
    free(score_table);
    free(path_table);
    PATH *temp_node = start_node;
    while (start_node->next)
    {
        temp_node = start_node->next;
        free(start_node);
        start_node = temp_node;
    }
    return (0);
}

int string2integers(const char *input, size_t input_size, unsigned short *output)
{
    char temp;
    output[0] = 4;
    for (size_t index = 0; index < input_size; index++)
    {
        temp = input[index];
        if (temp == 'A' || temp == 'a')
            output[index + 1] = 0;
        else if (temp == 'C' || temp == 'c')
            output[index + 1] = 1;
        else if (temp == 'G' || temp == 'g')
            output[index + 1] = 2;
        else if (temp == 'T' || temp == 't')
            output[index + 1] = 3;
        else
            output[index + 1] = 4;
    };
    return (0);
}

int get_source(int *numbers, short *source)
{
    numbers[0] = 0;
    if (numbers[1] >= numbers[0] && numbers[1] >= numbers[3])
    {
        numbers[0] = numbers[1];
        *source = -1;
    }
    else if (numbers[3] >= numbers[0] && numbers[3] >= numbers[1])
    {
        numbers[0] = numbers[3];
        *source = 1;
    };
    if (numbers[2] >= numbers[0])
    {
        numbers[0] = numbers[2];
        *source = 0;
    };
    return (0);
}

int get_path(int **score_table, short **path_table, const size_t rows, const size_t columns, const char *row, const char *column, PATH **start_node)
{
    int score = -1, temp_score;
    size_t row_index, column_index, score_row_index, score_column_index;

    for (row_index = 0; row_index < rows; row_index++)
    {
        for (column_index = 0; column_index < columns; column_index++)
        {
            temp_score = score_table[row_index][column_index];
            if (temp_score > score)
            {
                score = temp_score;
                score_row_index = row_index;
                score_column_index = column_index;
            };
        };
    };
    // printf("(%lu, %lu): %f\n", score_row_index, score_column_index, score);

    PATH *previous_node = NULL;

    while (score_row_index && score_column_index)
    {
        PATH *node = malloc(sizeof(PATH));
        switch (path_table[score_row_index][score_column_index])
        {
        case -1:
        {
            score_column_index--;
            node->pair[0] = row[score_column_index];
            node->pair[1] = '-';
        }
        break;
        case 0:
        {
            score_row_index--;
            score_column_index--;
            node->pair[0] = row[score_column_index];
            node->pair[1] = column[score_row_index];
        }
        break;
        case 1:
        {
            score_row_index--;
            node->pair[0] = '-';
            node->pair[1] = column[score_row_index];
        }
        break;
        };
        node->next = previous_node;
        previous_node = node;
    }
    *start_node = previous_node;
    return (score);
}
