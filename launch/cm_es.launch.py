"""Basic launch with realsense"""

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch_ros.actions import Node

ES_CAM_NUMBERS = [-1,12, 33, 37, 38, 39, 40, 44, 67]
ES_CAM_QUALITIES = [2, 2, 2, 2, 2, 2, 2, 2, 2]

def generate_launch_description():
    ld = LaunchDescription()

    # Start node
    ld.add_action(
        Node(
            executable='CameraManager',
            package='camera_manager',
            parameters=[
		{'cameras_prestart' : ES_CAM_NUMBERS},
		{'cam_prestart_qual' : ES_CAM_QUALITIES}
	    ],
	    output={'both'  : 'log'}, #Mute executable output
        )
    )

    # Start realsense rs_launch
    #ld.add_action(
    #    IncludeLaunchDescription(
    #        PythonLaunchDescriptionSource(
    #            PathJoinSubstitution([FindPackageShare('realsense2_camera'), 'launch', 'rs_launch.py'])
    # 	    ),
    #	    launch_arguments={
	#	'depth_module.depth_profile': '424x240x15',
		#'depth_module.infra_profile': '424x240x15',
		#'rgb_camera.color_profile': '424x240x15',
		#'enable_sync': 'true',
		#'initial_reset': 'true'
	    #}.items()
       # )
    #)

    return ld
