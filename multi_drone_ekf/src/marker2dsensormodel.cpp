#include "multi_drone_ekf/marker2dsensormodel.h"
#include <Eigen/Cholesky>
#include <iostream>
#include "multi_drone_ekf/random.h"
#include <ros/ros.h>
#include <tf/transform_broadcaster.h>

namespace ranav {

Marker2dSensorModel::Marker2dSensorModel()
{
    noiseCov = Eigen::MatrixXd::Identity(3,3);
    noiseCovSqrt = Eigen::MatrixXd::Identity(3,3);
    stateDim = 3;
    measurementDim = 3;
    noiseDim = 2;
    H = Eigen::MatrixXd::Zero(3, 3);
    W = Eigen::MatrixXd::Identity(3, 3);
    visibilityRadius = 0.2;
    measurementNoise = 0.01;
    distanceNoiseFactor = 0.2;

}

Marker2dSensorModel::~Marker2dSensorModel() {
}

//void Marker2dSensorModel::init(const TParam &p) {
//  params = p;
//  nA = params("multi_rotor_control/numAgents").toInt();
//  nT = params("multi_rotor_control/numTargets").toInt();
//  stateDim = 2*nA+4;
//  measurementDim = 2;
//  noiseDim = 2;
//  H = Eigen::MatrixXd::Zero(measurementDim, stateDim);
//  if (toId >= 0)
//    H.block(0, 2*toId, 2, 2) = Eigen::MatrixXd::Identity(2, 2);
//  if (fromId >= 0)
//    H.block(0, 2*fromId, 2, 2) = -Eigen::MatrixXd::Identity(2, 2);
//  W = Eigen::MatrixXd::Identity(measurementDim, noiseDim);
//  measurementNoise = p("multi_rotor_control/measurementStdDev").toDouble();
//  measurementNoise = measurementNoise*measurementNoise;
//  distanceNoiseFactor = p("multi_rotor_control/distanceStdDevFactor").toDouble();
//  distanceNoiseFactor = distanceNoiseFactor*distanceNoiseFactor;
//  visibilityRadius = p("multi_rotor_control/visibilityRadius").toDouble();
//  noiseCov = Eigen::MatrixXd::Identity(noiseDim, noiseDim)*measurementNoise;
//  noiseCovSqrt = noiseCov.llt().matrixL();
//}

bool Marker2dSensorModel::measurementAvailable(const Eigen::VectorXd &state) const {
    return true;
}

Eigen::VectorXd Marker2dSensorModel::sense(const Eigen::VectorXd &state, /*const tf::Transform& cam_to_world_flat, const tf::Transform& drone_to_marker_flat,*/ const Eigen::VectorXd &noise) const {

    tf::Transform state_pose_flat;
    btVector3 state_origin_flat(state(0),state(1),0);
    state_pose_flat.setOrigin(state_origin_flat);
    btQuaternion state_quaternion_flat;
    state_quaternion_flat.setEulerZYX(state(2),0,0);
    state_pose_flat.setRotation(state_quaternion_flat);



    tf::Transform h_transform_flat;

    h_transform_flat = /*cam_to_world_flat**/state_pose_flat/**drone_to_marker_flat*/;

    Eigen::VectorXd h(3);
    h << h_transform_flat.getOrigin().getX(), h_transform_flat.getOrigin().getY(), tf::getYaw(h_transform_flat.getRotation());

    return h;



}

Eigen::VectorXd Marker2dSensorModel::sampleNoise(const Eigen::VectorXd &state, const Eigen::VectorXd &measurement) const {
  return Random::multivariateGaussian(getNoiseCov(state, measurement));
}

Eigen::MatrixXd Marker2dSensorModel::getNoiseCov(const Eigen::VectorXd &state, const Eigen::VectorXd &measurement) const {
//  if (fromId < 0)
//    return noiseCov;
//  double dist = measurement.norm();
//  return noiseCov + Eigen::MatrixXd::Identity(noiseDim, noiseDim) * measurementNoise * distanceNoiseFactor * pow(dist, 4);
}

double Marker2dSensorModel::getInformation(const Eigen::VectorXd &state, const Eigen::VectorXd &measurement) const {
//  if (!measurementAvailable(state))
    return 0;
//  double dist = measurement.norm();
//  double noise = measurementNoise + measurementNoise * distanceNoiseFactor * pow(dist, 4);
//  return 1/sqrt(noise);
}

Eigen::MatrixXd Marker2dSensorModel::jacobianState(const Eigen::VectorXd &state, const Eigen::VectorXd &) const {
  return H;
}

Eigen::MatrixXd Marker2dSensorModel::jacobianNoise(const Eigen::VectorXd &, const Eigen::VectorXd &) const {
  return W;
}

} /* namespace ranav */

