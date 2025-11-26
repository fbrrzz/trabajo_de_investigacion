#include "ibex.h"
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <vector>


#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;
using namespace ibex;

static const int MAX_VARS = 6;
static const int PORT = 5000;       


string trim(const string& s) {
    size_t start = s.find_first_not_of(" \n\r\t");
    size_t end = s.find_last_not_of(" \n\r\t");
    if (start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

int main() {
    cout << "[C++] Iniciando servidor Ibex en puerto " << PORT << "..." << endl;

    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return 1;
    }

    cout << "[C++] Esperando conexión..." << endl;

    int addrlen = sizeof(address);
    int client_fd = accept(server_fd, (struct sockaddr*)&address,
                           (socklen_t*)&addrlen);

    if (client_fd < 0) {
        perror("accept");
        return 1;
    }

    cout << "[C++] Cliente conectado." << endl;

    
    System sys("/home/labeit/Ibex/ibex-lib/benchs/optim/easy/ex3_1_3.bch"); //Cambiar la ruta

    while (true) {
        char buffer[4096] = {0};
        int valread = read(client_fd, buffer, 4096);

        if (valread <= 0) {
            cout << "[C++] Cliente desconectado." << endl;
            break;
        }

        string msg = trim(buffer);
        cout << "[C++] Recibido: " << msg << endl;

        if (msg == "exit") break;

        
        double rel_eps_f = 1e-7;

        if (msg.rfind("run ", 0) == 0) {
            rel_eps_f = atof(msg.substr(4).c_str());
        }

        
        using namespace std::chrono;
        auto start = high_resolution_clock::now();

        DefaultOptimizer opt(sys, rel_eps_f);
        opt.optimize(sys.box);

        auto end = high_resolution_clock::now();
        double exec_ms =
            duration_cast<microseconds>(end - start).count() / 1000.0;

        double loup = opt.get_loup();
        double uplo = opt.get_uplo();

        const IntervalVector& iv = opt.get_loup_point();
        vector<double> point;
        for (int i = 0; i < min(MAX_VARS, (int)iv.size()); i++)
            point.push_back(iv[i].mid());

        
        ostringstream json;
        json << "{";
        json << "\"status\":\"ok\",";
        json << "\"loup\":" << loup << ",";
        json << "\"uplo\":" << uplo << ",";
        json << "\"exec_time_ms\":" << exec_ms << ",";
        json << "\"point\":[";

        for (size_t i = 0; i < point.size(); i++) {
            json << point[i];
            if (i < point.size() - 1) json << ",";
        }
        json << "]}";

        json << "\n";

        string out = json.str();

        send(client_fd, out.c_str(), out.size(), 0);
        cout << "[C++] Respuesta enviada." << endl;
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
