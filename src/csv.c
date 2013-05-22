#include "csv.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static int add_char(char **string, int *c, char ch)
{
        char *tmp = NULL;
        (*c)++;
        tmp = realloc(*string, (*c)+1);
        if (tmp == NULL)
                return 1;
        *string = tmp;
        (*string)[(*c)-1] = ch; 
        (*string)[*c] = '\0';

        return 0;
}

static CSV_FIELD *create_field()
{
        CSV_FIELD *field = malloc(sizeof(CSV_FIELD));
        field->length = 0;
        field->text = NULL;
        set_field(field, "\0");
        return field;
}

static void destroy_field(CSV_FIELD *field)
{
        if (field->text != NULL) {
                free(field->text);
                field->text = NULL;
        }
        free(field);
        field = NULL;
} 

static int set_field(CSV_FIELD *field, char *text)
{
        
        char *tmp;

        field->length = strlen(text) + 1;
        tmp = realloc(field->text, field->length);
        if (tmp == NULL)
                return 1;
        field->text = tmp;
        strcpy(field->text, text);

        return 0;
}

static int read_next_field(FILE *fp,
                char field_delim, char text_delim, 
                CSV_FIELD *field)
{

        char ch = 'a';

        bool done = false;
        bool in_text = false;
        bool esc = false;

        int c = 0;
        char *tmp = malloc(1);
        tmp[0] = '\0';
        while (!done) {
                ch = getc(fp);

                if (ch == EOF) {
                        c = 0;
                        done = true;
                } else if (!in_text) {
                        if (ch == text_delim) {
                                in_text = true;
                                c = 0;
                        } else if (ch == field_delim) {
                                done = true; 
                        } else if (ch == '\n') {
                                done = true;
                        } else { 
                               add_char(&tmp, &c, ch);
                        }  
                } else { /* in_text == true */
                        if (esc) {
                                if (ch == text_delim) {
                                        add_char(&tmp, &c, ch);
                                        esc = false;
                                } else {
                                        esc = false;
                                        done = true;
                                }
                        } else { /* !esc */
                                if (ch == text_delim) {
                                        esc = true;
                                } else if (ch == field_delim) {
                                        add_char(&tmp, &c, ch);
                                } else {
                                        add_char(&tmp, &c, ch);
                                }
                        }
                }
        } 
        if (field != NULL){
                set_field(field, tmp);
         }

        if (tmp != NULL)
                free(tmp);
        tmp = NULL;

        /* Moving the fp to the beginning of the next field and peeking 
         * to see if it is a new line or if there is in fact no next field.
         */
        fpos_t pos;
        int retval;
        done = false;
        while (!done) {
                if (ch == field_delim) {
                        retval = 0;
                        done = true;
                }
                else if (ch == '\n') {
                        /* Checking to see if this \n is the one that UNIX
                         * sometimes includes before the EOF */
                        fgetpos(fp, &pos);
                        ch = getc(fp);
                        if (ch == EOF)
                                retval = 2;
                        else
                                retval = 1;
                        fsetpos(fp, &pos);
                        done = true;
                }
                else if (ch == EOF) {
                        retval = 2;
                        done = true;
                } else {
                        ch = getc(fp);
                }
        }

        return retval;
}

static int append_field(CSV_BUFFER *buffer, size_t row)
{

        CSV_FIELD **temp_field;

        if (buffer->rows < row + 1)
                return 1;

        /* Set col equal to the index of the new field */
        int col = buffer->width[row];

        temp_field = realloc(buffer->field[row], 
                        (col + 1) * sizeof(CSV_FIELD*));
        if (temp_field == NULL) {
                return 2;
        } else {
                buffer->field[row] = temp_field;
                buffer->field[row][col] = create_field(); 
                buffer->width[row]++;
        } 

        return 0;
}

static int append_row(CSV_BUFFER *buffer)
{
        size_t *temp_width;
        CSV_FIELD ***temp_field;

        size_t row  = buffer->rows;

        temp_width = realloc(buffer->width, (buffer->rows + 1) * 
                        sizeof(size_t));
        if (temp_width != NULL) { 
                buffer->width = temp_width;
                buffer->width[row] = 0;
        } else {
                return 1;
        }

        temp_field = realloc(buffer->field, (buffer->rows + 1) * 
                        sizeof(CSV_FIELD**));
        if (temp_field != NULL) {
                buffer->field = temp_field;
                buffer->field[row] = NULL;
        }
        else {
                free(temp_width);
                return 2;
        }

        buffer->rows++;
        append_field(buffer, row);
        return 0;
}

static int remove_last_field(CSV_BUFFER *buffer, size_t row)
{

        size_t entry = buffer->width[row] - 1;

        CSV_FIELD **temp_row;

        /* If there are no entries in the row there is nothing to
         * remove, but return success because this is expected */
        if (row > buffer->rows - 1) 
                return 0;
        /* If t he row exists, but has no fields, something went
         * wrong; every row in the scope should have at least one
         * field. 
         */
        else if (buffer->width[row] == 0)
                return 1;
        /* If there is only one entry left, just clear it. If the 
         * caller wants to remove the row, she will call
         * csv_remove_row.
         */
        else if (buffer->width[row] == 1) {
                csv_clear_field(buffer, row, 0);
                return 0;
        }
        /* Otherwise destroy the final field and decrement the width */
        else {
                destroy_field(buffer->field[row][entry]);
                temp_row = realloc(buffer->field[row], entry
                                * sizeof (CSV_FIELD*));
                if (temp_row != NULL)
                        buffer->field[row] = temp_row;
                else
                        return 3; 
                buffer->width[row]--;
        }

        return 0;
}

static int remove_last_row(CSV_BUFFER *buffer)
{

        int row = buffer->rows - 1;
        int entry = buffer->width[row] - 1;

        CSV_FIELD ***temp_field = NULL;
        size_t *temp_width = NULL;

        while (entry >= 0) {
                remove_last_field(buffer, row);
                entry--;
        } 

        temp_field = realloc(buffer->field, (buffer->rows - 1) *
                        sizeof(CSV_FIELD**));
        temp_width = realloc(buffer->width, (buffer->rows - 1) *
                        sizeof(size_t)); 
        if (temp_width == NULL || temp_field == NULL)
                return 1;

        buffer->field = temp_field;
        buffer->width = temp_width;
        buffer->rows--;

        return 0;
}

CSV_BUFFER *csv_create_buffer()
{

        CSV_BUFFER *buffer = malloc(sizeof(CSV_BUFFER));

        if (buffer != NULL) {
                buffer->field = NULL;
                buffer->rows = 0;
                buffer->width = NULL;
                buffer->field_delim = ',';
                buffer->text_delim = '"';
        }

        return buffer;
}

void csv_destroy_buffer(CSV_BUFFER *buffer)
{

        int i, j;

        for (i = 0; i < buffer->rows; i++) {
                for (j = 0; j < buffer->width[i]; j++) {
                        destroy_field(buffer->field[i][j]);
                }
                free(buffer->field[i]);
                buffer->field[i] = NULL;
        }

        if (buffer->field != NULL)
                free(buffer->field);

        if (buffer->width != NULL)
                free(buffer->width);

        free(buffer);
}

int csv_load(CSV_BUFFER *buffer, char *file_name)
{

        FILE *fp = fopen(file_name, "r");
        if (fp == NULL)
                return 1;

        int next = 1;
        bool end = false;
        bool first = true;
        int i = -1, j = -1;

        while (!end) {

                if (!first) {
                        next = read_next_field(fp, 
                                        buffer->field_delim, buffer->text_delim,
                                        buffer->field[i][j-1]);
                }

                if (next == 2) 
                        end = true;

                if (next == 1) {
                        if (append_row(buffer) != 0)
                                return 2;
                        j = 1;
                        i++;
                }

                if (next == 0) {
                        if (append_field(buffer, i) != 0) 
                                return 2;
                        j++;
                }


                if (first) first = false;
        }

        fclose(fp);
        return 0;
}

int csv_save(char *file_name, CSV_BUFFER *buffer)
{

        int i, j, k;
        char *chloc; 
        FILE *fp = fopen(file_name, "w");
        if (fp == NULL)
                return 1;
        char text_delim = buffer->text_delim;
        char field_delim = buffer->field_delim;
        for(i = 0; i < buffer->rows; i++) {
                for(j = 0; j < buffer->width[i]; j++) {
                        chloc = strchr(buffer->field[i][j]->text, text_delim);
                        if(chloc == NULL)
                                chloc = strchr(buffer->field[i][j]->text, field_delim);
                        if(chloc == NULL)
                                chloc = strchr(buffer->field[i][j]->text, '\n');
                        /* if any of the above characters are found, chloc will be set
                         * and we must use text deliminators.
                         */
                        if(chloc != NULL) {
                                fputc(text_delim, fp);
                                for(k = 0; k < buffer->field[i][j]->length - 1; k++) {
                                        /* if there are any text delims in the string,
                                         * we must escape them.
                                         */
                                        if(buffer->field[i][j]->text[k] == text_delim)
                                                fputc(text_delim, fp);
                                        fputc(buffer->field[i][j]->text[k], fp);
                                }
                                fputc(text_delim, fp);
                                chloc = NULL;
                        } else {
                                fputs(buffer->field[i][j]->text, fp);
                        }
                        if(j < buffer->width[i] - 1)
                                fputc(field_delim, fp);
                        else if (i < buffer->rows - 1)
                                fputc('\n', fp);
                }
        }

        fclose(fp);
        return 0;
}

int csv_get_field(char *dest, size_t dest_len, 
        CSV_BUFFER *src, size_t row, size_t entry)
{
        int i;
        if (dest_len == 0)
                return 3;
        if (row >= src->rows /*row does not exist*/
            || row < 0
            || entry >= src->width[row] /*entry does not exist*/
            || entry < 0) {
                for (i = 0; i < dest_len; i++)
                        dest[0] = '\0';
                /* If the requested entry does not exist or is
                 * invalid, we clear the string provided consistent
                 * with the case of an empty entry.
                 */
                return 2;
        } else {

        /* If destination is not large enough to hold the whole entry,
         * strncpy will truncate it for us. 
         */
                strncpy(dest, src->field[row][entry]->text, dest_len);
                dest[dest_len] = '\0';
        }

        if (src->field[row][entry]->length > dest_len + 1)
                return 1;
        if (src->field[row][entry]->length == 0)
                return 2;
        else         
                return 0;
}

int csv_copy_field(CSV_BUFFER *dest, int dest_row, int dest_entry,
                   CSV_BUFFER *source, int source_row, int source_entry)
{
        return set_field(dest->field[dest_row][dest_entry],
                        source->field[source_row][source_entry]->text);
}

int csv_clear_field(CSV_BUFFER *buffer, size_t row, size_t entry)
{
        /* Field is already clear (out of range) */
        if (buffer->rows < row + 1 || buffer->width[row] < entry + 1)
                return 0;

        /* Destroy the field if it is last in the row (and now field 0) */ 
        if (entry == buffer->width[row] - 1 && entry != 0)
                remove_last_field(buffer, row);

        else
               set_field(buffer->field[row][entry], "\0");

        return 0; 
}

int csv_clear_row(CSV_BUFFER *buffer, size_t row)
{

        CSV_FIELD **temp_row;

        /* If the requested field is the last one, simply remove it. */
        if (row == buffer->rows-1) {
                if (remove_last_row(buffer) != 0)
                        return 1;
                else
                        return 0;
        }

        /* Destroy every field but the last one */
        int i;
        for (i = buffer->width[row] - 1; i > 0; i--) {
                destroy_field(buffer->field[row][i]);
        }
        /* Clear the last field */
        set_field(buffer->field[row][0], "\0");

        temp_row = realloc(buffer->field[row], sizeof (CSV_FIELD*));
        /* If it didn't shrink, recreate the destroyed fields */
        if (temp_row == NULL) { 
                for (i = 1; i < buffer->width[row]; i++) {
                        append_field(buffer, row);
                        set_field(buffer->field[row][i], "\0");
                }
                return 1;
        } else {
                buffer->field[row] = temp_row;
        }

        buffer->width[row] = 1;

        return 0;
}

int csv_copy_row(CSV_BUFFER *dest, int dest_row, 
                        CSV_BUFFER *source, int source_row)
{

        if (source_row > source->rows - 1) {
                csv_clear_row(dest, dest_row);
                return 0;
        }

        while (dest->rows < (dest_row + 1)) 
               if(append_row(dest) != 0)
                        return 1;

        while (dest->width[dest_row] > source->width[source_row])
                if(remove_last_field(dest, dest_row) != 0)
                        return 1;
        
        while (dest->width[dest_row] < source->width[source_row])
                if(append_field(dest, dest_row) != 0)
                        return 1;

        int i;
        for(i = 0; i < dest->width[dest_row]; i++)
                csv_copy_field(dest, dest_row, i, source, source_row, i);
        
        return 0;
}

int csv_remove_row(CSV_BUFFER *buffer, size_t row)
{

        if(row > buffer->rows - 1)
                return 0;

        int i;
        for (i = row; i < buffer->rows-1; i++) 
                csv_copy_row(buffer, i, buffer, i + 1);
        remove_last_row(buffer);

        return 0;

}

int csv_remove_field(CSV_BUFFER *buffer, size_t row, size_t entry)
{
        if (row > buffer->rows - 1 || entry > buffer->width[row] - 1)
                return 0;
        
        int i;
        for (i = entry; i < buffer->width[row]-1; i++)
                csv_copy_field(buffer, row, i, buffer, row, i + 1);
        remove_last_field(buffer, row);

        return 0;
}

void csv_set_text_delim(CSV_BUFFER *buffer, char new_delim)
{
        buffer->text_delim = new_delim;
}

void csv_set_field_delim(CSV_BUFFER *buffer, char new_delim)
{
        buffer->field_delim = new_delim;
}

int csv_get_height(CSV_BUFFER *buffer)
{
        return buffer->rows;
}

int csv_get_width(CSV_BUFFER *buffer, size_t row)
{
        if (row > buffer->rows - 1)
                return 0;
        else 
                return buffer->width[row];
}

int csv_get_field_length(CSV_BUFFER *buffer, size_t row, size_t entry)
{

        if (row > buffer->rows - 1)
                return 0;
        else if (entry > buffer->width[row] - 1)
                return 0;
        else 
                return buffer->field[row][entry]->length - 1;
}

int csv_set_field(CSV_BUFFER *buffer, size_t row, size_t entry,
                char *field)
{

        while (row >= buffer->rows) {
                append_row(buffer);
        }
        while (entry >= buffer->width[row])
                append_field(buffer, row);

        if (set_field(buffer->field[row][entry], field) == 0)
                return 0;
        else 
                return 1;
}

int csv_insert_field(CSV_BUFFER *buffer, size_t row, size_t entry,
                char *field)
{
        /* If the field does not exist, simply set it */
        if (row > buffer->rows - 1
                || entry > buffer->width[row] - 1)
                csv_set_field(buffer, row, entry, field);

        /* Otherwise move everything over, then set it */
        else {
                append_field(buffer, row);
                int i = 0;
                for (i = buffer->width[row] - 1; i > entry; i--)
                        csv_copy_field(buffer, row, i,
                                        buffer, row, i-1);
                csv_set_field(buffer,row,entry,field);
        }

        return 0;
}
