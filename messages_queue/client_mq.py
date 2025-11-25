import posix_ipc
import json

MQ_MSG_SIZE = 1024

def main():
    print("[Py] Abriendo colas de mensajes...")

    mq_req = posix_ipc.MessageQueue(
        "/ibex_req",
        flags=posix_ipc.O_CREAT,
        max_messages=10,
        max_message_size=MQ_MSG_SIZE,
    )

    mq_res = posix_ipc.MessageQueue(
        "/ibex_res",
        flags=posix_ipc.O_CREAT,
        max_messages=10,
        max_message_size=MQ_MSG_SIZE,
    )

    rel_eps = 1e-4
    msg = f"RUN {rel_eps}"

    print("[Py] Enviando:", msg)
    mq_req.send(msg)

    
    raw, _ = mq_res.receive()
    response = raw.decode()

    print("[Py] Recibido:", response)

   
    response = response.split('\x00', 1)[0].strip()

    data = json.loads(response)

    print(f"[Py] Intervalo mínimo: [{data['uplo']}, {data['loup']}]")
    print(f"[Py] Punto: {data['point']}")
    print(f"[Py] Tiempo CPU: {data['exec_time_ms']:.3f} ms")

    
    mq_req.send("EXIT")

if __name__ == "__main__":
    main()
