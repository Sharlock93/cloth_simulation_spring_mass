#ifndef SH_UTILS_H
#define SH_UTILS_H

#include <stdint.h>
#include <Windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;


typedef float  f32;
typedef double f64;


u8* read_entire_file(const char *file_name, size_t *size) {
	char *file_content = NULL;

	HANDLE file = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE) {
		//handle error
		CloseHandle(file);
		return NULL;
	}

	DWORD s = GetFileSize(file, NULL);
	if(size != NULL) {
		*size = s;
	}

	if(s != INVALID_FILE_SIZE) {
		u8 *file_cont = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, s+1);
		DWORD read_amount = 0;
		ReadFile(file, file_cont, s, &read_amount, NULL);

		file_cont[s] = 0; // nullterminate
		CloseHandle(file);
		if(read_amount != s) {
			return NULL;
		}

		return file_cont;
	}

	CloseHandle(file);
	return NULL;
}

#define buf_hdr_ptr(buf) ((int *)(buf) - 2)
#define buf__size(buf)   (buf_hdr_ptr(buf)[1])
#define buf__cap(buf)    (buf_hdr_ptr(buf)[0])
#define buf_size(buf)    ((buf) ? buf__size(buf) : 0)
#define buf_cap(buf)     ((buf) ? buf__cap(buf) : 0)
#define has_size(buf, n) ((buf) ? buf__size(buf)+n <= buf__cap(buf) : 0)
#define buf_grow(buf, n) (*(void **)&buf = buf__grow(buf, n, sizeof(buf[0])))
#define buf_push(buf, item) (has_size((buf), 1) ? 0 : buf_grow((buf), 1), (buf)[buf__size(buf)++] = item)
#define buf_reset(buf) (buf_hdr_ptr(buf)[1] = 0)

// stretchy buffer
void* buf__grow(void *buf, int buf_new_cap, int item_size) {
	if(buf == NULL) {
		void *temp_buf = malloc(2*buf_new_cap*item_size + sizeof(int)*2);
		((int*)temp_buf)[0] = 2*buf_new_cap;
		((int*)temp_buf)[1] = 0;

		return (void *) ((int *)temp_buf+2);
	} else {
        //it has value get header
        void *t_buf = (void *) buf_hdr_ptr(buf);
		int old_cap = buf_cap(buf);
		void *temp_buf = realloc(t_buf, 2*old_cap*item_size + sizeof(int)*2);
		((int*)temp_buf)[0] = 2*old_cap;
		return (void *) ((int *)temp_buf+2);
	}
}

#endif
