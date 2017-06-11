#include "ooo_cpu.h"

#define GLOBAL_HISTORY_LENGTH 14
#define GLOBAL_HISTORY_MASK (1 << GLOBAL_HISTORY_LENGTH) - 1
int branch_history_vector[NUM_CPUS];

#define GS_HISTORY_TABLE_SIZE 16384
int gs_history_table[NUM_CPUS][GS_HISTORY_TABLE_SIZE];

uint8_t my_last_prediction[NUM_CPUS];

unsigned int gs_table_hash(uint64_t ip, int bh_vector)
{
    unsigned int hash = ip^(ip>>GLOBAL_HISTORY_LENGTH)^(ip>>(GLOBAL_HISTORY_LENGTH*2))^bh_vector;
    hash = hash%GS_HISTORY_TABLE_SIZE;

    //printf("%d\n", hash);

    return hash;
}

void O3_CPU::initialize_branch_predictor()
{
    for(int j=0; j<NUM_CPUS; j++)
    {
        branch_history_vector[j] = 0;

        for(int i=0; i<GS_HISTORY_TABLE_SIZE; i++)
        {
            gs_history_table[j][i] = 2; // 2 is slightly taken
        }

        my_last_prediction[j] = 0;
    }
}

uint8_t O3_CPU::predict_branch(uint64_t ip)
{
    uint8_t prediction = 1;

    int gs_hash = gs_table_hash(ip, branch_history_vector[cpu]);

    //printf("%d ", gs_hash);

    if(gs_history_table[cpu][gs_hash] >= 2)
        prediction = 1;
    else
        prediction = 0;

    my_last_prediction[cpu] = prediction;

    return prediction;
}

void O3_CPU::last_branch_result(uint64_t ip, uint8_t taken)
{
    /*
       static int total_branches = 0;
       static int correct_branches = 0;

       total_branches++;
       if(taken == my_last_prediction[cpu])
       {
       correct_branches++;
       }
       if(total_branches%1000 == 0)
       {
       printf("%0.1f%% ", (100.0*correct_branches)/total_branches);
       }
       */

    //printf("%d ", branch_history_vector);

    int gs_hash = gs_table_hash(ip, branch_history_vector[cpu]);

    if(taken == 1)
    {
        if(gs_history_table[cpu][gs_hash] < 3)
        {
            gs_history_table[cpu][gs_hash]++;
        }
    }
    else
    {
        if(gs_history_table[cpu][gs_hash] > 0)
        {
            gs_history_table[cpu][gs_hash]--;
        }
    }

    // update branch history vector
    branch_history_vector[cpu] <<= 1;
    branch_history_vector[cpu] &= GLOBAL_HISTORY_MASK;
    branch_history_vector[cpu] |= taken;
}
