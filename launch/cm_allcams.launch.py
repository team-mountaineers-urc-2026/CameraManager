"""Basic autonomy launch"""

# XXX In preview phase for testing

from launch import LaunchDescription
from launch_ros.actions import Node

CAM_NUMBERS = [44, 36, 38, 41, 40, 39, 37, 45, 34, 35, 43] # Front Mast, Rear Mast, Left Mast, Right Mast
CAM_QUALITIES = [8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8]

def generate_launch_description():
    ld = LaunchDescription()

    # Start node
    ld.add_action(
        Node(
            executable='CameraManager',
            package='camera_manager',
            parameters=[
                {'cameras_prestart' : CAM_NUMBERS},
                {'cam_prestart_qual' : CAM_QUALITIES}
            ],
            #output={'both' : 'log'}, # Mute executable output
        )
    )

    return ld
