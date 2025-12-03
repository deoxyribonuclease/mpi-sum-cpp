#include <mpi.h>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char hostname[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(hostname, &name_len);

    size_t ARRAY_SIZE = (argc > 2) ? atoll(argv[2]) : 20; 
    if (ARRAY_SIZE < 1) ARRAY_SIZE = 1;

    std::vector<long long> array;
    if (rank == 0) {
        array.resize(ARRAY_SIZE);
        for (size_t i = 0; i < ARRAY_SIZE; i++)
            array[i] = rand() % 100 + 1;
        // printf("Initial array:\n");
        // for (size_t i = 0; i < ARRAY_SIZE; i++)
        //     printf("%lld ", array[i]);
        // printf("\n\n");
    } else {
        array.resize(ARRAY_SIZE);
    }


    double start_time = MPI_Wtime();
    size_t len = ARRAY_SIZE;
    int wave = 0;

    while (len > 1) {
        wave++;
        size_t pairs = len / 2;

        MPI_Bcast(array.data(), len, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

        size_t chunk = (pairs + size - 1) / size; 
        size_t start_idx = rank * chunk;
        size_t end_idx = std::min(start_idx + chunk, pairs);

        std::vector<long long> local_res(pairs, 0);
        for (size_t i = start_idx; i < end_idx; i++)
            local_res[i] = array[i] + array[len - 1 - i];

        // if (end_idx > start_idx) {
        //     printf("Wave %d, Node %s (rank %d) computes pairs:", wave, hostname, rank);
        //     for (size_t i = start_idx; i < end_idx; i++)
        //         printf(" (%zu,%zu)", i, len - 1 - i);
        //     printf("\n");
        // }

        std::vector<long long> root_res(pairs);
        MPI_Reduce(local_res.data(), root_res.data(), pairs, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            for (size_t i = 0; i < pairs; i++)
                array[i] = root_res[i];
            if (len % 2 == 1)
                array[pairs] = array[len - 1];
            len = pairs + (len % 2);
        }

        MPI_Bcast(&len, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
    }

    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("FINAL GLOBAL SUM = %lld\n", array[0]);
        printf("Total time: %.6f seconds\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
