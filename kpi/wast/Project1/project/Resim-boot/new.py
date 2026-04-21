

from osi3.osi_sensorview_pb2 import sv
NANO_INCREMENT = 10000000
MOVING_OBJECT_LENGTH = 5
MOVING_OBJECT_WIDTH = 2
MOVING_OBJECT_HEIGHT = 1
#[Task 1]: update the frame separator between each sensorview
separator = “###”
separator_encoded = bytes(separator, 'utf-8')

#[Task 2]: create a file handler with file name Overtaking_from_right.txt for writing the scene to txt file
filehandler = open("Overtaking_from_right.txt", "wb") 

#[Task 3]: Create an sensorview object below
sensorview = sv()

sv_ground_truth = sensorview.global_ground_truth
sv_ground_truth.version.version_major = 3
sv_ground_truth.version.version_minor = 5
sv_ground_truth.version.version_patch = 0
sv_ground_truth.timestamp.seconds = 0
sv_ground_truth.timestamp.nanos = 0

sensorview.global_ground_truth.host_vehicle_id.value = 0

host_moving_object = sv_ground_truth.moving_object.add()
host_moving_object.id.value = 0
host_moving_object.vehicle_classification.type = 2
host_moving_object.base.dimension.length = MOVING_OBJECT_LENGTH
host_moving_object.base.dimension.width = MOVING_OBJECT_WIDTH
host_moving_object.base.dimension.height = MOVING_OBJECT_HEIGHT
host_moving_object.base.position.x = 0.0
host_moving_object.base.position.y = 0.0
host_moving_object.base.position.z = 0.0
host_moving_object.base.orientation.roll = 0.0
host_moving_object.base.orientation.pitch = 0.0
host_moving_object.base.orientation.yaw = 0.0
host_moving_object.type = 2


#[Task 4]: add target moving object

target_moving_object = sv_ground_truth.moving_object.add()
target_moving_object.id.value = 1
target_moving_object.vehicle_classification.type = 2
target_moving_object.base.dimension.length = 3
target_moving_object.base.dimension.width = 1
target_moving_object.base.dimension.height = 1
target_moving_object.base.position.x = 10.0  # Starting position x
target_moving_object.base.position.y = 0.0
target_moving_object.base.position.z = 0.0
target_moving_object.base.orientation.roll = 0.0
target_moving_object.base.orientation.pitch = 0.0
target_moving_object.base.orientation.yaw = 0.0
target_moving_object.type = 2

for i in range(500):
    if sv_ground_truth.timestamp.nanos > 1000000000:
        sv_ground_truth.timestamp.seconds += 1
        sv_ground_truth.timestamp.nanos = 0
    sv_ground_truth.timestamp.nanos += NANO_INCREMENT
	
#[Task 5]: add target positions, dimension and orientation below. 
# Note: that target position has to change every iteration as per the above question
target_moving_object.base.position.x += 0.1  # Moving towards host vehicle


#[Task 6]: Write Sensorview serialized string to file. 
filehandler.write(sensorview.SerializeToString())
if i != 179:
    filehandler.write(separator_encoded)

filehandler.close()
