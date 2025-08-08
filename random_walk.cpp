#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc != 3)
    {
        if (world_rank == 0)
        {
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    domain_size = atoi(argv[1]);
    max_steps = atoi(argv[2]);

    if (world_rank == 0)
    {
        controller_process();
    }
    else
    {
        walker_process();
    }

    MPI_Finalize();
    return 0;
}

void walker_process()
{
    srand(time(NULL) + world_rank);

    int pos = 0;
    int step;
    for (step = 1; step <= max_steps; ++step)
    {
        int move = (rand() % 2 == 0) ? -1 : 1;
        pos += move;

        if (pos < -domain_size || pos > domain_size)
        {
            break;
        }
    }

    std::cout << "Rank " << world_rank << ": Walker finished in " << step << " steps." << std::endl;

    // Send a message (any int) to controller (rank 0) to signal completion
    int msg = step;
    MPI_Send(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void controller_process()
{
    int walkers = world_size - 1;
    int completed_walkers = 0;
    MPI_Status status;

    for (int i = 0; i < walkers; ++i)
    {
        int recv_buf;
        // Receive from any source (any walker that finishes)
        MPI_Recv(&recv_buf, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        ++completed_walkers;
        // Optionally, you can record which rank just finished using status.MPI_SOURCE
    }
    std::cout << "Controller: All " << walkers << " walkers have finished their walks." << std::endl;
}