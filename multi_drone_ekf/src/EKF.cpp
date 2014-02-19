/*
 * EKF.h
 *
 *  Created on: Feb 8, 2014
 *      Author: Karol Hausman
 */

#include "multi_drone_ekf/EKF.h"


void ExtendedKalmanFilter::computeHJacobian(const tf::Transform& cam_to_world_flat, const tf::Transform& drone_to_marker_flat, Eigen::Matrix3f& dh)
{

    double c11,c12,/*c13,*/c21,c22,/*c23,*/m11,/*m12,*/m13,m21,/*m22,*/m23;
    tf::Matrix3x3 c_rot = cam_to_world_flat.getBasis();
//    tf::Vector3 c_transl = cam_to_world_flat.getOrigin();

    c11 = c_rot.getRow(0).getX(); c12 = c_rot.getRow(0).getY(); //c13 = c_transl.getX();
    c21 = c_rot.getRow(1).getX(); c22 = c_rot.getRow(1).getY(); //c23 = c_transl.getY();


    tf::Matrix3x3 m_rot = drone_to_marker_flat.getBasis();
    tf::Vector3 m_transl = drone_to_marker_flat.getOrigin();
    m11 = m_rot.getRow(0).getX();/* m12 = m_rot.getRow(0).getY();*/ m13 = m_transl.getX();
    m21 = m_rot.getRow(1).getX();/* m22 = m_rot.getRow(1).getY();*/ m23 = m_transl.getY();



    dh << c11,c12,m13*(c12*cos(state_(2)) - c11*sin(state_(2))) +
             m23*(-(c11*cos(state_(2))) - c12*sin(state_(2))),
           c21,c22,m13*(c22*cos(state_(2)) - c21*sin(state_(2))) +
             m23*(-(c21*cos(state_(2))) - c22*sin(state_(2))),
           0,0,((m11*(c22*cos(state_(2)) - c21*sin(state_(2))) +
                  m21*(-(c21*cos(state_(2))) - c22*sin(state_(2))))/
                (m21*(c12*cos(state_(2)) - c11*sin(state_(2))) + m11*(c11*cos(state_(2)) + c12*sin(state_(2)))) -
               ((m11*(c12*cos(state_(2)) - c11*sin(state_(2))) + m21*(-(c11*cos(state_(2))) - c12*sin(state_(2))))*
                  (m21*(c22*cos(state_(2)) - c21*sin(state_(2))) + m11*(c21*cos(state_(2)) + c22*sin(state_(2)))))/
                pow(m21*(c12*cos(state_(2)) - c11*sin(state_(2))) + m11*(c11*cos(state_(2)) + c12*sin(state_(2))),
                 2))/(1 + pow(m21*(c22*cos(state_(2)) - c21*sin(state_(2))) +
                  m11*(c21*cos(state_(2)) + c22*sin(state_(2))),2)/
                pow(m21*(c12*cos(state_(2)) - c11*sin(state_(2))) + m11*(c11*cos(state_(2)) + c12*sin(state_(2))),
                 2));

}


void ExtendedKalmanFilter::init(const tf::Transform& world_to_drone_pose)
{
    state_(0) = world_to_drone_pose.getOrigin().getX();
    state_(1) = world_to_drone_pose.getOrigin().getY();
    double yaw = 0;
    double pitch = 0;
    double roll = 0;
    world_to_drone_pose.getBasis().getEulerYPR(yaw, pitch, roll);
    state_(2) = yaw;
    initialized_ = true;

}



// odometry:
// x: distance travelled in local x-direction
// y: distance travelled in local y-direction
// yaw: rotation update
void ExtendedKalmanFilter::predictionStep(const Eigen::Vector3f& odometry) {




    state_(0) = state_(0) + cos(state_(2)) * odometry(0)
            - sin(state_(2)) * odometry(1);
    state_(1) = state_(1) + sin(state_(2)) * odometry(0)
            + cos(state_(2)) * odometry(1);
    state_(2) = state_(2) + odometry(2);

    state_(2) = atan2(sin(state_(2)), cos(state_(2))); // normalize angle

    // dg/dx:
    Eigen::Matrix3f G;


    G << 1, 0, -sin(state_(2)) * odometry(0) - cos(state_(2)) * odometry(1), 0, 1, cos(
            state_(2)) * odometry(0) - sin(state_(2)) * odometry(1), 0, 0, 1;


    sigma_ = G * sigma_ * G.transpose() + Q_;

}

void ExtendedKalmanFilter::correctionStep(const Eigen::Vector6f& measurement, const tf::Transform& cam_to_world_transform, const tf::Transform& drone_to_marker_transform) { // compare expected and measured values, update state and uncertainty

    Eigen::Vector3f h;



    tf::Transform state_pose_flat;
    btVector3 state_origin_flat(state_(0),state_(1),0);
    state_pose_flat.setOrigin(state_origin_flat);
    btQuaternion state_quaternion_flat;
    state_quaternion_flat.setEulerZYX(state_(2),0,0);
    state_pose_flat.setRotation(state_quaternion_flat);


    tf::Transform drone_to_marker_flat = drone_to_marker_transform;
    double d_yaw = 0;
    double d_pitch = 0;
    double d_roll = 0;
    drone_to_marker_flat.getBasis().getEulerYPR(d_yaw, d_pitch, d_roll);
    btVector3 d2m_origin_flat(drone_to_marker_flat.getOrigin().getX(),drone_to_marker_flat.getOrigin().getY(),0);
    drone_to_marker_flat.setOrigin(d2m_origin_flat);
    btQuaternion d2m_quaternion_flat;
    d2m_quaternion_flat.setEulerZYX(d_yaw,0,0);
    drone_to_marker_flat.setRotation(d2m_quaternion_flat);




    tf::Transform cam_to_world_flat;
    double c_yaw = 0;
    double c_pitch = 0;
    double c_roll = 0;
    cam_to_world_transform.inverse().getBasis().getEulerYPR(c_yaw,c_pitch,c_roll);
    btVector3 c2w_origin_flat(cam_to_world_transform.inverse().getOrigin().getX(),cam_to_world_transform.inverse().getOrigin().getY(),0);
    cam_to_world_flat.setOrigin(c2w_origin_flat);
    btQuaternion c2w_quaternion_flat;
    c2w_quaternion_flat.setEulerZYX(c_yaw,0,0);
    cam_to_world_flat.setRotation(c2w_quaternion_flat);

    cam_to_world_flat = cam_to_world_flat.inverse();
    Eigen::Vector3f debug_cam;
    debug_cam << cam_to_world_flat.getOrigin().getX(),cam_to_world_flat.getOrigin().getY(),tf::getYaw(cam_to_world_flat.getRotation());





    tf::Transform h_transform_flat;

    h_transform_flat = cam_to_world_flat*state_pose_flat*drone_to_marker_flat;

    h << h_transform_flat.getOrigin().getX(), h_transform_flat.getOrigin().getY(), tf::getYaw(h_transform_flat.getRotation());

    Eigen::Matrix3f dh;



    computeHJacobian(cam_to_world_flat, drone_to_marker_flat, dh);



    Eigen::Matrix3f K;


    Eigen::Matrix3f brackets = dh * sigma_ * dh.transpose() + R_;

    K = sigma_ * dh.transpose() * brackets.inverse();

    sigma_ = (Eigen::Matrix3f::Identity() - K * dh) * sigma_;

    Eigen::Vector3f measurement_3dog = Eigen::Vector3f::Zero();


    reduceMeasurementDimensions(measurement, cam_to_world_transform.inverse(), drone_to_marker_transform.inverse(), measurement_3dog);

    Eigen::Vector3f brackets2 = measurement_3dog - h;
    //normalize yaw angle
    brackets2(2) = atan2(sin(brackets2(2)), cos(brackets2(2)));




    state_ = state_ + K * brackets2;


}

void ExtendedKalmanFilter::reduceMeasurementDimensions (const Eigen::Vector6f& measurement, const tf::Transform& world_to_cam, const tf::Transform& marker_to_drone, Eigen::Vector3f& measurement_3dog)
{
    double c_yaw = 0;
    double c_pitch = 0;
    double c_roll = 0;

    world_to_cam.getBasis().getEulerYPR(c_yaw,c_pitch,c_roll);


    double c_x =0;double c_y=0; //double c_z=0;

    c_x = world_to_cam.getOrigin().getX();
    c_y = world_to_cam.getOrigin().getY();
//    c_z = world_to_cam.getOrigin().getZ();


    tf::Transform cam_to_marker;
    tf::Vector3 origin(measurement(0), measurement(1), measurement(2));
    cam_to_marker.setOrigin(origin);
    tf::Quaternion rotation;
    rotation.setRPY(measurement(3),measurement(4), measurement(5));
    cam_to_marker.setRotation(rotation);

    tf::Transform world_to_marker = world_to_cam * cam_to_marker;

    double m_yaw = 0;
    double m_pitch = 0;
    double m_roll = 0;

    world_to_marker.getBasis().getEulerYPR(m_yaw,m_pitch,m_roll);

    double m_x =0;double m_y=0; //double m_z=0;

    m_x = world_to_marker.getOrigin().getX();
    m_y = world_to_marker.getOrigin().getY();
//    m_z = world_to_marker.getOrigin().getZ();


    tf::Transform c_3d,z_3d,m_3d;

    tf::Vector3 c_3d_origin(c_x,c_y,0);
    tf::Quaternion c_3d_rotation;
    c_3d_rotation.setRPY(0,0,c_yaw);
    c_3d.setOrigin(c_3d_origin);
    c_3d.setRotation(c_3d_rotation);



    tf::Vector3 m_3d_origin(m_x,m_y,0);
    tf::Quaternion m_3d_rotation;
    m_3d_rotation.setRPY(0,0,m_yaw);
    m_3d.setOrigin(m_3d_origin);
    m_3d.setRotation(m_3d_rotation);


    z_3d = c_3d.inverse()*m_3d;


    measurement_3dog(0) = z_3d.getOrigin().getX();
    measurement_3dog(1) = z_3d.getOrigin().getY();
    measurement_3dog(2) = tf::getYaw(z_3d.getRotation());

}



