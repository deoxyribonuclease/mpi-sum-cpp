#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <chrono>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (argc < 2) {
        if (world_rank == 0)
            printf("Usage: mpiexec -n <proc> ./mpi_sum <array_size>\n");
        MPI_Finalize();
        return 0;
    }

    size_t array_size = atoll(argv[2]);
    if (array_size < 1) array_size = 1;

    if (world_rank == 0) {
        printf("Running on %d MPI processes, array size = %zu\n", world_size, array_size);
    }

    std::vector<int> full_array;

    if (world_rank == 0) {
        full_array.resize(array_size);
        for (size_t i = 0; i < array_size; i++)
            full_array[i] = rand() % 100;
    }

    size_t base = array_size / world_size;
    size_t rem  = array_size % world_size;
    size_t local_size = base + (world_rank == world_size - 1 ? rem : 0);

    std::vector<int> local_array(local_size);

    if (world_rank == 0) {
        for (int r = 0; r < world_size; r++) {
            size_t start = r * base;
            size_t end   = (r == world_size - 1) ? array_size : (r + 1) * base;
            size_t size_r = end - start;

            if (r == 0) {
                std::copy(full_array.begin(), full_array.begin() + size_r, local_array.begin());
            } else {
                MPI_Send(full_array.data() + start, size_r, MPI_INT, r, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        MPI_Recv(local_array.data(), local_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    auto node_start_time = std::chrono::high_resolution_clock::now();

    long long local_sum = 0;
    for (size_t i = 0; i < local_size; i++)
        local_sum += local_array[i];

    auto node_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> node_elapsed = node_end_time - node_start_time;

    char hostname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(hostname, &name_len);

    printf("Node %s (rank %d) local sum = %lld, time = %.3f sec\n",
           hostname, world_rank, local_sum, node_elapsed.count());

    // Глобальне сумування
    long long global_sum = 0;
    auto global_start_time = std::chrono::high_resolution_clock::now();
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    auto global_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> global_elapsed = global_end_time - global_start_time;

    if (world_rank == 0) {
        printf("\nFINAL GLOBAL SUM = %lld\n", global_sum);
        printf("Time for global reduce = %.3f sec\n", global_elapsed.count());
    }

    MPI_Finalize();
    return 0;
}
