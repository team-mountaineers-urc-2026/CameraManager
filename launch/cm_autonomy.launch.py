"""Basic autonomy launch"""

# XXX In preview phase for testing

from launch import LaunchDescription
from launch_ros.actions import Node

AUTONOMY_CAM_NUMBERS = [36, 38, 40, 41, 44] #44, 38, 41, 36, 40] # Front Mast, Rear Mast, Left Mast, Right Mast
AUTONOMY_CAM_QUALITIES = [2, 2, 2, 2] #, 4, 4, 4, 4]

def generate_launch_description():
    ld = LaunchDescription()

    # Start node
    ld.add_action(
        Node(
            executable='CameraManager',
            package='camera_manager',
            parameters=[
                {'cameras_prestart' : AUTONOMY_CAM_NUMBERS},
                {'cam_prestart_qual' : AUTONOMY_CAM_QUALITIES}
            ],
            #output={'both' : 'log'}, # Mute executable output
        )
    )

    return ld
