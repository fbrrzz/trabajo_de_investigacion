#include "ibex.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <mqueue.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace ibex;

static const int MAX_VARS = 6;
static const long MQ_MSG_SIZE = 1024;   

int main() {

    cout << "[C++] Worker IBEX con Message Queues iniciado..." << endl;

    
    mq_unlink("/ibex_req");
    mq_unlink("/ibex_res");

    
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = 10;             
    attr.mq_msgsize = MQ_MSG_SIZE;    
    attr.mq_curmsgs = 0;

    
    mqd_t mq_req = mq_open("/ibex_req",
                           O_RDONLY | O_CREAT,
                           0666,
                           &attr);

    mqd_t mq_res = mq_open("/ibex_res",
                           O_WRONLY | O_CREAT,
                           0666,
                           &attr);

    if (mq_req == -1 || mq_res == -1) {
        perror("mq_open");
        return 1;
    }

    
    System sys("/home/labeit/Ibex/ibex-lib/benchs/optim/easy/ex3_1_3.bch");

    char buffer[MQ_MSG_SIZE];

    while (true) {

        memset(buffer, 0, MQ_MSG_SIZE);

        
        ssize_t bytes_read = mq_receive(mq_req, buffer, MQ_MSG_SIZE, NULL);

        if (bytes_read < 0) {
            perror("mq_receive");
            break;
        }

        string msg(buffer);

        if (msg == "EXIT") {
            cout << "[C++] Fin solicitado." << endl;
            break;
        }

        cout << "[C++] Recibido: " << msg << endl;

        
        double rel_eps = 1e-7;
        if (msg.rfind("RUN", 0) == 0) {
            rel_eps = atof(msg.substr(4).c_str());
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

        string out = json.str();

        if (out.size() + 1 > (size_t)MQ_MSG_SIZE) {
            cerr << "[C++] Error: JSON más grande que MQ_MSG_SIZE" << endl;
        } else {
            mq_send(mq_res, out.c_str(), out.size() + 1, 0);
            cout << "[C++] Respuesta enviada." << endl;
        }
    }

    mq_close(mq_req);
    mq_close(mq_res);
    mq_unlink("/ibex_req");
    mq_unlink("/ibex_res");

    return 0;
}
