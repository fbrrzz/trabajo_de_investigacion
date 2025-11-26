COMANDOS PARA EJECUTAR CADA COMUNICACIÓN: 


Comunicación con HDD
=============================

En una terminal 1: 
cd hdd_ipc 

g++ ibex_worker_file.cpp -o ibex_worker_file $(pkg-config --cflags --libs ibex) -lrt

python3 client_file.py



Memoria Compartida 
===========================

En terminal 1: 

cd memoria 

g++ ibex_worker.cpp -o ibex_worker $(pkg-config --cflags --libs ibex) -lrt


python3 controller.py



Message Queue
=========================

En terminal 1:

cd messages_queue

g++ ibex_worker_mq.cpp -o ibex_worker_mq $(pkg-config --cflags --libs ibex) -lrt

./ibex_worker_mq


En otra terminal 2: 

python3 client_mq.py




Named Pipes
=========================

En terminal 1: 

cd named_pipes

mkfifo fifo_req

mkfifo fifo_res

g++ ibex_worker_fifo.cpp -o ibex_worker_fifo $(pkg-config --cflags --libs ibex) -lrt

./ibex_worker_fifo 


En otra terminal 2: 

python3 client_fifo.py


Semaforos
========================

En terminal 1:

cd semaforos 

g++ ibex_worker_sem.cpp -o ibex_worker_sem $(pkg-config --cflags --libs ibex) -lrt -lpthread

./ibex_worker_sem


En otra terminal 2: 

python3 client_sem.py



Sockets
=========================

En terminal 1: 

cd sockets

g++ ibex_server.cpp -o ibex_server $(pkg-config --cflags --libs ibex) -lrt

./ibex_server


En otra terminal 2: 

python3 client.py
