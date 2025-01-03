import grpc
from grpc_interceptor import AsyncServerInterceptor
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from datetime import datetime, timezone
import socket
import fcntl
from os import fsync
import aiofiles
import asyncio


class LogAndAuthInterceptor(AsyncServerInterceptor):
    def __init__(self, server_port, log_file):
        self.server_port = server_port
        self.log_file = log_file
        self._lock = asyncio.Lock()

    async def intercept(self, method, request, context, method_name):
        cert = context.auth_context()['x509_pem_cert'][0].decode(
            'utf-8') if context.auth_context() and 'x509_pem_cert' in context.auth_context() else None
        server_ip = socket.gethostbyname(socket.gethostname())
        current_timestamp = datetime.now(timezone.utc).strftime("%B %d, %Y at %I:%M %p %Z")
        if cert:
            cert_expiry_date = self.get_cert_expiry_date(cert).strftime("%B %d, %Y at %I:%M %p %Z")
            name, role = self.get_name_and_role(cert)
        else:
            cert_expiry_date = "NO CERTIFICATE FOUND."
            name, role = "UNKNOWN", "UNKNOWN"

        log_message = f"Client named '{name}' with role '{role}' called method '{method_name}' on server '{server_ip}:{self.server_port}' on {current_timestamp}. Their certificate expires on {cert_expiry_date}.\n"
        print(log_message)
        await self.async_write_to_log(log_message)

        if not cert:
            context.abort(grpc.StatusCode.UNAUTHENTICATED, "Invalid certificate provided.")
        else:
            if self.has_expired(cert):
                context.abort(grpc.StatusCode.UNAUTHENTICATED, "Provided an expired certificate.")
            if not self.has_access(self.get_name_and_role(cert)[1], method_name):
                context.abort(grpc.StatusCode.PERMISSION_DENIED, "Access denied.")

        return await method(request, context)

    async def async_write_to_log(self, message: str) -> None:
        async with self._lock:  # Ensure only one coroutine writes at a time
            async with aiofiles.open(self.log_file, 'a') as log_file:
                fd = log_file.fileno()
                fcntl.flock(fd, fcntl.LOCK_EX)
                try:
                    await log_file.write(message)
                    await log_file.flush()
                    fsync(fd)
                finally:
                    fcntl.flock(fd, fcntl.LOCK_UN)

    def has_expired(self, cert) -> bool:
        cert_obj = x509.load_pem_x509_certificate(cert.encode(), default_backend())
        return datetime.now(timezone.utc) > cert_obj.not_valid_after_utc

    def get_cert_expiry_date(self, cert) -> datetime:
        cert_obj = x509.load_pem_x509_certificate(cert.encode(), default_backend())
        return cert_obj.not_valid_after_utc

    def get_name_and_role(self, cert) -> tuple[str, str]:
        cert_obj = x509.load_pem_x509_certificate(cert.encode(), default_backend())
        cn = cert_obj.subject.get_attributes_for_oid(x509.NameOID.COMMON_NAME)
        if cn:
            name_role_list = cn[0].value.split('_')
            return name_role_list[0], name_role_list[1]

        return "UNAUTHORIZED", "UNAUTHORIZED"

    def has_access(self, role, method_name) -> bool:
        if role == "driver":
            return method_name in ["/MyUber.DriverServices/SubscribeToRideRequests",
                                   "/MyUber.DriverServices/CompleteRide"]
        elif role == "rider":
            return method_name in ["/MyUber.RiderServices/RequestRide",
                                   "/MyUber.RiderServices/GetRideStatus"]
        return False
