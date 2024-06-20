#include "hostTools.h"
#include "microsat_host.h"
#include "log.h"
#include <time.h>
#include "utils.h"

void HOST_TOOLS_allocate_dpus(struct dpu_set_t *set, uint32_t *nb_dpus)
{
  DPU_ASSERT(dpu_alloc(*nb_dpus, NULL, set));
  DPU_ASSERT(dpu_get_nr_dpus(*set, nb_dpus));
  printf("ALLOCATED : %u DPUs \n", *nb_dpus);
}

void HOST_TOOLS_send_id(struct dpu_set_t set)
{
  struct dpu_set_t dpu;
  uint32_t id = 0;
  DPU_FOREACH(set, dpu, id)
  {
    DPU_ASSERT(dpu_copy_to(dpu, "dpu_id", 0, &id, sizeof(uint32_t)));
  }
}

void HOST_TOOLS_compile(uint8_t nb_tasklets)
{
  char command[100];
  sprintf(command, "make dpu NB_TASKLETS=%d", nb_tasklets);
  system(command);
}
static void populate_offsets(int offsets[13], struct solver S)
{
  //log_message(LOG_LEVEL_INFO,"populating offsets");
  offsets[0] = (int) (S.model - S.DB);
  offsets[1] = (int) (S.next - S.DB);
  offsets[2] = (int) (S.prev - S.DB);
  offsets[3] = (int) (S.buffer - S.DB);
  offsets[4] = (int) (S.reason - S.DB);
  offsets[5] = (int) (S.falseStack - S.DB);
  offsets[6] = (int) (S.forced - S.DB);
  offsets[7] = (int) (S.processed - S.DB);
  offsets[8] = (int) (S.assigned - S.DB);
  offsets[9] = (int) (S.falses - S.DB);
  offsets[10]= (int) (S.first - S.DB);
  offsets[11] = (int)((int*)(S.scores) - (S.DB));
  offsets[12]= (int) (S.decision_level - S.DB);
}
static void populate_vars(int vars[12], struct solver S)
{
    log_message(LOG_LEVEL_INFO, "Populating vars");
    vars[0] = S.nVars;
    vars[1] = S.nClauses;
    vars[2] = S.mem_used;
    vars[3] = S.mem_fixed;
    vars[4] = S.maxLemmas;
    vars[5] = S.nLemmas;
    vars[6] = S.nConflicts;
    vars[7] = S.fast;
    vars[8] = S.slow;
    vars[9] = S.head;
    vars[10] = S.res;
    vars[11] = S.decision_counter;
}

int geometric(int base, int factor, int step)
{
    return base * pow(factor, step - 1);
}
void HOST_TOOLS_pure_portfolio(char* filename, struct dpu_set_t set)
{
    struct solver dpu_solver;
    int ret = parse(&dpu_solver, filename);
    if (ret == UNSAT)
    {
        log_message(LOG_LEVEL_INFO, "parsing UNSAT");
        exit(0);
    }
    log_message(LOG_LEVEL_INFO, "parsing finished");

    int finish = 0;
    int offsets[13];
    int vars[12];

    populate_offsets(offsets, dpu_solver);
    populate_vars(vars, dpu_solver);

    log_message(LOG_LEVEL_INFO, "Broadcasting");
    initialize_dpu_solver(set, &dpu_solver, vars, offsets);
    configure_dpu(set, &dpu_solver);

    clock_t start, end;
    double duration;
    start = clock();

    int mode = 1; // 0 for Luby sequence, 1 for geometric sequence
    int luby_base = 2;
    int geo_base = 10; // Example base for geometric sequence
    int geo_factor = 2; // Example factor for geometric sequence

    while (!finish)
    {
        broadcast_iterations_and_launch(set, &finish, mode, luby_base, geo_base, geo_factor);
        check_dpu_results(set, &dpu_solver, &finish);
    }

    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    log_message(LOG_LEVEL_INFO, "DPU %lf ms", duration);
}


void initialize_dpu_solver(struct dpu_set_t set, struct solver *dpu_solver, int *vars, int *offsets)
{
    DPU_ASSERT(dpu_broadcast_to(set, "dpu_vars", 0, vars, 12 * sizeof(int), DPU_XFER_DEFAULT));
    DPU_ASSERT(dpu_broadcast_to(set, "dpu_DB_offsets", 0, offsets, 13 * sizeof(int), DPU_XFER_DEFAULT));
    DPU_ASSERT(dpu_broadcast_to(set, DPU_MRAM_HEAP_POINTER_NAME, 0, dpu_solver->DB, roundup((dpu_solver->mem_used * sizeof(int)),8), DPU_XFER_DEFAULT));
    DPU_ASSERT(dpu_broadcast_to(set, "config", 0, &dpu_solver->config, sizeof(config_t) - sizeof(float*) - 2 * sizeof(int*), DPU_XFER_DEFAULT));
}

void configure_dpu(struct dpu_set_t set, struct solver *dpu_solver)
{
    struct dpu_set_t dpu;
    HOST_TOOLS_send_id(set);
    DPU_FOREACH(set, dpu)
    {
        dpu_solver->config.br_p = BR_VMTF;//rand() % 3 ;
        dpu_solver->config.rest_p = REST_DEFAULT;//rand() % 4;
        dpu_solver->config.reduce_p = RED_DEFAULT;//rand() % 3;
        DPU_ASSERT(dpu_copy_to(dpu, "config", 0, &dpu_solver->config, sizeof(config_t) - sizeof(float*) - 2 * sizeof(int*)));
    }
}

void broadcast_iterations_and_launch(struct dpu_set_t set, int *finish, int mode, int luby_base, int geo_base, int geo_factor)
{
    int iterations;
    static int luby_index = 1;
    static int geo_step = 1;

    if (mode == 0) 
    {
        iterations = luby(luby_base, luby_index);
        luby_index++;
    }
    else if (mode == 1)
    {
        iterations = geometric(geo_base, geo_factor, geo_step);
        geo_step++;
    }

    DPU_ASSERT(dpu_broadcast_to(set, "dpu_iterations", 0, &iterations, sizeof(int), DPU_XFER_DEFAULT));
    log_message(LOG_LEVEL_INFO, "Launching with %d iterations", iterations);
    DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
}

void check_dpu_results(struct dpu_set_t set, struct solver *dpu_solver, int *finish)
{
    struct dpu_set_t dpu;
    int dpu_ret;
    DPU_FOREACH(set, dpu)
    {
        DPU_ASSERT(dpu_copy_from(dpu, "dpu_ret", 0, &dpu_ret, sizeof(int)));
        if (dpu_ret == SAT)
        {
            DPU_ASSERT(dpu_copy_from(dpu, "config", 0, &dpu_solver->config, sizeof(int)));
            log_message(LOG_LEVEL_INFO, "DPU SAT");
            dpu_log_read(dpu, stdout);
            *finish = 1;
            break;
        }
        else if (dpu_ret == UNSAT)
        {
            log_message(LOG_LEVEL_INFO, "DPU UNSAT");
            *finish = 1;
            dpu_log_read(dpu, stdout);
            break;
        }
    }
}