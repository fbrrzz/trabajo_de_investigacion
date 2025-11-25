import ctypes
import time
from multiprocessing import shared_memory
import subprocess

MAX_VARS = 6

class SharedData(ctypes.Structure):
    _fields_ = [
        ("command", ctypes.c_int),
        ("rel_eps_f", ctypes.c_double),
        ("loup", ctypes.c_double),
        ("uplo", ctypes.c_double),
        ("loup_point", ctypes.c_double * MAX_VARS),
        ("exec_time_ms", ctypes.c_double),   # tiempo CPU
    ]

def main():
    shm_name = "ibex_shm"
    size = ctypes.sizeof(SharedData)

   
    try:
        shm = shared_memory.SharedMemory(name=shm_name, create=True, size=size)
        is_creator = True
    except FileExistsError:
        shm = shared_memory.SharedMemory(name=shm_name, create=False, size=size)
        is_creator = False

    buf = shm.buf
    shared = SharedData.from_buffer(buf)

    if is_creator:
        shared.command = 0
        shared.rel_eps_f = 1e-7
        shared.loup = 0.0
        shared.uplo = 0.0
        shared.exec_time_ms = 0.0
        for i in range(MAX_VARS):
            shared.loup_point[i] = 0.0

    
    worker_proc = subprocess.Popen(["./ibex_worker"])

    
    shared.rel_eps_f = 1e-4
    shared.command = 1

    print("[Py] Comando RUN enviado, esperando resultado...")

    while shared.command != 2:
        time.sleep(0.01)

    print("[Py] Resultado recibido")
    print(f"[Py] Intervalo mínimo: [{shared.uplo}, {shared.loup}]")

    point = [shared.loup_point[i] for i in range(MAX_VARS)]
    print(f"[Py] Punto minimizante aproximado: {point}")

    print(f"[Py] Tiempo CPU del solver: {shared.exec_time_ms:.3f} ms")

    
    shared.command = 3
    time.sleep(0.1)
    worker_proc.wait()

    
    del shared
    del buf
    shm.close()
    if is_creator:
        shm.unlink()

if __name__ == "__main__":
    main()
