#include <Arduino.h>

#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int64_multi_array.h>
#include <std_msgs/msg/multi_array_dimension.h>

#include "servo.hpp"

#include <cmath>

void cmd_servo_callback(const void *msgin);
void timer_servo_callback(rcl_timer_t * timer, int64_t last_call_tm);
void error_loop();

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}


rcl_publisher_t servo_pub;
rcl_subscription_t servo_sub;

std_msgs__msg__Int64MultiArray servo_msg_cmd;
std_msgs__msg__Int64MultiArray servo_msg_feedback;

fwd::ServoMotor servo_1(0, 180, 16);
fwd::ServoMotor servo_2(0, 180, 17);
fwd::ServoMotor servo_3(0, 180, 18);
fwd::ServoMotor servo_g(0, 180, 19);

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

void setup()
{
    Serial.begin(155200);
    set_microros_serial_transports(Serial);

    delay(2000);

    allocator = rcl_get_default_allocator();

    unsigned long start = millis();
    rcl_ret_t ret;
    do
    {
        ret = rclc_support_init(&support, 0, NULL, &allocator);
        if (ret != RCL_RET_OK)
        {
            delay(500);
        }
    } while (ret != RCL_RET_OK && (millis() - start < 120000));
    
    RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_mk1_ctl", "", &support));

    RCCHECK(rclc_publisher_init_default(
        &servo_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64MultiArray),
        "mk1_servos_feedback"
    ));

    RCCHECK(rclc_subscription_init_default(
        &servo_sub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int64MultiArray),
        "mk1_servos_cmd"
    ));

    const unsigned int timeout = 250;

    RCCHECK(rclc_timer_init_default(
        &timer,
        &support,
        RCL_MS_TO_NS(timeout),
        timer_servo_callback
    ));

    servo_msg_cmd.data.capacity = 2;
    servo_msg_cmd.data.size = 0;
    servo_msg_cmd.data.data = (int64_t*) malloc(servo_msg_cmd.data.capacity * sizeof(int64_t));

    servo_msg_cmd.layout.dim.capacity = 2;
    servo_msg_cmd.layout.dim.size = 0;
    servo_msg_cmd.layout.dim.data = (std_msgs__msg__MultiArrayDimension*) malloc(
        servo_msg_cmd.layout.dim.capacity * sizeof(std_msgs__msg__MultiArrayDimension));

    for(size_t i = 0; i < servo_msg_cmd.layout.dim.capacity; ++i)
    {
        servo_msg_cmd.layout.dim.data[i].label.capacity = 2;
        servo_msg_cmd.layout.dim.data[i].label.size = 0;
        servo_msg_cmd.layout.dim.data[i].label.data = (char*) malloc(
            servo_msg_cmd.layout.dim.data[i].label.capacity * sizeof(char));
    }

    servo_msg_feedback.data.capacity = 2;
    servo_msg_feedback.data.size = 0;
    servo_msg_feedback.data.data = (int64_t*) malloc(servo_msg_feedback.data.capacity * sizeof(int64_t));

    servo_msg_feedback.layout.dim.capacity = 2;
    servo_msg_feedback.layout.dim.size = 0;
    servo_msg_feedback.layout.dim.data = (std_msgs__msg__MultiArrayDimension*) malloc(
        servo_msg_feedback.layout.dim.capacity * sizeof(std_msgs__msg__MultiArrayDimension));

    for(size_t i = 0; i < servo_msg_feedback.layout.dim.capacity; ++i)
    {
        servo_msg_feedback.layout.dim.data[i].label.capacity = 2;
        servo_msg_feedback.layout.dim.data[i].label.size = 0;
        servo_msg_feedback.layout.dim.data[i].label.data = (char*) malloc(
            servo_msg_feedback.layout.dim.data[i].label.capacity * sizeof(char));
    }

    RCCHECK(rclc_executor_init(&executor, &support.context, 5, &allocator));

    RCCHECK(rclc_executor_add_timer(&executor, &timer));

    RCCHECK(rclc_executor_add_subscription(
        &executor,
        &servo_sub,
        &servo_msg_cmd,
        &cmd_servo_callback,
        ON_NEW_DATA
    ))

    servo_1.begin();
    servo_2.begin();
    servo_3.begin();
    servo_g.begin();
}

void loop()
{
    delay(100);
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

void cmd_servo_callback(const void *msgin)
{
    const std_msgs__msg__Int64MultiArray *msg = (const std_msgs__msg__Int64MultiArray *) msgin;

    if(servo_1.getPositionRad() - M_PI_2 != msg->data.data[0])
    {
        servo_1.setPositionRad(msg->data.data[0] + M_PI_2);
    }
    if(servo_2.getPositionRad() - M_PI_2 != msg->data.data[1])
    {
        servo_2.setPositionRad(msg->data.data[1] + M_PI_2);
    }
    if(servo_3.getPositionRad() - M_PI_2 != msg->data.data[2])
    {
        servo_3.setPositionRad(msg->data.data[2] + M_PI_2);
    }
    if(servo_g.getPositionRad() - M_PI_2 != msg->data.data[3])
    {
        servo_g.setPositionRad(msg->data.data[3] + M_PI_2);
    }

}

void timer_servo_callback(rcl_timer_t * timer, int64_t last_call_tm)
{
    servo_msg_cmd.data.data[0] = servo_1.getPositionRad() - M_PI_2;
    servo_msg_cmd.data.data[1] = servo_2.getPositionRad() - M_PI_2;
    servo_msg_cmd.data.data[2] = servo_3.getPositionRad() - M_PI_2;
    servo_msg_cmd.data.data[3] = servo_g.getPositionRad() - M_PI_2;
    
    RCSOFTCHECK(rcl_publish(&servo_pub, (const void*)&servo_msg_cmd, NULL));
}

void error_loop()
{
    while(1)
    {
        delay(100);
    }
}