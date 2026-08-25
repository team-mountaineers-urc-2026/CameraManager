"""Basic autonomy launch"""

# XXX In preview phase for testing

from launch import LaunchDescription
from launch_ros.actions import Node

CAM_NUMBERS = [38] # temp

def generate_launch_description():
    ld = LaunchDescription()

    # Start node
    ld.add_action(
        Node(
            executable='CameraManager',
            package='camera_manager',
            parameters=[
                {'cameras_prestart' : CAM_NUMBERS},
                {'cam_prestart_qual' : [5]}
            ],
            remappings=[('/image_topic','/image_raw')],
            output={'both' : 'log'}, # Mute executable output
        )
    )

    return ld
