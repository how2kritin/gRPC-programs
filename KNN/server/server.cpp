/* Author: Kritin Maddireddy
   Filename: server.cpp
   Date and Time of file creation: 13/10/24, 5:16 PM
*/

#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include "KNN/protofiles/q2.grpc.pb.h"
#include "KNN/protofiles/q2.pb.h"

float calcEuclideanDistance(const std::vector<float>& p1, const std::vector<float>& p2) {
    float sum = 0;
    for (int i = 0; i < p1.size(); i++) sum += (p2[i] - p1[i]) * (p2[i] - p1[i]);
    return std::sqrt(sum);
}

class CallData {
public:
    CallData(KNN::KNNPerformerService::AsyncService* service, grpc::ServerCompletionQueue* cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
        Proceed();
    }

    void Proceed() {
        if (status_ == CREATE) {
            status_ = PROCESS;
            service_->RequestFindKNearestNeighbors(&ctx_, &request_, &responder_, cq_, cq_, this);
        } else if (status_ == PROCESS) {
            new CallData(service_, cq_);

            std::cout << "Received a request!\n";
            int k = request_.k();
            std::vector<float> query_point(request_.query_point().coord().begin(), request_.query_point().coord().end());

            // for each point, calculate distances and store neighbours.
            std::vector<std::pair<std::vector<float>, float>> neighbours;
            neighbours.reserve(request_.data_points_size());  // preallocate to avoid reallocations
            for (const auto& point : request_.data_points()){
                std::vector<float> datapoint(point.coord().begin(), point.coord().end());
                float dist = calcEuclideanDistance(query_point, datapoint);
                neighbours.emplace_back(datapoint, dist);
            }

            // partial sort by distance and select top-k, if this server got k elements. else, just return all of them in sorted order, after calculating distances.
            int effective_k = std::min(k, static_cast<int>(neighbours.size()));
            if (effective_k > 0) {
                std::partial_sort(neighbours.begin(),
                                  neighbours.begin() + effective_k,
                                  neighbours.end(),
                                  [](const auto& a, const auto& b) { return a.second < b.second; });

                for (int i = 0; i < effective_k; i++) {
                    KNN::Neighbour* neighbour = response_.add_neighbour_points();
                    auto* point = new KNN::Point;
                    for(float coord : neighbours[i].first) point->add_coord(coord);
                    neighbour->set_allocated_point(point);
                    neighbour->set_distance(neighbours[i].second);
                }
            }

            status_ = FINISH;
            responder_.Finish(response_, grpc::Status::OK, this);
        } else {
            assert(status_ == FINISH);
            delete this;
        }
    }

private:
    KNN::KNNPerformerService::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    KNN::KNNRequest request_;
    KNN::KNNResponse response_;
    grpc::ServerAsyncResponseWriter<KNN::KNNResponse> responder_;

    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
};

class AsyncServerImpl final {
public:
    ~AsyncServerImpl() {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run(const std::string& address) {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "KNN Server is running on " << address << "\n" << "Currently listening for requests.\n";
        HandleRpcs();
    }

private:
    void HandleRpcs() {
        new CallData(&service_, cq_.get());
        void* tag;
        bool ok;
        while (true) {
            assert(cq_->Next(&tag, &ok));
            assert(ok);
            static_cast<CallData*>(tag)->Proceed();
        }
    }

    KNN::KNNPerformerService::AsyncService service_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    std::unique_ptr<grpc::Server> server_;
};

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "Please provide a port number to run this server on.";
        return 1;
    }
    std::string serverAddress = "0.0.0.0:" + std::string(argv[1]);

    AsyncServerImpl server;
    server.Run(serverAddress);

    return 0;
}