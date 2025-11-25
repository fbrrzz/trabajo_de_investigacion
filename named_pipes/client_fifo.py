import json
import time

def main():

    fifo_req = open("fifo_req", "w")
    fifo_res = open("fifo_res", "r")


    print("[Py] Enviando RUN...")

    rel_eps = 1e-4

    
    fifo_req.write(f"RUN {rel_eps}\n")
    fifo_req.flush()

    
    response = fifo_res.readline().strip()

    print("[Py] Respuesta bruta:", response)

    data = json.loads(response)

    print(f"[Py] Intervalo mínimo: [{data['uplo']}, {data['loup']}]")
    print(f"[Py] Punto: {data['point']}")
    print(f"[Py] Tiempo CPU: {data['exec_time_ms']:.3f} ms")

    
    fifo_req.write("EXIT\n")
    fifo_req.flush()

if __name__ == "__main__":
    main()
