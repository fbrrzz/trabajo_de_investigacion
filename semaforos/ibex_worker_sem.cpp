#include "ibex.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <semaphore.h>

using namespace std;
using namespace ibex;

static const int MAX_VARS = 6;

struct SharedData {
    int command;                
    double rel_eps_f;
    double loup;
    double uplo;
    double loup_point[MAX_VARS];
    double exec_time_ms;
};

int main() {
    const char* shm_name   = "/ibex_shm2";
    const char* sem_req_nm = "/ibex_sem_req";
    const char* sem_res_nm = "/ibex_sem_res";

    cout << "[C++] Iniciando worker con semáforos" << endl;

    
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        return 1;
    }

    void* addr = mmap(nullptr, sizeof(SharedData),
                      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (addr == MAP_FAILED) { perror("mmap"); return 1; }

    SharedData* data = (SharedData*)addr;

   
    sem_t* sem_req = sem_open(sem_req_nm, O_CREAT, 0666, 0);
    sem_t* sem_res = sem_open(sem_res_nm, O_CREAT, 0666, 0);

    if (!sem_req || !sem_res) {
        perror("sem_open");
        return 1;
    }

    
    System sys("/home/labeit/Ibex/ibex-lib/benchs/optim/easy/ex3_1_3.bch");

    cout << "[C++] Worker listo. Esperando señales..." << endl;

    bool running = true;

    while (running) {

        
        sem_wait(sem_req);

        if (data->command == 3) {
            cout << "[C++] Recibido EXIT" << endl;
            running = false;
            sem_post(sem_res);
            break;
        }

        if (data->command == 1) {
            cout << "[C++] Recibido RUN" << endl;

            double rel_eps = (data->rel_eps_f > 0 ? data->rel_eps_f : 1e-7);

            using namespace chrono;
            auto start = high_resolution_clock::now();

            DefaultOptimizer opt(sys, rel_eps);
            opt.optimize(sys.box);

            auto end = high_resolution_clock::now();
            data->exec_time_ms =
                duration_cast<microseconds>(end - start).count() / 1000.0;

            data->loup = opt.get_loup();
            data->uplo = opt.get_uplo();

            const IntervalVector& iv = opt.get_loup_point();
            for (int i = 0; i < min(MAX_VARS, (int)iv.size()); i++)
                data->loup_point[i] = iv[i].mid();

            
            sem_post(sem_res);
        }
    }

    munmap(addr, sizeof(SharedData));
    close(fd);

    sem_close(sem_req);
    sem_close(sem_res);

    return 0;
}
