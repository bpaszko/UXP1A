#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <unistd.h>

static int TUPLE_DATA_SIZE = 8;
static int TUPLE_SIZE = 0x40;
static int META_SIZE = 0x100;
static char NULL_SIGN = 0xFF;
static int STRING_SIZE = 0x40;

typedef enum data_type {
	NO_DATA = -1,
	DATA_INT = 1,
	DATA_FLOAT = 2,
	DATA_STRING = 3
} data_type;

union data{
	int data_int;
	float data_float;
	char* data_string;
};

typedef struct {
	data_type type;
	data data_union;
} Tuple_Data;
//8 bytes

struct Tuple {
	Tuple_Data data[8];
	Tuple() { for(int i = 0; i < 8; ++i) this->data[i].type = data_type::NO_DATA; }
};
//64 bytes

typedef struct {
	char pattern[543];
	sem_t* sem;
} Pattern_Pair;

typedef struct {
	int shared_size;
	int tuple_array_offset;
	int string_array_offset;
	int waiting_array_offset;
	sem_t* main_sem;
} Meta_Data;
