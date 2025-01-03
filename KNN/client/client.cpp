/* Author: Kritin Maddireddy
   Filename: client.cpp
   Date and Time of file creation: 13/10/24, 5:16 PM
*/

#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include "KNN/protofiles/q2.grpc.pb.h"
#include "KNN/protofiles/q2.pb.h"

std::vector<KNN::Point> readDataset(const std::string& filename) {
    std::vector<KNN::Point> dataset;
    std::string line;
    std::ifstream file(filename);
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        KNN::Point data_point;
        std::string value;
        while (std::getline(ss, value, ','))
            data_point.add_coord(std::stof(value));
        dataset.push_back(data_point);
    }
    file.close();
    return dataset;
}

std::vector<std::vector<KNN::Point>> partitionDataset(const std::vector<KNN::Point>& dataset, int numPartitions) {
    std::vector<std::vector<KNN::Point>> partitions;
    int numElems = dataset.size() / numPartitions;
    int rem = dataset.size() % numPartitions;

    auto itr = dataset.begin();
    for (int i = 0; i < numPartitions; i++) {
        int current_partition_size = numElems + (i < rem);
        partitions.emplace_back(itr, itr + current_partition_size);
        itr += current_partition_size;
    }

    return partitions;
}

class Server {
public:
    explicit Server(const std::shared_ptr<grpc::Channel>& channel) : stub(KNN::KNNPerformerService::NewStub(channel)) {}

    void AsyncFindKNearestNeighbors(const KNN::KNNRequest& request, grpc::CompletionQueue* cq) {
        auto* call = new AsyncServerCall;
        call->response_reader = stub->AsyncFindKNearestNeighbors(&call->context, request, cq);
        call->response_reader->Finish(&call->response, &call->status, (void*)call);
    }

    struct AsyncServerCall {
        KNN::KNNResponse response;
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<KNN::KNNResponse>> response_reader;
    };

private:
    std::unique_ptr<KNN::KNNPerformerService::Stub> stub;
};

// as ownership of point is transferred to set_allocated_query_point() function of request after giving it the point; need to copy point to reuse it.
KNN::Point* copyPoint(const KNN::Point* original) {
    auto* copy = new KNN::Point();
    for (int i = 0; i < original->coord_size(); i++)
        copy->add_coord(original->coord(i));
    return copy;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "Please specify the number of servers to use, followed by the port number of each server.";
        return 1;
    }

    std::string dataset_filename;
    std::cout << "Please provide the path to the dataset that you'd like to use: ";
    std::cin >> dataset_filename;

    int numServers = std::stoi(argv[1]);
    std::vector<std::string> server_ips(numServers);
    for(int i = 0; i < numServers; i++)
        server_ips[i] = "localhost:" + std::string(argv[i + 2]);
    std::vector<KNN::Point> dataset = readDataset(dataset_filename);
    std::vector<std::vector<KNN::Point>> partitions = partitionDataset(dataset, numServers);

    // array to handle async responses.
    grpc::CompletionQueue cq;

    // array to store async servers.
    std::vector<std::unique_ptr<Server>> servers;
    for (int i = 0; i < numServers; i++)
        servers.push_back(std::make_unique<Server>(grpc::CreateChannel(server_ips[i], grpc::InsecureChannelCredentials())));

    while(true) {
        // get point for this query
        int k, numCoords;
        float query_coord;
        auto* query_point = new KNN::Point();

        std::cout << "\nWaiting for a new query...\n";
        std::cin >> k >> numCoords;
        for(int i = 0; i < numCoords; i++) {
            std::cin >> query_coord;
            query_point->add_coord(query_coord);
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        // set up and send requests to each server
        for(int s = 0; s < numServers; s++) {
            KNN::KNNRequest request;
            request.set_k(k);
            KNN::Point* point_cpy = copyPoint(query_point);
            request.set_allocated_query_point(point_cpy);
            for (const auto &data_point: partitions[s]) {
                auto datapoint = request.add_data_points();
                for (int j = 0; j < data_point.coord_size(); j++)
                    datapoint->add_coord(data_point.coord(j));
            }
            servers[s]->AsyncFindKNearestNeighbors(request, &cq);
        }

        delete query_point;  // clean up the query point after you're done with it

        // now, collect all the responses async.
        int numResponses = 0;
        std::vector<std::pair<std::vector<float>, float>> allNeighbours;

        while (numResponses < numServers) { // need to receive 1 response from each server.
            void* obtainedTag;
            bool receivedOKResponse = false;
            cq.Next(&obtainedTag, &receivedOKResponse);

            if (receivedOKResponse) {
                auto call = static_cast<Server::AsyncServerCall*>(obtainedTag);

                if (call->status.ok()) {
                    for (const auto& neighbour : call->response.neighbour_points()) {
                        std::vector<float> neighbour_point;
                        for(int i = 0; i < neighbour.point().coord_size(); i++) neighbour_point.push_back(neighbour.point().coord(i));
                        allNeighbours.emplace_back(neighbour_point, neighbour.distance());
                    }
                } else
                    std::cerr << "RPC failed." << std::endl;

                numResponses++;
                delete call;
            }
        }

        // partial sort here
        std::partial_sort(allNeighbours.begin(), allNeighbours.begin() + k, allNeighbours.end(),[](const auto& a, const auto& b) { return a.second < b.second; });
        auto final_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < allNeighbours.size() && i < k; i++) {
            std::cout << "Neighbor " << i + 1 << ": ( ";
            for (float coord : allNeighbours[i].first)
                std::cout << coord << " ";
            std::cout << ") is at a distance: " << allNeighbours[i].second << std::endl;
        }

        std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::microseconds>(final_time - start_time).count() / 1e6 << "s\n";
    }
}