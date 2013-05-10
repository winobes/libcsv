#ifndef _CSV_H
#define _CSV_H

#include <stdlib.h>
#include <stdio.h>

typedef struct CSV_BUFFER CSV_BUFFER;

CSV_BUFFER *csv_create_buffer();
void csv_destroy_buffer();

int csv_load(CSV_BUFFER *buffer, char *file_name);
int csv_save(char *file_name, CSV_BUFFER *buffer);

void csv_set_text_delim(CSV_BUFFER *buffer, char new_delim);
void csv_set_field_delim(CSV_BUFFER *buffer, char new_delim);

int csv_get_height(CSV_BUFFER *buffer);
int csv_get_width(CSV_BUFFER *bufer, size_t row);

int csv_get_field(char *dest, size_t dest_len, 
                CSV_BUFFER *source, size_t row, size_t entry);
int csv_get_field_length(CSV_BUFFER *buffer, size_t row, size_t entry);

int csv_copy_row(CSV_BUFFER *dest, int dest_row, 
                CSV_BUFFER *source, int source_row);
int csv_clear_row(CSV_BUFFER *buffer, size_t row);
int csv_remove_row(CSV_BUFFER *buffer, size_t row);

int csv_copy_field(CSV_BUFFER *dest, int dest_row, int dest_entry,
                CSV_BUFFER *source, int source_row, int source_entry);
int csv_clear_field(CSV_BUFFER *buffer, size_t row, size_t entry);
int csv_remove_field(CSV_BUFFER *buffer, size_t row, size_t entry);

int csv_set_field(CSV_BUFFER *buffer, size_t row, size_t entry,
        char *field);
int csv_insret_field(CSV_BUFFER *buffer, size_t row, size_t entry,
        char *field);


#endif /* CSV_H_ */
