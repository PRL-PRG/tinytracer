typedef struct {
    unsigned long *array;
    size_t used;
    size_t size;
} ext_unsigned_long_array;

void init_unsigned_long(ext_unsigned_long_array *, size_t initial_size);

void insert_unsigned_long(ext_unsigned_long_array *, int index, unsigned long element);

unsigned long get_unsigned_long(ext_unsigned_long_array *, int index);

unsigned long increment_unsigned_long(ext_unsigned_long_array *, int index);

void free_unsigned_long(ext_unsigned_long_array *);