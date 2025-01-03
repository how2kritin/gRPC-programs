import asyncio
import json
from time import sleep
from q3_pb2 import *
from q3_pb2_grpc import *
import redis.asyncio as redis

class DriverServiceImplementation(DriverServicesServicer):
    def __init__(self, redis_server):
        self.redis: redis.Redis = redis_server
        # keep track of active subscriptions per driver
        self.active_subscriptions = {}

    async def SubscribeToRideRequests(self, request_iterator, context):
        # create a new pubsub instance for each subscriber
        pubsub = self.redis.pubsub()

        try:
            await pubsub.subscribe('RideRequestChannel')

            async for request in request_iterator:
                if request.ride_id == -1:
                    driver_name = request.driver_name

                    # store the pubsub instance for this driver
                    self.active_subscriptions[driver_name] = pubsub

                    # mark driver as available
                    await self.redis.hset("driver:status", driver_name, "available")
                    print(f"Subscription active for driver {driver_name}")

                    while True:
                        # small delay, as without this sometimes the CPU just spins
                        await asyncio.sleep(0.1)

                        # wait until a message is received
                        message = await pubsub.get_message(ignore_subscribe_messages=True)
                        if not message:
                            continue
                        ride_details = json.loads(message['data'])

                        # only if this driver is assigned this ride, proceed. else, just ignore this ride request as it has nothing to do with you.
                        print(driver_name, ride_details["driver_id"])
                        if ride_details["driver_id"] == driver_name:
                            print(f"Received a ride request: {ride_details}")
                            print(f"Sending this ride request to driver {driver_name}")

                            # send this ride request to the corresponding driver.
                            response = RespondToRideRequest(
                                ride_id=ride_details['ride_id'],
                                source=ride_details['pickup_location'],
                                destination=ride_details['destination_location'],
                                status=RespondToRideRequest.Status.AVAILABLE
                            )
                            await context.write(response)

                            # set a timeout of 10 seconds and wait for their response.
                            try:
                                print("Waiting for the driver to respond...")
                                client_response = await asyncio.wait_for(request_iterator.__anext__(), timeout=10)
                                print(f"Received the driver's response: {client_response}")

                                # if the driver responds in time, check their response.
                                if client_response.request == GetRideRequest.Request.ACCEPTED:
                                    # if the driver has accepted the ride
                                    await self.redis.hset(f"ride:{ride_details['ride_id']}", "status", "in_progress")
                                    acceptance_response = RespondToRideRequest(
                                        ride_id=ride_details['ride_id'],
                                        source=ride_details['pickup_location'],
                                        destination=ride_details['destination_location'],
                                        status=RespondToRideRequest.Status.ACCEPTED
                                    )
                                    await context.write(acceptance_response)
                                    print(f"Driver {driver_name} has accepted the ride of ride_id: {ride_details['ride_id']}")
                                    return

                                elif client_response.request == GetRideRequest.Request.REJECTED:
                                    # if the driver has rejected the ride
                                    await self.handle_ride_rejection(driver_name, ride_details)
                                    print(f"Driver {driver_name} has rejected the ride of ride_id: {ride_details['ride_id']}")
                                    return

                            except asyncio.TimeoutError:
                                print(f"Driver {driver_name} failed to respond in time.")
                                # send them a timeout response, for whenever they respond, handle it as a rejection case, and close the stream.
                                timeout_response = RespondToRideRequest(
                                    ride_id=ride_details['ride_id'],
                                    source=ride_details['pickup_location'],
                                    destination=ride_details['destination_location'],
                                    status=RespondToRideRequest.Status.TIMED_OUT
                                )
                                await context.write(timeout_response)
                                await self.handle_ride_rejection(driver_name, ride_details)
                                return

        finally:
            if driver_name in self.active_subscriptions:
                del self.active_subscriptions[driver_name]
            await pubsub.unsubscribe('RideRequestChannel')
            await pubsub.close()

    async def handle_ride_rejection(self, driver_name, ride_details):
        # mark this driver as available again, since they have rejected the request.
        await self.redis.hset("driver:status", driver_name, "available")

        # add the driver to the list of rejected drivers for this ride id, and increment the ride retry count.
        await self.redis.rpush(f"ride:{ride_details['ride_id']}:rejected_drivers", driver_name)
        retry_count = await self.redis.hincrby(f"ride:{ride_details['ride_id']}", "retry_count", 1)

        # get the list of all available drivers, excluding those who have previously rejected the ride.
        rejected_drivers = await self.redis.lrange(f"ride:{ride_details['ride_id']}:rejected_drivers", 0, -1)
        available_drivers = await self.redis.hgetall('driver:status')
        print(f"Available drivers: {available_drivers}, Drivers who rejected this request thus far: {rejected_drivers}")

        # find a new driver, and send the request to them.
        new_driver = None
        for driver_id, status in available_drivers.items():
            if driver_id not in rejected_drivers and status == "available":
                new_driver = driver_id
                break

        if new_driver:
            # update the ride's details with the new driver, and update this driver's status as being busy (on_ride).
            await self.redis.hset(f"ride:{ride_details['ride_id']}", "current_driver", new_driver)
            await self.redis.hset("driver:status", new_driver, "on_ride")

            # finally, publish the new ride request.
            new_ride_message = {
                "ride_id": ride_details["ride_id"],
                "driver_id": new_driver,
                "pickup_location": ride_details["pickup_location"],
                "destination_location": ride_details["destination_location"]
            }
            await self.redis.publish('RideRequestChannel', json.dumps(new_ride_message))
            print(f"Reassigned ride {ride_details['ride_id']} to driver {new_driver}")

        else:
            # if there are no available drivers, then simply cancel the ride.
            await self.redis.hset(f"ride:{ride_details['ride_id']}", "status", "canceled")
            # cancel_message = {
            #     "ride_id": ride_details["ride_id"],
            #     "status": "canceled"
            # }
            # await self.redis.publish(
            #     f'RiderNotificationChannel:{ride_details["rider_id"]}',
            #     json.dumps(cancel_message)
            # )
            print(f"No available drivers found for ride {ride_details['ride_id']}, canceling request")

    async def CompleteRide(self, request, context):
        print("Ride Completion Request received!")
        driver_id = await self.redis.hget(f"ride:{request.ride_id}", "current_driver")
        await self.redis.hset(f"ride:{request.ride_id}", "status", "completed")

        if driver_id:
            driver_id = driver_id
            await self.redis.hset(f"driver:status", driver_id, "available")

        return RideCompletionResponse(ride_id=request.ride_id)