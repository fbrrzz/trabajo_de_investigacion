import ctypes
import time
from multiprocessing import shared_memory
import posix_ipc

MAX_VARS = 6

class SharedData(ctypes.Structure):
    _fields_ = [
        ("command", ctypes.c_int),
        ("rel_eps_f", ctypes.c_double),
        ("loup", ctypes.c_double),
        ("uplo", ctypes.c_double),
        ("loup_point", ctypes.c_double * MAX_VARS),
        ("exec_time_ms", ctypes.c_double),
    ]

def main():
    shm_name = "ibex_shm2"
    size = ctypes.sizeof(SharedData)

    
    try:
        shm = shared_memory.SharedMemory(name=shm_name, create=True, size=size)
        creator = True
    except FileExistsError:
        shm = shared_memory.SharedMemory(name=shm_name, create=False, size=size)
        creator = False

    buf = shm.buf
    shared = SharedData.from_buffer(buf)

    
    sem_req = posix_ipc.Semaphore("/ibex_sem_req", flags=posix_ipc.O_CREAT, initial_value=0)
    sem_res = posix_ipc.Semaphore("/ibex_sem_res", flags=posix_ipc.O_CREAT, initial_value=0)

   
    if creator:
        shared.command = 0
        shared.rel_eps_f = 1e-7
        shared.loup = 0
        shared.uplo = 0
        shared.exec_time_ms = 0
        for i in range(MAX_VARS):
            shared.loup_point[i] = 0

    print("[Py] Enviando RUN...")

    
    shared.rel_eps_f = 1e-4
    shared.command = 1

    
    sem_req.release()

    
    sem_res.acquire()

    print("[Py] Resultado recibido:")
    print(f"   Intervalo mínimo: [{shared.uplo}, {shared.loup}]")
    print(f"   Punto: {[shared.loup_point[i] for i in range(MAX_VARS)]}")
    print(f"   Tiempo CPU: {shared.exec_time_ms:.3f} ms")

    
    shared.command = 3
    sem_req.release()
    sem_res.acquire()

    print("[Py] Worker terminado.")

    del shared
    del buf
    shm.close()
    if creator:
        shm.unlink()

if __name__ == "__main__":
    main()
