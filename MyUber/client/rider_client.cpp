/* Author: Kritin Maddireddy
   Filename: rider_client.cpp
   Date and Time of file creation: 19/10/24, 8:52 PM
*/

#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include "MyUber/protofiles/q3.grpc.pb.h"
#include "MyUber/protofiles/q3.pb.h"

class RiderClientService{
public:
    RiderClientService(const std::string& server_ip, const grpc::SslCredentialsOptions& ssl_opts, const grpc::ChannelArguments& args) {
        channel = grpc::CreateCustomChannel(server_ip, grpc::SslCredentials(ssl_opts), args);
        stub = MyUber::RiderServices::NewStub(channel);
    }

    int32_t requestRide(std::string source, std::string destination, std::string rider_name) {
        grpc::ClientContext context;

        MyUber::RideRequest request;
        MyUber::RideResponse response;

        request.set_source(source);
        request.set_destination(destination);
        request.set_rider_name(rider_name);

        grpc::Status status = stub->RequestRide(&context, request, &response);

        if (status.ok() && response.status() == MyUber::RideResponse_Status::RideResponse_Status_SEARCHING_FOR_DRIVER) {
            std::cout << "Successfully requested a ride. Ride ID: " << response.ride_id() << "\n";
            return response.ride_id();
        } else if (status.ok() && response.status() == MyUber::RideResponse_Status::RideResponse_Status_NO_DRIVERS_AVAILABLE) {
            std::cout << "No drivers are currently available. Please try again later." << "\n";
            return -1;
        } else {
            std::cerr << "Failed to request a ride. Please try again later. Error: " << status.error_message() << "\n";
            return -1;
        }
    }

    void getRideStatus(int32_t ride_id) {
        grpc::ClientContext context;

        MyUber::RideStatusRequest request;
        MyUber::RideStatusResponse response;

        request.set_ride_id(ride_id);

        grpc::Status status = stub->GetRideStatus(&context, request, &response);

        if (status.ok()) std::cout << "Successfully obtained status of your ride with ride ID: " << ride_id << ". Status: " << response.status() << ".\n";
        else std::cerr << "Failed to get the status of your ride. Please try again later. Error: " << status.error_message() << "\n";
    }
private:
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<MyUber::RiderServices::Stub> stub;
};

class RiderClient{
public:
    RiderClient(const std::vector<std::string>& server_ips, const grpc::SslCredentialsOptions& ssl_opts, const std::string& load_balancing_policy, const std::string& client_name){
        this->load_balancing_policy = load_balancing_policy;
        this->client_name = client_name;

        grpc::ChannelArguments args;
        args.SetLoadBalancingPolicyName(load_balancing_policy);
        args.SetSslTargetNameOverride("localhost");
        std::string addresses;
        for (int i = 0; i < server_ips.size(); i++) {
            if (i > 0) addresses += ",";
            addresses += server_ips[i];
        }
        this->server = new RiderClientService("ipv4:" + addresses, ssl_opts, args);
    }

    void runClient() {
        while(true){
            std::cout << "Enter an integer:\n";
            std::cout << "1: Request a ride\n";
            std::cout << "2: Get ride status\n";

            int32_t choice;
            std::cin >> choice;

            switch(choice){
                case 1: {
                    std::string source, destination;
                    std::cout << "Enter name of source location: ";
                    std::cin >> source;
                    std::cout << "Enter name of destination location: ";
                    std::cin >> destination;

                    int32_t ride_id = server->requestRide(source, destination, client_name);
                    if (ride_id != -1) rideIDs.insert(ride_id);
                    break;
                }
                case 2: {
                    int32_t ride_id;
                    std::cout << "Enter ride ID of a ride you've requested, whose status you would like to check: ";
                    std::cin >> ride_id;

                    if (rideIDs.find(ride_id) == rideIDs.end()) std::cout << "Could not find a ride that you have requested with that ID.\n";
                    else server->getRideStatus(ride_id);
                    break;
                }
                default: {
                    std::cout << "Invalid choice. Please enter an integer from the options presented.\n\n";
                    continue;
                }
            }
        }
    }
private:
    RiderClientService* server;
    std::set<int32_t> rideIDs;
    std::string load_balancing_policy;
    std::string client_name;
};