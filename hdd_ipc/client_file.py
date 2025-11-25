import os
import time
import json
import subprocess

REQ_FILE = "req.txt"
RES_FILE = "res.json"

def wait_for_file(path, timeout=10.0):
    start = time.time()
    while not os.path.exists(path):
        if time.time() - start > timeout:
            raise TimeoutError(f"Timeout esperando archivo {path}")
        time.sleep(0.1)

def main():
    
    for f in (REQ_FILE, RES_FILE):
        if os.path.exists(f):
            os.remove(f)

    
    worker = subprocess.Popen(["./ibex_worker_file"])

    
    rel_eps = 1e-4
    print("[Py] Escribiendo petición RUN en req.txt")

    with open(REQ_FILE, "w") as f:
        f.write(f"RUN {rel_eps}\n")

    
    print("[Py] Esperando resultado en res.json")
    wait_for_file(RES_FILE, timeout=30.0)

    with open(RES_FILE, "r") as f:
        response = f.read().strip()

    print("[Py] Respuesta bruta:", response)

    data = json.loads(response)

    print(f"[Py] Intervalo mínimo: [{data['uplo']}, {data['loup']}]")
    print(f"[Py] Punto minimizante: {data['point']}")
    print(f"[Py] Tiempo CPU: {data['exec_time_ms']:.3f} ms")

    
    print("[Py] Enviando EXIT al worker")
    with open(REQ_FILE, "w") as f:
        f.write("EXIT\n")

    
    worker.wait()

    print("[Py] Worker terminado.")

if __name__ == "__main__":
    main()
