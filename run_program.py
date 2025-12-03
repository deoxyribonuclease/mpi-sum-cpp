import subprocess
import argparse
import os

def compile_file(file_path):
    if not os.path.exists(file_path):
        print("Error: File not found.")
        return

    subprocess.run(f"multipass transfer {file_path} manager:cloud", shell=True)

    full_file_name = os.path.basename(file_path)
    file_name = os.path.splitext(full_file_name)[0]
    print(f"File {full_file_name} transferred to manager.")

    subprocess.run(
        f"multipass exec manager -- mpicxx -std=c++17 -O2 -o /home/ubuntu/cloud/{file_name} /home/ubuntu/cloud/{full_file_name}",
        shell=True
    )

def run_mpi_program(file_name, num_processes, threads_per_node, array_size):
    if subprocess.run(f"multipass exec manager -- test -f /home/ubuntu/cloud/{file_name}",
                      shell=True).returncode != 0:
        print("Error: Compiled file not found on manager machine.")
        return

    mpi_command = f"mpiexec -hostfile /home/ubuntu/cloud/mpi_hosts"
    if num_processes != -1:
        mpi_command += f" -n {num_processes}"
    
    # рівномірний розподіл задач між вузлами
    mpi_command += " --map-by node"

    mpi_command += f" /home/ubuntu/cloud/{file_name} {threads_per_node} {array_size}"

    print(f"Running MPI program: {mpi_command}")
    subprocess.run(f"multipass exec manager -- {mpi_command}", shell=True)


def main():
    parser = argparse.ArgumentParser(description='Run MPI cluster')
    parser.add_argument('--c', '-c', metavar='path_to_file', type=str,
                        help='Path to the C/C++ file to compile')
    parser.add_argument('--r', '-r', metavar='file_name', type=str,
                        help='Name of the compiled file to run')
    parser.add_argument('--n', '-n', type=int, default=-1,
                        help='Number of MPI processes to run')
    parser.add_argument('--t', '-t', type=int, default=1,
                        help='Number of threads per node')
    parser.add_argument('--a', '-a', type=int, default=1,
                        help='Array size')

    args = parser.parse_args()

    if args.c:
        compile_file(args.c)
        print("File compiled successfully.")
    elif args.r:
        run_mpi_program(args.r, args.n, args.t, args.a)
    else:
        print("Please provide either --c or --r option.")

if __name__ == "__main__":
    main()
