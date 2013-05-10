#ifndef _CSV_H
#define _CSV_H

#include <stdlib.h>
#include <stdio.h>

typedef struct CSV_FIELD {
        char *text;
        size_t length;
} CSV_FIELD;

typedef struct CSV_BUFFER {
        CSV_FIELD ***field;
        size_t rows;
        size_t *width; 
        char field_delim;
        char text_delim;
} CSV_BUFFER;

/* Function: add_char
 * ------------------
 * Appends a character to the end of a string, and increases 
 * c by one.
 * c = length of string (including '\0')
 * ch = character to be added to the string.
 *
 * Returns:
 * 0: success
 * 1: realloc failure
 */
static int add_char(char **string, int *c, char ch);

/* Function: create_field 
 * ------------------------
 * Should be called once on every CSV_FIELD used. Allocates
 * memory for the field. Length is set to 0 and text to NULL
 * 
 * Returns NULL on error via malloc.
 */
static CSV_FIELD *create_field();

/* Function: destroy_field
 * ---------------------------
 * Frees CSV_FIELD memory. If the field has been initialized
 * but not set, field->text has not been malloc'd, and so
 * is not freed.
 * 
 * Returns:
 * 0: success
 * 1: error realloc'ing field's char array
 */
static void destroy_field(CSV_FIELD *field);

/* Function: set_field
 * -----------------------
 * Sets a field text to the string provided. Adjusts field
 * length accordingly. 
 * 
 * Returns:
 *  0: success
 *  1: error allocating space to the string
 */
static int set_field(CSV_FIELD *field, char *text);

/* Function: csv_create_buffer
 * ---------------------------
 * Must be called before any declared buffer is used. 
 * Allocates memory for the buffer.
 * Sets the number of rows to 0
 * Sets Text Delim to '"' and field delim to ',' by default.
 */
CSV_BUFFER *csv_create_buffer();

/* Function: csv_destroy_buffer
 * ----------------------------
 * Frees memory allocated by csv_create_buffer and any fields
 * that are part of the buffer.
 */
void csv_destroy_buffer();

/* Function: append_row
 * -------------------------------
 * Adds a "row" to the end of a CSV_BUFFER. The row is 
 * initialized with no fields. Also adds an integer to the
 * width array and sets width = 0.
 *
 * Returns:
 * 0: success
 * 1: error allocating width memory
 * 2: error allocating field memory
 */
static int append_row(CSV_BUFFER *buffer);

/* Function: append_field
 * ---------------------------------
 * Adds a field to the end of a given row in a CSV_BUFFER. 
 * The field is initialized using csv_create_buffer.
 *
 * Returns:
 * 0: success
 * 1: the given row does not extist
 * 2: memory allocation error 
 */
static int append_field(CSV_BUFFER *buffer, size_t row);

/* Function: remove_last_field
 * -------------------------------
 * Removes the field at the end of a given row.
 * 
 * Returns:
 *  0: success
 *  1: the requested row is already empty
 *  2: the requested row does not exist
 */
static int remove_last_field(CSV_BUFFER *buffer, size_t row);

/* Function: remove_last_row
 * -----------------------------
 * Removes the final row of the buffer.
 *
 * Returns:
 *  0: success
 */
static int remove_last_row(CSV_BUFFER *buffer);

/* Function: read_next_field
 * -----------------------------
 * Moves the file pointer to the beginning of the next 
 * entry. If not NULL, the length and text are copied to the
 * field provided.
 *
 * Note that consecutive field delimenators indicate empty
 * cells and lines ending with a delimenator (before the
 * new line characte)  are interpreted as having a trailing 
 * empty cell. 
 *
 * Text-deliminated cells may contain text deliminator, field
 * deliminator, and newline characers. Characters between the
 * end of a text deliminaton and the field deliminator (or
 * newline or EOF) are ignored. 
 *
 * Returns: 
 *  0: Moved successfully to the next entry in this row  
 *  1: The next entry is on a new row 
 *  2: There is no next entry (EOF)
 */
static int read_next_field(FILE *fp,
                char field_delim, char text_delim, 
                CSV_FIELD *field);

/* Function: csv_load
 * -----------------------
 * Loads the given file into the buffer.
 *
 * Returns:
 *  0: success
 *  1: file not found
 *  2: failure to resize buffer (memory failure)
 */
int csv_load(CSV_BUFFER *buffer, char *file_name);

/* Function: csv_save
 * -----------------------
 * Saves the csv buffer to a given file. If the file already
 * exists, it is overwritten.
 *
 * Returns:
 *  0: success
 *  1: unable to write to file (invalid name or inufficient
 *     access)
 */
int csv_save(char *file_name, CSV_BUFFER *buffer);

/* Function: csv_copy_row
 * ----------------------
 * Deep copy of a row of a CSV_BUFFER. Destination row may
 * be in the same buffer as source. Adjusts the length of 
 * the row.
 *
 * Returns:
 *  0: success
 *  1: the requested src row does not exist
 */
int csv_copy_row(CSV_BUFFER *dest, int dest_row,
                        CSV_BUFFER *src, int src_row);

/* Function: copy_field
 * ------------------------
 * 
 * Copies a given field from one buffer to another (note, source and
 * dest buffers MAY be the same.
 * 
 * Returns:
 *  0: success
 *  1: memory error (see set_field)
 */
int csv_copy_field(CSV_BUFFER *dest, int dest_row, int dest_entry,
                   CSV_BUFFER *source, int source_row, int source_entry);


/* Function: csv_get_field
 * ----------------------------------
 * Copies an entry from a CSV_BUFFER to a string provided.
 * The caller is expected to provide the string's length.
 * If the requested cell does not exist, or is empty, the
 * string is filled with null characters.
 *
 * Returns:
 *  0: the whole entry was copied
 *  1: the entry was trucated to fit the string
 *  2: the request cell was empty (or does not exist)
 *  3: the length given was 0
 */
int csv_get_field(char *dest, size_t dest_len, 
        CSV_BUFFER *src, size_t row, size_t entry);

/* Function: csv_clear_field
 * -------------------------
 * 
 * If the field is the last in the row (and not the first), it is
 * removed (the width of the row is decremented by one). Otherwise,
 * it is just cleared, and a spacer remains.
 *
 * Returns:
 *  0: success
 *  TODO errors
 */
int csv_clear_field(CSV_BUFFER *buffer, size_t row, size_t entry);

/* Function: csv_clear_row
 * -----------------------
 * 
 * Destroys all but one field in the row, and clears that field.
 * Row size is now 1.
 *
 * Returns:
 *  0: success
 *  1: memory allocation falirure (note this function only 
 *      reduces memory used, so reallocation should never fail)
 */
int csv_clear_row(CSV_BUFFER *buffer, size_t row);

/* Function: csv_remove_row
 * ------------------------
 * 
 * Completely removes a row from the buffer such that it's two
 * neighboring rows are now adjacent and buffer height is reduced
 * by one.
 *
 * Returns:
 *  0: success
 *  1: memory allocation falirure (note this function only 
 *      reduces memory used, so reallocation should never fail)
 */
int csv_remove_row(CSV_BUFFER *buffer, size_t row);

void csv_set_text_delim(CSV_BUFFER *buffer, char new_delim);

void csv_set_field_delim(CSV_BUFFER *buffer, char new_delim);

int csv_get_height(CSV_BUFFER *buffer);
/* Returns: height of buffer */

int csv_get_width(CSV_BUFFER *bufer, size_t row);
/* Returns: width of row (or 0 if row does not exist) */

int csv_get_field_length(CSV_BUFFER *buffer, size_t row, size_t entry);

int csv_set_field(CSV_BUFFER *buffer, size_t row, size_t entry,
        char *field);

int csv_insret_field(CSV_BUFFER *buffer, size_t row, size_t entry,
        char *field);

#endif /* CSV_H_ */
