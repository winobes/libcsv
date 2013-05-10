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
        csv_destroy_buffer(my_buffer);
        printf("destroyed buffer\n");

return 0;
}
