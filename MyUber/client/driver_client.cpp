/* Author: Kritin Maddireddy
   Filename: driver_client.cpp
   Date and Time of file creation: 19/10/24, 8:52 PM
*/

#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include "MyUber/protofiles/q3.grpc.pb.h"
#include "MyUber/protofiles/q3.pb.h"

class DriverClientService{
public:
    DriverClientService(const std::string& server_ip, const grpc::SslCredentialsOptions& ssl_opts, const grpc::ChannelArguments& args) {
        channel = grpc::CreateCustomChannel(server_ip, grpc::SslCredentials(ssl_opts), args);
        stub = MyUber::DriverServices::NewStub(channel);
    }

    int32_t SubscribeToRideRequests(const std::string& driver_name) {
        grpc::ClientContext context;
        std::shared_ptr<grpc::ClientReaderWriter<MyUber::GetRideRequest, MyUber::RespondToRideRequest>> stream(stub->SubscribeToRideRequests(&context));

        // initial req with ride_id = -1 so that this driver subscribes to the server
        MyUber::GetRideRequest request;
        request.set_driver_name(driver_name);
        request.set_ride_id(-1);
        stream->Write(request);

        MyUber::RespondToRideRequest response;
        int32_t ride_id_accepted = -1;

        // keep reading until you accept ride
        while (ride_id_accepted == -1 && stream->Read(&response)) {
            if(response.status() == MyUber::RespondToRideRequest_Status_AVAILABLE) {
                std::cout << "Received ride details: " << response.source() << " to " << response.destination() << "\n";

                int32_t choice;
                std::cout << "The request will timeout in 10 seconds. Enter 1 to accept this ride, or enter any other integer to reject this ride: ";
                std::cin >> choice;

                // attempt to accept or reject the ride
                if (choice == 1) {
                    request.set_ride_id(response.ride_id());
                    request.set_request(MyUber::GetRideRequest::ACCEPTED);
                    stream->Write(request);
                    std::cout << "Sent a request to accept the ride." << std::endl;

                    // wait for the server to confirm acceptance or timeout
                    while (stream->Read(&response)) {
                        if (response.status() == MyUber::RespondToRideRequest_Status_ACCEPTED) {
                            std::cout << "Ride accepted successfully." << std::endl;
                            ride_id_accepted = response.ride_id();
                            break;
                        } else if (response.status() == MyUber::RespondToRideRequest_Status_TIMED_OUT) {
                            std::cout << "Ride acceptance timed out." << std::endl;
                            break;
                        }
                    }
                } else {
                    // reject the ride
                    request.set_ride_id(response.ride_id());
                    request.set_request(MyUber::GetRideRequest::REJECTED);
                    stream->Write(request);
                    std::cout << "Successfully rejected the ride." << std::endl;
                }
            }
        }

        // once client accepts ride, you break out of while loop and reach here. just end the connection.
        stream->WritesDone();
        grpc::Status status = stream->Finish();
        if (!status.ok()) std::cout << "RPC failed. Error: " << status.error_message() << std::endl;
        return ride_id_accepted;
    }

    bool completeRide(int32_t ride_id) {
        // returns true if successful, false if failed.
        grpc::ClientContext context;

        MyUber::RideCompletionRequest request;
        MyUber::RideCompletionResponse response;

        request.set_ride_id(ride_id);

        grpc::Status status = stub->CompleteRide(&context, request, &response);

        if (status.ok() && response.ride_id() == ride_id) {
            std::cout << "Successfully marked the ride as completed. Ride ID: " << response.ride_id() << "\n";
            return true;
        } else {
            std::cerr << "Failed to mark ride as completed. Please try again later. Error: " << status.error_message() << "\n";
            return false;
        }
    }

private:
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<MyUber::DriverServices::Stub> stub;
};

class DriverClient{
public:
    DriverClient(const std::vector<std::string>& server_ips, const grpc::SslCredentialsOptions& ssl_opts, const std::string& load_balancing_policy, const std::string& driver_name){
        this->load_balancing_policy = load_balancing_policy;
        this->driver_name = driver_name;

        grpc::ChannelArguments args;
        args.SetLoadBalancingPolicyName(load_balancing_policy);
        args.SetSslTargetNameOverride("localhost");
        std::string addresses;
        for (int i = 0; i < server_ips.size(); i++) {
            if (i > 0) addresses += ",";
            addresses += server_ips[i];
        }
        this->server = new DriverClientService("ipv4:" + addresses, ssl_opts, args);
    }

    void runClient() {
        while(true){
            std::cout << "Waiting to receive a ride request...\n";
            int32_t accepted_ride_id = server->SubscribeToRideRequests(driver_name);
            if(accepted_ride_id == -1) continue;
            std::cout << "Ride ID: " << accepted_ride_id << "\n";

            int32_t choice = 0;
            std::cout << "Please enter 1 to mark the ride as completed once you drop the rider off: ";
            while(choice != 1) std::cin >> choice;

            server->completeRide(accepted_ride_id);
        }
    }
private:
    DriverClientService* server;
    std::string load_balancing_policy;
    std::string driver_name;
};