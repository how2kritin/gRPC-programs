import asyncio
import signal
import sys
import subprocess
import uuid
import redis.asyncio as redis
from q3_pb2_grpc import *
from interceptors.LogAndAuthInterceptor import LogAndAuthInterceptor
from service_implementation.RiderServiceImplementation import RiderServiceImplementation
from service_implementation.DriverServiceImplementation import DriverServiceImplementation

def perform_server_side_mtls(server_name):
    certificates_directory = "../certificate"
    generate_cert_cmd = f"../utils/certificate_creation_script.sh {certificates_directory} {server_name} server"

    try:
        subprocess.run(generate_cert_cmd, shell=True, check=True)
    except subprocess.CalledProcessError:
        print("Failed to generate certificates. Please check the bash script.")
        sys.exit(1)

    with open(f"{certificates_directory}/{server_name}_server.crt", "rb") as f:
        server_cert = f.read()
    with open(f"{certificates_directory}/{server_name}_server.key", "rb") as f:
        server_key = f.read()
    with open(f"{certificates_directory}/ca.crt", "rb") as f:
        ca_cert = f.read()

    server_credentials = grpc.ssl_server_credentials(
        [(server_key, server_cert)],
        root_certificates=ca_cert,
        require_client_auth=True
    )

    return server_credentials

async def serve():
    if len(sys.argv) != 2:
        print("Usage: python server.py <port>")
        sys.exit(1)

    server_name = uuid.uuid4()
    port = sys.argv[1]

    server_credentials = perform_server_side_mtls(server_name)

    redis_server = redis.Redis(host='localhost', port=6379, decode_responses=True)
    await redis_server.flushall()

    server = grpc.aio.server(interceptors=[LogAndAuthInterceptor(port, '.log')])
    add_RiderServicesServicer_to_server(RiderServiceImplementation(redis_server), server)
    add_DriverServicesServicer_to_server(DriverServiceImplementation(redis_server), server)
    server.add_secure_port(f"[::]:{port}", server_credentials)


    await server.start()
    print(f"MyUber Server is running on port {port}")
    print("Currently listening for both rider and driver requests.")

    def shutdown():
        shutdown_event.set()

    loop = asyncio.get_event_loop()
    shutdown_event = asyncio.Event()
    for signal_type in (signal.SIGINT, signal.SIGTERM):
        loop.add_signal_handler(signal_type, shutdown)

    await shutdown_event.wait()
    await server.stop(0)

if __name__ == "__main__":
    asyncio.run(serve())