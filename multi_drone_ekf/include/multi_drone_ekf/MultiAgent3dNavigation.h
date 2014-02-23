#ifndef MULTIAGENT3DNAVIGATION_H_
#define MULTIAGENT3DNAVIGATION_H_

#include <ranav/tparam.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_broadcaster.h>



namespace ranav {
  class MultiAgentMotionModel;
  class EKF;
  class TargetTrackingController;
  class SensorModel;
}

class MultiAgent3dNavigation {
public:
  //! fill TargetTrackingController::Topology::allModels with appropriate Marker3dSensorModels
  MultiAgent3dNavigation(const tf::Transform& world_to_cam, const tf::Transform& drone_to_marker, const tf::Transform& drone_to_front_cam, const ranav::TParam &p);
  ~MultiAgent3dNavigation();

  double getCycleDt() { return params("estimation/motionDt").toDouble(); }

  struct Measurement3D {
    int fromId, toId; //!< agent ID (-1: global GPS; 0, ..., N-1: agents; N: target)
    tf::Transform measurement;
  };
  struct Odometry3D {
    int id; //!< agent ID (0, ..., N-1)
    Eigen::Vector3d movement2d; //!< incremental movement
    double roll, pitch, z; //!< current values
  };

  //! downproject all to 2D
  //! apply odometry in EKF
  //! check which measurements have to be used according to current topology and apply them in EKF
  void navigate(const std::vector<Measurement3D> &measurements, const std::vector<Odometry3D> &odometry, std::vector<geometry_msgs::Twist> &control, std::vector<tf::Transform> &stateEstimate);

  static const bool broadcast_3d_measurements = true;
  static const bool broadcast_projected_measurements = true;

protected:
  struct AddOn3d {
      AddOn3d() : roll(0), pitch(0), z(0) {}
    AddOn3d(double r, double p, double z) : roll(r), pitch(p), z(z) {}
    double roll, pitch, z;
  };
  std::vector<AddOn3d> addOn3d; // for every agent
  void getStateEstimate(std::vector<tf::Transform> &stateEstimate);

  // 3D navigation stuff
  tf::Transform world_to_cam;
  tf::Transform drone_to_marker;
  tf::Transform drone_to_front_cam;
  tf::Transform world_to_cam_2d;
  tf::Transform drone_to_front_cam_2d;

  // 2D navigation stuff
  ranav::TParam params;
  ranav::MultiAgentMotionModel *motionModel;
  ranav::EKF *ekf;
  ranav::TargetTrackingController *ttc;
  std::vector<ranav::SensorModel*> sensorModels;

  //transform broadcaster for 2d projections
  tf::TransformBroadcaster transform_broadcaster;

};

#endif
