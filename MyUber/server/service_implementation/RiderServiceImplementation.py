import json
from q3_pb2 import *
from q3_pb2_grpc import *
import redis.asyncio as redis

class RiderServiceImplementation(RiderServicesServicer):
    def __init__(self, redis_server):
        self.redis: redis.Redis = redis_server

    async def RequestRide(self, request, context):
        print("Ride Request received!")
        ride_id = await self.redis.incr("next_ride_id")
        print(f"Ride ID: {ride_id}")

        available_drivers = await self.redis.hgetall('driver:status')
        print(f"Available drivers: {available_drivers}")

        available_driver = None
        for driver_id, status in available_drivers.items():
             if status == "available":
                available_driver = driver_id
                break

        if not available_driver:
            return RideResponse(status=RideResponse.Status.NO_DRIVERS_AVAILABLE)

        await self.redis.hset(
            f"ride:{ride_id}",
            mapping={
                "ride_id": str(ride_id),
                "rider_id": request.rider_name,
                "pickup_location": request.source,
                "destination_location": request.destination,
                "status": "requested",
                "retry_count": "0"
            }
        )

        await self.redis.hset(
            f"ride:{ride_id}",
            "current_driver",
            available_driver
        )

        # update this driver's status as being busy (on_ride).
        await self.redis.hset(
            f"driver:status",
            available_driver,
            "on_ride"
        )

        # publish this ride request to the 'RideRequestChannel'
        ride_message = {
            "ride_id": ride_id,
            "driver_id": available_driver,
            "pickup_location": request.source,
            "destination_location": request.destination
        }
        await self.redis.publish('RideRequestChannel', json.dumps(ride_message))

        print(f"Ride request published for driver {available_driver}")
        return RideResponse(ride_id=ride_id, status=RideResponse.Status.SEARCHING_FOR_DRIVER)

    async def GetRideStatus(self, request, context):
        print("Ride Status Request received!")
        ride_status = await self.redis.hget(f"ride:{request.ride_id}", "status")

        if ride_status is None:
            return RideStatusResponse(status="RIDE_NOT_FOUND")

        return RideStatusResponse(status=ride_status)