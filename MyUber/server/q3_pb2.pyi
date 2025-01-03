from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class RideRequest(_message.Message):
    __slots__ = ("rider_name", "source", "destination")
    RIDER_NAME_FIELD_NUMBER: _ClassVar[int]
    SOURCE_FIELD_NUMBER: _ClassVar[int]
    DESTINATION_FIELD_NUMBER: _ClassVar[int]
    rider_name: str
    source: str
    destination: str
    def __init__(self, rider_name: _Optional[str] = ..., source: _Optional[str] = ..., destination: _Optional[str] = ...) -> None: ...

class RideResponse(_message.Message):
    __slots__ = ("ride_id", "status")
    class Status(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
        __slots__ = ()
        SEARCHING_FOR_DRIVER: _ClassVar[RideResponse.Status]
        NO_DRIVERS_AVAILABLE: _ClassVar[RideResponse.Status]
    SEARCHING_FOR_DRIVER: RideResponse.Status
    NO_DRIVERS_AVAILABLE: RideResponse.Status
    RIDE_ID_FIELD_NUMBER: _ClassVar[int]
    STATUS_FIELD_NUMBER: _ClassVar[int]
    ride_id: int
    status: RideResponse.Status
    def __init__(self, ride_id: _Optional[int] = ..., status: _Optional[_Union[RideResponse.Status, str]] = ...) -> None: ...

class RideStatusRequest(_message.Message):
    __slots__ = ("ride_id",)
    RIDE_ID_FIELD_NUMBER: _ClassVar[int]
    ride_id: int
    def __init__(self, ride_id: _Optional[int] = ...) -> None: ...

class RideStatusResponse(_message.Message):
    __slots__ = ("status", "driver_name")
    STATUS_FIELD_NUMBER: _ClassVar[int]
    DRIVER_NAME_FIELD_NUMBER: _ClassVar[int]
    status: str
    driver_name: str
    def __init__(self, status: _Optional[str] = ..., driver_name: _Optional[str] = ...) -> None: ...

class GetRideRequest(_message.Message):
    __slots__ = ("ride_id", "driver_name", "request")
    class Request(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
        __slots__ = ()
        ACCEPTED: _ClassVar[GetRideRequest.Request]
        REJECTED: _ClassVar[GetRideRequest.Request]
    ACCEPTED: GetRideRequest.Request
    REJECTED: GetRideRequest.Request
    RIDE_ID_FIELD_NUMBER: _ClassVar[int]
    DRIVER_NAME_FIELD_NUMBER: _ClassVar[int]
    REQUEST_FIELD_NUMBER: _ClassVar[int]
    ride_id: int
    driver_name: str
    request: GetRideRequest.Request
    def __init__(self, ride_id: _Optional[int] = ..., driver_name: _Optional[str] = ..., request: _Optional[_Union[GetRideRequest.Request, str]] = ...) -> None: ...

class RespondToRideRequest(_message.Message):
    __slots__ = ("ride_id", "source", "destination", "status")
    class Status(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
        __slots__ = ()
        AVAILABLE: _ClassVar[RespondToRideRequest.Status]
        ACCEPTED: _ClassVar[RespondToRideRequest.Status]
        TIMED_OUT: _ClassVar[RespondToRideRequest.Status]
    AVAILABLE: RespondToRideRequest.Status
    ACCEPTED: RespondToRideRequest.Status
    TIMED_OUT: RespondToRideRequest.Status
    RIDE_ID_FIELD_NUMBER: _ClassVar[int]
    SOURCE_FIELD_NUMBER: _ClassVar[int]
    DESTINATION_FIELD_NUMBER: _ClassVar[int]
    STATUS_FIELD_NUMBER: _ClassVar[int]
    ride_id: int
    source: str
    destination: str
    status: RespondToRideRequest.Status
    def __init__(self, ride_id: _Optional[int] = ..., source: _Optional[str] = ..., destination: _Optional[str] = ..., status: _Optional[_Union[RespondToRideRequest.Status, str]] = ...) -> None: ...

class RideCompletionRequest(_message.Message):
    __slots__ = ("ride_id",)
    RIDE_ID_FIELD_NUMBER: _ClassVar[int]
    ride_id: int
    def __init__(self, ride_id: _Optional[int] = ...) -> None: ...

class RideCompletionResponse(_message.Message):
    __slots__ = ("ride_id",)
    RIDE_ID_FIELD_NUMBER: _ClassVar[int]
    ride_id: int
    def __init__(self, ride_id: _Optional[int] = ...) -> None: ...
