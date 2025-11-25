#include "ibex.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace ibex;

static const int MAX_VARS = 6;

struct SharedData {
    int    command;                 
    double rel_eps_f;
    double loup;
    double uplo;
    double loup_point[MAX_VARS];
    double exec_time_ms;            
};

int main() {
    const char* shm_name = "/ibex_shm";

    
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        return 1;
    }

    void* addr = mmap(nullptr, sizeof(SharedData),
                      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    SharedData* data = static_cast<SharedData*>(addr);

    cout << "[C++] Ibex worker iniciado" << endl;

    
    System sys("/home/labeit/Ibex/ibex-lib/benchs/optim/easy/ex3_1_3.bch");

    bool running = true;
    while (running) {

        if (data->command == 1) {
            cout << "[C++] Recibido comando RUN" << endl;

            double rel_eps = (data->rel_eps_f > 0) ? data->rel_eps_f : 1e-7;

            
            using namespace std::chrono;
            auto start = high_resolution_clock::now();

            DefaultOptimizer opt(sys, rel_eps);
            opt.optimize(sys.box);

            auto end = high_resolution_clock::now();
            double exec_ms =
                duration_cast<microseconds>(end - start).count() / 1000.0;
            

            data->loup = opt.get_loup();
            data->uplo = opt.get_uplo();

            
            const IntervalVector& iv = opt.get_loup_point();
            int n = std::min((int)iv.size(), MAX_VARS);
            for (int i = 0; i < n; ++i)
                data->loup_point[i] = iv[i].mid();

            data->exec_time_ms = exec_ms;

            data->command = 2;  
        }
        else if (data->command == 3) {
            cout << "[C++] Exit recibido" << endl;
            running = false;
        }
        else {
            usleep(1000); 
        }
    }

    munmap(addr, sizeof(SharedData));
    close(fd);

    return 0;
}
