#include <stdio.h>

extern void decode (unsigned char);

int main (void)
{
	unsigned a, b, c, d;

	asm ("movl $0x2, %%eax\n\t"
             "cpuid"
             :"=a"(a),"=b"(b),"=c"(c),"=d"(d));

	if ((a >> 31) == 0) {
		decode ((unsigned char)((a >> 8) & 0xff));
		decode ((unsigned char)((a >> 16) & 0xff));
		decode ((unsigned char)((a >> 24) & 0xff));
	}
	if ((b >> 31) == 0) {
		decode ((unsigned char)(b & 0xff));
		decode ((unsigned char)((b >> 8) & 0xff));
		decode ((unsigned char)((b >> 16) & 0xff));
                decode ((unsigned char)((b >> 24) & 0xff));
	}
	if ((c >> 31) == 0) {
                decode ((unsigned char)(c & 0xff));
                decode ((unsigned char)((c >> 8) & 0xff));
                decode ((unsigned char)((c >> 16) & 0xff));
                decode ((unsigned char)((c >> 24) & 0xff));
        }
	if ((d >> 31) == 0) {
                decode ((unsigned char)(d & 0xff));
                decode ((unsigned char)((d >> 8) & 0xff));
                decode ((unsigned char)((d >> 16) & 0xff));
                decode ((unsigned char)((d >> 24) & 0xff));
        }

	return 0;
}
