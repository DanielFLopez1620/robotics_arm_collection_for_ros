import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    # ------------------------ Paths definitions ----------------------------
    pkg_sim = get_package_share_directory('eezybot_mk2_gz')
    spawn_file = os.path.join(pkg_sim, 'launch', 'spawn_robot.launch.py')


    # --------------------------- Configurations -----------------------------

    world = LaunchConfiguration('world')
    x = LaunchConfiguration('x')
    y = LaunchConfiguration('y')
    z = LaunchConfiguration('z')
    roll = LaunchConfiguration('R')
    pitch = LaunchConfiguration('P')
    yaw = LaunchConfiguration('Y')

    # -------------------------- Launch arguments -----------------------------
    world_arg = DeclareLaunchArgument(
        'world',
        default_value='empty.sdf',
        description='Specify the world file for Gazebo'
    )

    x_arg = DeclareLaunchArgument(
        'x', 
        default_value='0.0', 
        description='Initial X position'
    )

    y_arg = DeclareLaunchArgument(
        'y',
        default_value='0.0',
        description='Initial Y position'
    )

    z_arg = DeclareLaunchArgument(
        'z',
        default_value='0.5',
        description='Initial Z position'
    )

    roll_arg = DeclareLaunchArgument(
        'R',
        default_value='0.0',
        description='Initial Roll'
    )

    pitch_arg = DeclareLaunchArgument(
        'P',
        default_value='0.0',
        description='Initial Pitch'
    )

    yaw_arg = DeclareLaunchArgument(
        'Y',
        default_value='0.0',
        description='Initial Yaw'
    )

    # -------------------------- Includes --------------------------------------
    spawn_include = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(spawn_file),
        launch_arguments={
            'x': x,
            'y': y,
            'z': z,
            'roll': roll,
            'pitch': pitch,
            'yaw': yaw,
            'world': world,
        }.items()
    )

    # ------------------------ Nodes --------------------------------------------
    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            # Clock (IGN -> ROS2)
            '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
            # Joint states (IGN -> ROS2)
            '/world/empty/model/mk2/joint_state@sensor_msgs/msg/JointState[gz.msgs.Model',
        ],
        remappings=[
            ('/world/empty/model/mk2/joint_state', 'joint_states'),
        ],
        output='screen'
    )

    return LaunchDescription([
        world_arg,
        x_arg,
        y_arg,
        z_arg,
        roll_arg,
        pitch_arg,
        yaw_arg,
        spawn_include,
        bridge
    ])