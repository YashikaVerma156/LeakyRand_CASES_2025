#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#define MAX_REP 100
#define NUM_SIZES 10

#define max(a,b) ((a) > (b) ? (a) : (b))
#define usage_exit(exe, s) do{ \
                         printf("Error: %s\n", (s)); \
                         printf("Usage: %s [raw data file path] [occupancy target] [static optimal cycles]\n", (exe)); \
                         exit(-1); \
                      }while(0);
struct entries{
      int size;
      long rem_budget;
      long inc_ptime;
      long inc_atime;
      long occ;
}; 

typedef struct{
                int count;
                struct entries e[MAX_REP];
}OptPath;

typedef struct{
                 int t_rep;
                 long atime[MAX_REP];
                 long ptime;
                 long l_ptime;
                 long occupancy[MAX_REP];  
                 
}Step;

Step steps[NUM_SIZES];

void print_solution(OptPath *p)
{
  printf("Solution: Remaining budget %ld\n", p->e[p->count - 1].rem_budget);
  for(int i=0; i < p->count; ++i){
     printf("size %d rem_budget %ld inc_ptime %ld inc_atime %ld occ %ld\n", 
            p->e[i].size, p->e[i].rem_budget, p->e[i].inc_ptime, p->e[i].inc_atime, p->e[i].occ);
  }
  return;
}
int read_csv(FILE *fpt, long target)
{
   char buf[1024];
   fgets(buf, 1024, fpt);  // Skip the comment line
   while(fgets(buf, 1024, fpt)){
       char *str = strtok(buf, ","); //arr_size multiple
       if(!str)
            goto err_ret;
       float arr_size = atof(str) + 0.01; 
       arr_size = (arr_size - 1) * 10;
       int s_offset = (int)arr_size;
       if(s_offset < 0 || s_offset > 9)
            goto err_ret;
       Step *s = &steps[s_offset];
       str = strtok(NULL, ",");  //random seed
       str = strtok(NULL, ",");  //num_iter
       
       if(!str || atoi(str) != s->t_rep + 1)
            goto err_ret;
       str = strtok(NULL, ",");  //llc occupancy
       if(!str)
            goto err_ret;
       s->occupancy[s->t_rep] = atol(str); 
       str = strtok(NULL, ",");  //total cycles
       str = strtok(NULL, ",");  //llc filling cycles
       str = strtok(NULL, ",");  //thrashing (access cycles)
       if(!str)
            goto err_ret;
       s->atime[s->t_rep] = atol(str); 
       str = strtok(NULL, ",");  //probe cycles
       if(!str)
            goto err_ret;
       s->l_ptime = atol(str);
       if(!s->ptime && s->occupancy[s->t_rep] >= target)
              s->ptime = s->l_ptime; 
       str = strtok(NULL, ",");  //probe cycles
       if(str)
            goto err_ret;  //Must be NULL
       //printf("%d %ld %ld %ld\n", s->t_rep, s->atime[s->t_rep], s->ptime, s->occupancy[s->t_rep]);
       s->t_rep++;
   }
    //Check if for every size, we reach the target occupancy
    for(int i=0; i<NUM_SIZES; ++i){
       Step *s = &steps[i];
       if(!s->ptime){
             assert(s->occupancy[s->t_rep - 1] < target);  
             printf("Warning: size: %d can not fill probe time for the target occupancy, using last probe time\n", i);
             s->ptime = s->l_ptime;
       }
    }
   return 0;

err_ret:
       return -1;
}

long next_occupancy(long c_oc, Step *s, long *cycles)
{
   long prev = 0, incr = 0, p_cycles = 0;
   int ctr;
   for(ctr=0; ctr<s->t_rep; ++ctr){
         if(s->occupancy[ctr] > c_oc)
              break;
   }
   if(ctr){
       prev = s->occupancy[ctr-1];
       p_cycles = s->atime[ctr-1];
   }
   incr = s->occupancy[ctr] - c_oc;
   double D = ((double)(s->occupancy[ctr] - prev - incr)) / ((double)(s->occupancy[ctr] - prev));
   
   incr += D * ((double) (s->occupancy[ctr+1] - s->occupancy[ctr]));
   p_cycles = (1.0 - D) * ((double)(s->atime[ctr] - p_cycles));
   p_cycles += D * ((double)(s->atime[ctr+1] - s->atime[ctr]));
   *cycles = p_cycles;

   // assert(incr >= 0);  //some data points show decreasing pattern
   // assert(p_cycles >= 0);
   
   return incr;
}

int search_feasible(long c_oc, long t_oc, int max_size, long budget, OptPath path)
{
    int i = (max_size == -1) ? 0 : max_size;
    for(; i<NUM_SIZES; ++i){
        long a_cycles = 0, a_oc = 0;
        long lbudget = budget;
        path.e[path.count].size = i;
        if(max_size == -1){
             lbudget -= steps[i].ptime;
             if(lbudget <= 0)
                 return 0; 
        }else if(max_size < i){
            assert(i >= 1);
            lbudget += steps[max_size].ptime;
            lbudget -= steps[i].ptime;
            if(lbudget <= 0)
                 return 0;
        }
        path.e[path.count].inc_ptime = budget - lbudget;
        a_oc = next_occupancy(c_oc, &steps[i], &a_cycles);
        lbudget -= a_cycles;
        if(lbudget < 0)
              return 0;

        path.e[path.count].rem_budget = lbudget;
        path.e[path.count].inc_atime = a_cycles;
        path.e[path.count].occ = c_oc + a_oc;
        path.count++;

        if(c_oc + a_oc >= t_oc){
             print_solution(&path);
        }else{
             search_feasible(c_oc + a_oc, t_oc, max(i, max_size), lbudget, path);
        }
        path.count--;
    }

    return 0;
}

int main(int argc, char **argv)
{
  FILE *fpt;

  if(argc != 4)
          usage_exit(argv[0], "Invalid number of arguments");
    
  fpt = fopen(argv[1], "r");
  
  if(!fpt)
          usage_exit(argv[0], "Raw data file path not found");
   long t_oc = atol(argv[2]);
   long budget = atol(argv[3]);

   if(read_csv(fpt, t_oc) < 0)
          usage_exit(argv[0], "Raw data file format is wrong");
   
   OptPath path;
   memset(&path, 0, sizeof(OptPath));
   
   search_feasible(0, t_oc, -1, budget, path);
   return 0;
}
