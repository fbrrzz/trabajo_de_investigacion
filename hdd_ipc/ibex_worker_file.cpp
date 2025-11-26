#include "ibex.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <thread>
#include <cstdio>   

using namespace std;
using namespace ibex;

static const int MAX_VARS = 6;

bool file_exists(const string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main() {
    cout << "[C++] Worker IBEX con archivos (HDD) iniciado" << endl;

    const string REQ_FILE = "req.txt";
    const string RES_FILE = "res.json";

    
    System sys("/home/labeit/Ibex/ibex-lib/benchs/optim/easy/ex3_1_3.bch"); //Cambiar la ruta

    bool running = true;
    while (running) {

        
        if (!file_exists(REQ_FILE)) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }

        ifstream fin(REQ_FILE);
        string line;
        getline(fin, line);
        fin.close();

        
        std::remove(REQ_FILE.c_str());

        if (line == "EXIT") {
            cout << "[C++] Fin solicitado (EXIT en req.txt)." << endl;
            running = false;
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
        json << "\"loup\":" << loup << ",";
        json << "\"uplo\":" << uplo << ",";
        json << "\"exec_time_ms\":" << exec_ms << ",";
        json << "\"point\":[";
        for (size_t i = 0; i < point.size(); i++) {
            json << point[i];
            if (i < point.size() - 1) json << ",";
        }
        json << "]}";

        ofstream fout(RES_FILE, ios::trunc);
        fout << json.str() << endl;
        fout.close();

        cout << "[C++] Resultado escrito en " << RES_FILE << endl;
    }

    return 0;
}
