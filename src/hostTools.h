#ifndef HOST_TOOLS_H
#define HOST_TOOLS_H
#include <dpu.h>
#include <assert.h>
/**
 * @brief This function simply parse arguments from commandline and update variables.
 * @param argc Number of arguments.
 * @param argv The values of arguments.
 * @param nb_dpus The pointer to nb_dpu variable to update.
 * @param nb_tasklets The pointer to nb_tasklets variable to update.
 * @param nb_boots The pointer to nb_boots variable to update.
 * @param nb_blocks_to_mine The pointer to nb_blocks_to_mine to update..
 */

void HOST_TOOLS_parse_args(int argc, char **argv, uint32_t *nb_dpus, uint8_t *nb_tasklets, uint32_t *nb_boots, uint32_t *nb_blocks_to_mine);

/**
 * @brief allocates the number of dpu needed.
 * @param set A pointer to a dpu_set_t structure.
 * @param nb_dpus A pointer to the number of dpus to allocate,
 *                It stores the actual number of dpus allocated.
 *
 */
void HOST_TOOLS_allocate_dpus(struct dpu_set_t *set, uint32_t *nb_dpus);

/**
 * @brief Sends id to each DPU allocated.
 * @param set The set to send ids to .
 */
void HOST_TOOLS_send_id(struct dpu_set_t set);

/**
 * @brief Compile DPU program with the number of tasklets needed.
 * @param nb_tasklets Number of tasklets to compile with
 *
 */
void HOST_TOOLS_compile(uint8_t nb_tasklets);

/**
 * @brief Launches portfolio solvers
*/
void HOST_TOOLS_portfolio_launch(char* filename,struct dpu_set_t set);
void HOST_TOOLS_launch(char* filename, struct dpu_set_t set);
#endif // HOST_TOOLS_H