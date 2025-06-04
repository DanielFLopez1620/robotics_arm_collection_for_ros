import os
import xacro
from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction, IncludeLaunchDescription, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.parameter_descriptions import ParameterValue



def get_argument(context, arg):
    return LaunchConfiguration(arg).perform(context)
 
def generate_robot_description(context):
    # -------------------------- Paths --------------------------------------
    pkg_description = get_package_share_directory('eezybot_mk1_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'mk1.urdf.xacro')
 
 
    mappings = {}
 
    robot_description_config = xacro.process_file(xacro_file, mappings=mappings)
    robot_desc = robot_description_config.toprettyxml(indent='  ')

    robot_desc = robot_desc.replace(
        'package://eezybot_mk1_description/', f'file://{pkg_description}/'
    )

 
    # -------------------------- Nodes ----------------------------------------
    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name="robot_state_publisher",
        output='screen',
        parameters=[{
            'robot_description': robot_desc,
            'rate': 200,
        }]
    )

   
 
    
   
    return [
        rsp_node
    ]

def generate_launch_description():
 
    gazebo_arg = DeclareLaunchArgument(
        'gazebo',
        default_value='true',
        description="True for using gazebo tags, false otherwise"
    )
    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(
                get_package_share_directory("ros_gz_sim"),
                "launch",
                "gz_sim.launch.py"
            )
        ]),
        launch_arguments={"gz_args": "-v 4 -r empty.sdf"}.items()
    )
    spawn_robot = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=["-topic", "robot_description", "-name", "eezybot_mk1"]
    )

    ros_gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock"
        ]
    )
    
 
    return LaunchDescription([
        gazebo_arg,
        OpaqueFunction(function=generate_robot_description),
        gazebo_launch,
        spawn_robot,
        ros_gz_bridge
    ])
