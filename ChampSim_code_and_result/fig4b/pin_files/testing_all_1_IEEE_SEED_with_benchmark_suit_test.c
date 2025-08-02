#include "config_test.h"
void waiting_and_no_op(){} /* Don't add any instruction into ROB. */
void fence(){} /* don't add next instruction to ROB until previous all instructions has retired. */

struct all_stack_variables
{
    long int arr[ARR_SIZE_SENDER*8], // Memory access that transfers bit 1.
             j,       // Variable to move value of memory location when bit is 1.
             i;       // Loop variable.
    char data[NUM_BITS+1];      //Message to be transferred.
    char data_SEED[NUM_BITS*DIFFERENTIAL_SIGNALLING*REPETETION_CODING]; //Message to be transferred as per SEED idea.
    int str_num;
}__attribute__((aligned (64))); 

static struct all_stack_variables strct_var;

#define STACK_SIZE (1 << 25)
static char buf[STACK_SIZE]  __attribute__((aligned(64)));
           
int main()
{ 
  strct_var.i = 0, strct_var.j = 4, strct_var.str_num = STR_NUM;  
  register int num_bits = 0, code = 0;
  register long int *ptr = strct_var.arr, *ptr1 = &strct_var.i, *ptr2 = &strct_var.arr[(ARR_SIZE_SENDER*8) - 1];

  // Read message to be transferred to receiver.
  read_message(strct_var.data);

  for(int num_bits = 0; num_bits < NUM_BITS; num_bits++)
  {
      for(int code = 0; code < REPETETION_CODING; code++)
      { 
          strct_var.data_SEED[(2*num_bits*REPETETION_CODING) + (code * DIFFERENTIAL_SIGNALLING) ] = '0';
          strct_var.data_SEED[(num_bits*2*REPETETION_CODING) + 1 + (code * DIFFERENTIAL_SIGNALLING) ] = strct_var.data[num_bits];
      }
  }

  repeat: 
  asm volatile(
               "call fence;"  // call fence
               "call waiting_and_no_op;"  // call waiting_and_no_op
               "nop;"
               :
               :
               :"memory");

  if(strct_var.data_SEED[strct_var.i] - 48)
  {  
          ptr2 = &strct_var.arr[(ARR_SIZE_SENDER*8) - 1];
          while(ptr < ptr2)
          {
							asm volatile("mov (%1), %0;"
													:"=r"(strct_var.j)
													:"a"(ptr2)
													: "memory");
              ptr2 = ptr2 - 8; 
          }

			asm volatile("call fence" // call fence
									 :
									 :
									 :"memory");
  }
  asm goto volatile (
                     "add $1, (%0);"
                     "cmpl %1, (%0);"
                     "jl %l[repeat]"
                     :/* No outputs. */
                     :"r"(ptr1), "ir"(NUM_BITS_SEED) // Register and immediate operand
                     :"memory" /* Clobbers */
                     : repeat
                    );
  asm volatile(
               "call fence;"
               "call waiting_and_no_op;"  // call waiting_and_no_op
               "nop;"
               :
               :
               :"memory"
             );
	
  int ret;
  asm volatile(  //exit(0)
               "syscall"
               : "=a" (ret)// return value
               : "0"(60), "D"(0)
               : "rcx", "r11", "memory" //the kernel dereferences pointer args
      );
}


void _sender_entry(void)
{       
        asm volatile( "mov %0, %%rsp;"
                      "jmp *%1;"
                 : 
                 :"r"(buf+STACK_SIZE), "r"(&main)
                 : "memory"
                 );
}
