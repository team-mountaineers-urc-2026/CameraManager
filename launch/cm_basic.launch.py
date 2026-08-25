"""Basic taskless launch"""

from launch import LaunchDescription
from launch_ros.actions import Node


BASIC_CAM_NUMBERS = [34, 35, 37, 39, 40, 41, 43, 44, 67] # Left, Science, Front-Under, Right, Rear
BASIC_CAM_QUALITIES = [2, 2, 2, 2, 2, 2, 2, 2, 2]


def generate_launch_description():
    ld = LaunchDescription()

    # Start node
    ld.add_action(
        Node(
            executable='CameraManager',
            package='camera_manager',
	    parameters=[
                {'cameras_prestart' : BASIC_CAM_NUMBERS},
                {'cam_prestart_qual' : BASIC_CAM_QUALITIES}
            ]

            # arguments=[]
        )
    )

    return ld
