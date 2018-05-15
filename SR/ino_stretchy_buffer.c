#include <malloc.h>  // Fixme: arena allocation for the stretchy buffer

#ifdef offsetof
#undef offsetof
#define offsetof(type, field) ((size_t)&(((type *)0)->field)) 
#endif

#define sb__header(buf) ((sb_header *)((byte *)buf - offsetof(sb_header, data))) 
#define sb__not_empty(buf) (sb_len(buf) > 0)
#define sb__fits(buf, n) (sb_len(buf) + (n) <= sb_cap(buf))
#define sb__fit(buf, n) (sb__fits(buf, n) ? 0 : ((buf) = sb__grow(buf, sizeof(*buf)))) 
//#define sb__fit(buf, n) (sb__fits(buf, n) ? 0 : (buf) = sb__grow(buf, sb_len(buf)+sizeof(sb_header)+n, sizeof(*buf))) 
#define sb__pow(buf) ((((buf)[sb_len(buf)] & -(buf)[sb_len(buf)]) == ((buf)[sb_len(buf)])) ? sb__clear(buf, 0x8b, sizeof(*buf)) : 0) 

#define sb_len(buf) (((buf)) ? sb__header((buf))->len : 0)
#define sb_cap(buf) (((buf)) ? sb__header((buf))->cap : 0)
#define sb_push(buf, x) (sb__fit(buf, 1), (buf)[sb_len(buf)] = (x), sb__header(buf)->len++) 
#define sb_pop(buf) (((buf) && sb__not_empty(buf)) ? (--sb__header(buf)->len, sb__pow(buf)) : (void)0) 
#define sb_free(buf) ((buf) ? (free(sb__header(buf)), (buf) = NULL) : 0)

#ifdef __cplusplus
extern "C" {
#endif

// 16 bytes (x64)
typedef struct {
   size_t len, cap;
   byte data[1];
} sb_header;

static void *sb__grow(const void *old_buf, size_t elem_size)
{
   sb_header *new_header;
   size_t new_cap, bytes_to_alloc;

   Assert(elem_size != 0);
   new_cap = MAX(sb_cap(old_buf)*2, 1);
   bytes_to_alloc = new_cap*elem_size + sizeof(sb_header);
   Assert(bytes_to_alloc > 0);
   if (old_buf) {
      new_header = (sb_header *)realloc(sb__header(old_buf), bytes_to_alloc);
   } else {
      new_header = (sb_header *)malloc(bytes_to_alloc);
      new_header->len = 0;
   }

   Assert(new_header);
   new_header->cap = new_cap;

   return new_header->data;
}

static void sb__clear(void *buf, int val, size_t elem_size)
{
   size_t pow2_len = sb_len(buf);
   size_t next_pow2_len = sb_len(buf)*2;
   memset((byte *)buf + pow2_len*elem_size, val, (next_pow2_len-pow2_len+1)*elem_size);
}

#ifdef __cplusplus
}
#endif

