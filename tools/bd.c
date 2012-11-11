/* bd - output standard input bytes as binary */
/* Iestyn Pryce 2012 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t buf;
	int	len;
} Buffer;

/* Return the bit value from the byte that the buffer counter is currently *
 * pointing at 								   */
bool get_bit(Buffer *b) {
        bool ret;
        uint8_t i = 0x01;

        /* Return the bit that the buffer is currently pointing to */
        i <<= (8*sizeof(b->buf) - b->len - 1);
        i &=  b->buf;
        i >>= (8*sizeof(b->buf) - b->len - 1);

        if (i == 1) {
                ret = true;
        } else {
                ret = false;
        }

        return ret;
}

int main(void) {

	FILE *in, *out;
	in = stdin;
	out = stdout;
	
	int i = 0;

	Buffer b;
	while(fread(&b.buf,1,sizeof(b.buf),in)) {
		printf("%08d\t",i++);
		b.len = 0;
		while (b.len < 8*sizeof(b.buf)) {
			fprintf(out,"%d",get_bit(&b));
			b.len++;
		}
		fprintf(out,"\n");
	}

	return 0;
}
