# Parallel Sum Cluster

## Setup Cluster

```bash
python.exe setup_cluster.py 4 --cpu 2 --ram 2048
```

* 4 вузли, 2 CPU, 2048 MB RAM

## Compile MPI Programs

```bash
python run_program.py --c mpi_part_sum.cpp
python run_program.py --c mpi_wave_sum.cpp
```

## Run MPI Programs

```bash
python run_program.py --r mpi_part_sum --n 8 --a 200000000
python run_program.py --r mpi_wave_sum --n 8 --a 200000000
```

* `--n` — число процесів MPI
* `--a` — розмір масиву


