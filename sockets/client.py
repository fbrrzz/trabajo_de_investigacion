import socket
import json

HOST = "127.0.0.1"
PORT = 5000

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    print("[Py] Conectado al servidor C++")

    
    rel_eps = 1e-4
    msg = f"run {rel_eps}\n"
    sock.sendall(msg.encode())

    print("[Py] Enviado:", msg.strip())

    
    data = sock.recv(4096).decode()
    print("[Py] Recibido:", data)

    response = json.loads(data)

    print(f"[Py] Intervalo mínimo: [{response['uplo']}, {response['loup']}]")
    print(f"[Py] Punto minimizante: {response['point']}")
    print(f"[Py] Tiempo CPU: {response['exec_time_ms']:.3f} ms")

    
    sock.sendall(b"exit\n")
    sock.close()

if __name__ == "__main__":
    main()
