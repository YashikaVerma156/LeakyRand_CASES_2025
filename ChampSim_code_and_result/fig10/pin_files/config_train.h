#define code() { asm volatile("mov (%1),%0;"\
                             :"=r"(j)\
                             :"a"(ptr)\
                             :"memory"); \
                ptr = ptr-8;\
                }
#define Extra0 //nothing
#define Extra1 code()
#define Extra2 Extra1 Extra1
#define Extra3 Extra2 Extra1
#define Extra4 Extra2 Extra2
#define Extra5 Extra4 Extra1
#define Extra6 Extra3 Extra3
#define Extra7 Extra4 Extra3
#define Extra8 Extra4 Extra4
#define Extra9 Extra5 Extra4
#define Extra10 Extra5 Extra5
#define Extra11 Extra6 Extra5
#define Extra12 Extra6 Extra6
#define Extra13 Extra12 Extra1
#define Extra14 Extra13 Extra1
#define Extra15 Extra14 Extra1
#define Extra16 Extra15 Extra1
#define Extra17 Extra16 Extra1
#define Extra18 Extra2 Extra16
#define Extra19 Extra3 Extra16
#define Extra20 Extra4 Extra16
#define Extra21 Extra5 Extra16
#define Extra22 Extra6 Extra16
#define Extra23 Extra7 Extra16
#define Extra24 Extra8 Extra16
#define Extra25 Extra9 Extra16
#define Extra26 Extra10 Extra16
#define Extra27 Extra11 Extra16
#define Extra28 Extra12 Extra16
#define Extra29 Extra13 Extra16
#define Extra30 Extra14 Extra16
#define Extra31 Extra15 Extra16
#define Extra32 Extra16 Extra16
#define Extra33 Extra32 Extra1
#define Extra34 Extra32 Extra2
#define Extra35 Extra3 Extra32
#define Extra36 Extra33 Extra3
#define Extra37 Extra4 Extra33
#define Extra38 Extra4 Extra34
#define Extra39 Extra35 Extra4
#define Extra40 Extra35 Extra5
#define Extra41 Extra36 Extra5
#define Extra42 Extra36 Extra6
#define Extra43 Extra42 Extra1
#define Extra44 Extra43 Extra1
#define Extra45 Extra44 Extra1
#define Extra46 Extra45 Extra1
#define Extra47 Extra46 Extra1
#define Extra48 Extra32 Extra16
#define Extra49 Extra33 Extra16
#define Extra50 Extra30 Extra20
#define Extra51 Extra30 Extra21
#define Extra52 Extra30 Extra22
#define Extra53 Extra30 Extra23
#define Extra54 Extra30 Extra24
#define Extra55 Extra30 Extra25
#define Extra56 Extra30 Extra26
#define Extra57 Extra30 Extra27
#define Extra58 Extra30 Extra28
#define Extra59 Extra30 Extra29
#define Extra60 Extra30 Extra30
#define Extra61 Extra30 Extra31
#define Extra62 Extra32 Extra30
#define Extra63 Extra32 Extra31
#define Extra64 Extra32 Extra32
#define Extra65 Extra64 Extra1
#define Extra66 Extra64 Extra2
#define Extra67 Extra64 Extra3
#define Extra68 Extra64 Extra4
#define Extra69 Extra64 Extra5
#define Extra70 Extra64 Extra6
#define Extra71 Extra64 Extra7
#define Extra72 Extra64 Extra8
#define Extra73 Extra64 Extra9
#define Extra74 Extra64 Extra10
#define Extra75 Extra64 Extra11
#define Extra76 Extra64 Extra12
#define Extra77 Extra64 Extra13
#define Extra78 Extra64 Extra14
#define Extra79 Extra64 Extra15
#define Extra80 Extra64 Extra16
#define Extra81 Extra64 Extra17
#define Extra82 Extra64 Extra18
#define Extra83 Extra64 Extra19
#define Extra84 Extra64 Extra20
#define Extra85 Extra64 Extra21
#define Extra86 Extra64 Extra22
#define Extra87 Extra64 Extra23
#define Extra88 Extra64 Extra24
#define Extra89 Extra64 Extra25
#define Extra90 Extra64 Extra26
#define Extra91 Extra64 Extra27
#define Extra92 Extra64 Extra28
#define Extra93 Extra64 Extra29
#define Extra94 Extra64 Extra30
#define Extra95 Extra64 Extra31
#define Extra96 Extra32 Extra64
#define Extra97 Extra33 Extra64
#define Extra98 Extra34 Extra64
#define Extra99 Extra35 Extra64
#define Extra100 Extra36 Extra64
#define Extra101 Extra37 Extra64
#define Extra102 Extra38 Extra64
#define Extra103 Extra39 Extra64
#define Extra104 Extra40 Extra64
#define Extra105 Extra41 Extra64
#define Extra106 Extra42 Extra64
#define Extra107 Extra43 Extra64
#define Extra108 Extra44 Extra64
#define Extra109 Extra45 Extra64
#define Extra110 Extra46 Extra64
#define Extra111 Extra47 Extra64
#define Extra112 Extra48 Extra64
#define Extra113 Extra49 Extra64
#define Extra114 Extra50 Extra64
#define Extra115 Extra51 Extra64
#define Extra116 Extra50 Extra66
#define Extra117 Extra51 Extra66
#define Extra118 Extra52 Extra66
#define Extra119 Extra53 Extra66
#define Extra120 Extra54 Extra66
#define Extra121 Extra55 Extra66
#define Extra122 Extra61 Extra61
#define Extra123 Extra62 Extra61
#define Extra124 Extra62 Extra62
#define Extra125 Extra63 Extra62
#define Extra126 Extra63 Extra63
#define Extra127 Extra64 Extra63
#define Extra128 Extra64 Extra64


#define Base_string1 Extra1
#define Base_string2 Extra2
#define Base_string4 Extra4
#define Base_string8 Extra4 Extra4
#define Base_string16 Extra8 Extra8
#define Base_string32 Extra16 Extra16
#define Base_string64 Extra32 Extra32
#define Base_string128 Extra64 Extra64

#define NUM_ACCESS_FOR_BIT1 1
#define NUM_BITS 0  // 200 30 //140 //30
#define STR_NUM 0
#define DO_ERR_CORR_AT 0
#define DO_OCCUPANCY_CHECK_AT 0
#define ERR_CORR_ITERATIONS 0
#define MAX_HOLES 0
#define TRIGGER_HOLES_AT 0
#define TRIGGER_EXTRA_ACCESS_AT 0

//Variables corresponding to IEEE SEED implementation.
#define ARR_SIZE_SENDER 0
#define REPETETION_CODING 0
#define DIFFERENTIAL_SIGNALLING 0
#define NUM_BITS_SEED NUM_BITS*REPETETION_CODING*DIFFERENTIAL_SIGNALLING

unsigned int strlength(char * str)
{
    int i=0;
    while(str[i] != '\0')
        i++;
      
     return i;
}

 long power(int num, int pow)
{
    long int ret = num;
    if(pow == 0)
        return 1;
    for(int i=1; i<pow; i++ )
        ret *= num;

    return ret;
}

void tostring(char str[], long int num)
{
    int i, rem, len = 0, n;

    n = num; 
    if(num == 0)
    {
        len++;
    }
    else 
    {
        while (n != 0)
        {
            len++;
            n /= 10;
        } 
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

int toint(char str[])
{
    int len = strlength(str);
    int i, num = 0;

    for (i = 0; i < len; i++)
    {
        num = num + ((str[len - (i + 1)] - '0') * power(10, i));
    }

   return num;
}

void print(char * hello)
{
    int ret;
    asm volatile  // write to standard output device.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(1), "S"(hello), "d"(strlength(hello))
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );
}   

int open_II(char * str)
{
    int ret1, ret;
    asm volatile  // open file descriptor.
    (
        "syscall"
        : "=a" (ret1)
//        : "0"(2), "D"("array_start_address.txt"), "S"(0102), "d"(00600)
        : "0"(2), "D"(str), "S"(0102), "d"(00600)
        : "rcx", "r11", "memory"  //the kernel dereferences pointer args
    );
    return ret1;
}

int open()
{
    int ret1, ret;
    asm volatile  // open file descriptor.
    (
        "syscall"
        : "=a" (ret1)
        : "0"(2), "D"("test2.txt"), "S"(0102), "d"(00600)
        : "rcx", "r11", "memory"  //the kernel dereferences pointer args
    );
    return ret1;
}

int write_II(unsigned long int ptr, int fd)
{
    int ret;
    char hello[40]; 
    tostring(hello, ptr);

    asm volatile  // write to standard output device.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(1), "S"(hello), "d"(strlength(hello))
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );

    asm volatile  // write to standard output device.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(1), "S"(" "), "d"(1)
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );

    asm volatile // write to file.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(fd), "S"(hello), "d"(strlength(hello))
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );

    asm volatile // write to file.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(fd), "S"(" "), "d"(1)
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );


    return 0;
}

int close(int fd)
{
    int ret;
    asm volatile  // close file.
    (
        "syscall"
        : "=a" (ret) // returns 0 on success
        : "0"(3), "D"(fd)
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );

    return ret;
}

int open_and_write(unsigned long int ptr)
{
    int ret1, ret;
    char hello[40];
    tostring(hello, ptr);
    asm volatile  // open file descriptor.
    (
        "syscall"
        : "=a" (ret1)
        : "0"(2), "D"("test1.txt"), "S"(0102), "d"(00600)
        : "rcx", "r11", "memory"  //the kernel dereferences pointer args
    );
   
   /*asm volatile  // TODO print newline character to standard output device.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(1), "S"('\n'), "d"(1)
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    ); */

    asm volatile  // write to standard output device.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(1), "S"(hello), "d"(strlength(hello))
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );

    asm volatile // write to file.
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(ret1), "S"(hello), "d"(strlength(hello))
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );
    
    asm volatile  // close file.
    (
        "syscall"
        : "=a" (ret) // returns 0 on success
        : "0"(3), "D"(ret1)
        : "rcx", "r11", "memory" //the kernel dereferences pointer args
    );
  
    return 0;
}

unsigned int getrandom()
{ 
    unsigned int rand; 
    int ret;
    void *pointer = &rand;
    asm volatile(  //getrandom syscall
               "syscall"
               : "=a" (ret)// no return value
               : "0"(318), "D"(pointer), "S"(sizeof(int)), "d"(1) // GRND_NONBLOCK == 1, GRND_RANDOM == 2.
               : "rcx", "r11", "memory" //the kernel dereferences pointer args
      );
    if(ret != sizeof(int))
    {   char hello[] = "random number not generated";
        asm volatile  // write to standard output device.
        (      "syscall"
               : "=a" (ret)
               : "0"(1), "D"(1), "S"(hello), "d"(strlength(hello))
               : "rcx", "r11", "memory" //the kernel dereferences pointer args
        );
        asm volatile(  //exit(1)
               "syscall"
               : "=a" (ret)// no return value
               : "0"(60), "D"(1)
               : "rcx", "r11", "memory" //the kernel dereferences pointer args
        ); 
    }
   //char hello[2];
   // tostring(hello, rand%2);   
   // print(hello); 
    return rand;
}

void read_message( char * msg_str)
{
    int ret, ret1;
    char hello[513], ch;
    asm volatile  // open file descriptor.
    (
        "syscall"
        : "=a" (ret1)
        : "0"(2), "D"("benchmark_train.txt"), "S"(0102), "d"(00600)
        : "rcx", "r11", "memory"  //the kernel dereferences pointer args
    );

    for (int i = 0; i < STR_NUM; i++)
    {
        asm volatile // Read from file.
        (
            "syscall"
            : "=a" (ret)
            : "0"(0), "D"(ret1), "S"(hello), "d"(NUM_BITS)
            : "rcx", "r11", "memory" //the kernel dereferences pointer args
        );
        asm volatile // Read from file.
        (
            "syscall"
            : "=a" (ret)
            : "0"(0), "D"(ret1), "S"(&ch), "d"(1)
            : "rcx", "r11", "memory" //the kernel dereferences pointer args
        );
    }
    int i =0;
    for(i=0; i < NUM_BITS; i++)
    {
        *(msg_str+i) = hello[i];
    }
    *(msg_str+i) = '\0';
}
