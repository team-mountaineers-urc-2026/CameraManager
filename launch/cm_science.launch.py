"""Basic science launch"""

# XXX In preview phase for testing

from launch import LaunchDescription
from launch_ros.actions import Node

SCIENCE_CAM_NUMBERS = [33, 35, 37, 39, 40, 67]
SCIENCE_CAM_QUALITIES = [2, 2, 2, 2, 2, 2]


def generate_launch_description():
    ld = LaunchDescription()

    # Start node
    ld.add_action(
        Node(
            executable='CameraManager',
            package='camera_manager',
            parameters=[
                {'cameras_prestart' : SCIENCE_CAM_NUMBERS},
                {'cam_prestart_qual' : SCIENCE_CAM_QUALITIES}
            ],
            output={'both' : 'log'}, # Mute executable output
        )
    )

    return ld
