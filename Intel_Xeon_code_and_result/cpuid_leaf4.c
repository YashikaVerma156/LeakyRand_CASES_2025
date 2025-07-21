#include <stdio.h>

int main (void)
{
	unsigned a, b, c, d, x;
	int i=0;

	while (1) {
		asm ("movl $0x4, %%eax\n\t"
		     "movl %4, %%ecx\n\t"
             	     "cpuid"
                     :"=a"(a),"=b"(b),"=c"(c),"=d"(d)
		     :"r"(i));

		x = a & 0x1f;
		if (x == 0) break;
		else if (x == 1) printf("**************Data Cache****************\n");
		else if (x == 2) printf("*******Instruction Cache****************\n");
		else if (x == 3) printf("***********Unified Cache****************\n");

		x = (a >> 5) & 0x7;
		printf("\tLevel: %u\n", x);
		if (((a >> 8) & 0x1) == 1) printf("\tSelf initializing cache level (does not need SW initialization)\n");
		if (((a >> 9) & 0x1) == 1) printf("\tFully associative\n");

		printf("\tCan be shared by up to %u logical cores\n", 1 + ((a >> 14) & 0xfff));
		printf("\tMaximum allowed physical cores in package: %u\n", 1 + ((a >> 26) & 0x3f));
		printf("\tBlock size: %u\n", 1 + (b & 0xfff));
		printf("\tBlocks per physical frame (Partitions): %u\n", 1 + ((b >> 12) & 0x3ff));
		printf("\tWays: %u\n", 1 + ((b >> 22) & 0x3ff));
		printf("\tSets: %u\n", 1 + c);
		printf("\tCapacity: %u KB\n", ((1 + (b & 0xfff))*(1 + ((b >> 12) & 0x3ff))*(1 + ((b >> 22) & 0x3ff))*(1 + c))/1024);

		if ((d & 0x1) == 0)printf("\tWBINVD/INVD\n");
		else printf("\tNo WBINVD/INVD\n");
		if (((d >> 1) & 0x1) == 1) printf("\tInclusive\n");
		else printf("\tNon-inclusive\n");
		if (((d >> 2) & 0x1) == 1) printf("\tComplex hash indexing\n");
                else printf("\tSimple hash indexing\n");

		i++;
		printf("\n");
	}

	return 0;
}
