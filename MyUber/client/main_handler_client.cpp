/* Author: Kritin Maddireddy
   Filename: client.cpp
   Date and Time of file creation: 13/10/24, 5:16 PM
*/

#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <uuid/uuid.h>
#include "driver_client.cpp"
#include "rider_client.cpp"

grpc::SslCredentialsOptions performClientSidemTLS(const std::string& client_name, const std::string& client_role) {
    // client-side mTLS
    const std::string certificates_directory = "../32/Q3/certificate";

    const std::string generate_cert_cmd = "../32/Q3/utils/certificate_creation_script.sh " + certificates_directory + " " + client_name + " " + client_role;

    if (system(generate_cert_cmd.c_str()) != 0) {
        std::cerr << "Failed to generate certificates. Please check the bash script." << "\n";
        exit(1);
    }

    std::ifstream crt_file(certificates_directory + "/" + client_name + "_" + client_role + ".crt");
    std::string crt_file_str = std::string((std::istreambuf_iterator<char>(crt_file)),std::istreambuf_iterator<char>());
    std::ifstream key_file(certificates_directory + "/" + client_name + "_" + client_role + ".key");
    std::string key_file_str = std::string((std::istreambuf_iterator<char>(key_file)),std::istreambuf_iterator<char>());
    std::ifstream ca_file(certificates_directory + "/" + "ca.crt");
    std::string ca_file_str = std::string((std::istreambuf_iterator<char>(ca_file)),std::istreambuf_iterator<char>());

    grpc::SslCredentialsOptions ssl_opts;
    ssl_opts.pem_private_key = key_file_str, ssl_opts.pem_cert_chain = crt_file_str, ssl_opts.pem_root_certs = ca_file_str;
    return ssl_opts;
}

int main(int argc, char* argv[]){
    if (argc == 1) {
        std::cerr << "Please specify the role of the client. It has to be one from {rider, driver}.";
        return 1;
    }
    else if (argc == 2) {
        std::cerr << "Please specify a load balancing policy to use. It has to be one from {pick_first, round_robin}.";
        return 1;
    }
    else if (argc == 3) {
        std::cerr << "Please specify the number of servers to use, followed by the port number of each server.";
        return 1;
    }

    uuid_t uuid;
    char uuid_str[37];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, uuid_str);
    std::string client_name = uuid_str, client_role = argv[1], load_balancing_policy = argv[2];
    grpc::SslCredentialsOptions ssl_opts = performClientSidemTLS(client_name, client_role);

    int numServers = std::stoi(argv[3]);
    std::vector<std::string> server_ips(numServers);
    for(int i = 0; i < numServers; i++)
        server_ips[i] = "127.0.0.1:" + std::string(argv[i + 4]);

    if(client_role == "rider") {
        RiderClient riderClient(server_ips, ssl_opts, load_balancing_policy, client_name);
        riderClient.runClient();
    }
    else if(client_role == "driver") {
        DriverClient driverClient(server_ips, ssl_opts, load_balancing_policy, client_name);
        driverClient.runClient();
    }
}