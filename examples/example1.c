#include <csv.h>

int main () {

        CSV_BUFFER *my_buffer = csv_create_buffer();
        printf("created buffer\n");
        csv_load(my_buffer, "testlist.csv");
        printf("loaded from file\n");
        csv_set_field(my_buffer, 10, 9, "test");
        printf("set first field to \"test\"\n");
        csv_save("testsave.csv", my_buffer);
        printf("saved buffer\n");
        size_t my_string_size = 10;
        char *my_string = malloc(my_string_size + 1);
        csv_get_field(my_string, my_string_size, my_buffer, 10, 9);
        printf("Got string = \"%s\"\n", my_string);
        csv_destroy_buffer(my_buffer);
        printf("destroyed buffer\n");
        free(my_string);
        printf("Free'd string\n");

return 0;
}
