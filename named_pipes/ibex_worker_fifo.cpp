#include "ibex.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;
using namespace ibex;

static const int MAX_VARS = 6;

int main() {
    cout << "[C++] Iniciando worker IBEX con FIFO..." << endl;

    
    ifstream fifo_req("fifo_req");
    ofstream fifo_res("fifo_res");
  

    if (!fifo_req.is_open() || !fifo_res.is_open()) {
        cerr << "[C++] Error abriendo FIFOs." << endl;
        return 1;
    }

    
    System sys("/home/labeit/Ibex/ibex-lib/benchs/optim/easy/ex3_1_3.bch"); //Cambiar la ruta

    string line;

    while (true) {
        getline(fifo_req, line);

        if (!fifo_req.good())
            continue;

        if (line == "EXIT") {
            cout << "[C++] Fin solicitado." << endl;
            break;
        }

        cout << "[C++] Recibido: " << line << endl;

        
        double rel_eps = 1e-7;
        if (line.rfind("RUN", 0) == 0) {
            rel_eps = atof(line.substr(4).c_str());
        }

        
        using namespace chrono;
        auto start = high_resolution_clock::now();

        DefaultOptimizer opt(sys, rel_eps);
        opt.optimize(sys.box);

        auto end = high_resolution_clock::now();
        double exec_ms = duration_cast<microseconds>(end - start).count() / 1000.0;

        double loup = opt.get_loup();
        double uplo = opt.get_uplo();

        const IntervalVector& iv = opt.get_loup_point();

        vector<double> point;
        for (int i = 0; i < min(MAX_VARS, (int)iv.size()); i++)
            point.push_back(iv[i].mid());

        
        ostringstream json;
        json << "{";
        json << "\"loup\":" << loup << ",";
        json << "\"uplo\":" << uplo << ",";
        json << "\"exec_time_ms\":" << exec_ms << ",";
        json << "\"point\":[";

        for (size_t i = 0; i < point.size(); i++) {
            json << point[i];
            if (i < point.size() - 1) json << ",";
        }
        json << "]}";

        fifo_res << json.str() << endl;
        fifo_res.flush();

        cout << "[C++] Respuesta enviada." << endl;
    }

    return 0;
}
