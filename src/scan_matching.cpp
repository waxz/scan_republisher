//
// Created by waxz on 22-11-14.
//

#include <thread>
#include <iostream>


#include "ros/ros.h"
#include "ros/callback_queue.h"
#include "nav_msgs/OccupancyGrid.h"
#include "visualization_msgs/MarkerArray.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "geometry_msgs/PoseArray.h"
#include "sensor_msgs/PointCloud2.h"


#include <geometry_msgs/Pose.h>
#include <interactive_markers/interactive_marker_server.h>
#include <ros/ros.h>
#include <string>
#include <tf/transform_broadcaster.h>
#include <visualization_msgs/InteractiveMarker.h>


#include <tf2_ros/transform_listener.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>

#include "sensor_msgs/LaserScan.h"
#include "xmlrpcpp/XmlRpc.h"


#include "common/task.h"
#include "sensor/laser_scan.h"

#include <pcl/point_types.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/filters/approximate_voxel_grid.h>
#include <pcl/filters/passthrough.h>

#include <pcl/filters/voxel_grid.h>


#include <pcl/registration/icp.h>
#include <pcl/registration/icp_nl.h>
#include <pcl/registration/transformation_estimation_lm.h>
#include <pcl/registration/warp_point_rigid_3d.h>
#include <pcl/registration/transformation_estimation_2D.h>

#include <pcl/registration/transformation_estimation_point_to_plane.h>
#include <pcl/registration/transformation_estimation_point_to_plane_lls.h>
#include <pcl/registration/transformation_estimation_point_to_plane_weighted.h>
#include <pcl/registration/transformation_estimation_point_to_plane_lls_weighted.h>


#include <pcl/octree/octree.h>
#include <pcl/octree/octree_impl.h>
#include <pcl/octree/octree_pointcloud_adjacency.h>
#include <pcl/octree/octree_pointcloud_occupancy.h>
#include <pcl/common/time.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

#include <pcl/io/pcd_io.h>

#include <pcl/octree/octree.h>
#include <pcl/octree/octree_impl.h>
#include <pcl/octree/octree_pointcloud_adjacency.h>
#include <fstream>


#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
#if __has_include(<filesystem>)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs = std::filesystem;
#endif
#endif
#ifndef GHC_USE_STD_FS
//#include "filesystem.hpp"
#include "ghc/filesystem.hpp"

namespace fs = ghc::filesystem;
#endif


#include "icp/Normal2dEstimation.h"


#include "nlohmann/json.hpp"



namespace serialization {
    bool to_json(nlohmann::json &data, const std::string &name, const transform::Transform2d &object);

    bool from_json(nlohmann::json &data, const std::string &name, transform::Transform2d &object);
}

#include "message/serialization_json.h"

namespace serialization {
    constexpr auto Transform2d_properties = std::make_tuple(
            property(&transform::Transform2d::matrix, "matrix")
    );
}

namespace serialization {
    bool to_json(nlohmann::json &data, const std::string &name, const transform::Transform2d &object) {
//     constexpr  auto properties = house_properties;
        return to_json_unpack(data, name, object, Transform2d_properties);
    }

    bool from_json(nlohmann::json &data, const std::string &name, transform::Transform2d &object) {

        return from_json_unpack(data, name, object, Transform2d_properties);

    }
}



void toEulerAngle(const float x, const float y, const float z, const float w, float &roll, float &pitch, float &yaw) {
// roll (x-axis rotation)
    float sinr_cosp = +2.0 * (w * x + y * z);
    float cosr_cosp = +1.0 - 2.0 * (x * x + y * y);
    roll = atan2(sinr_cosp, cosr_cosp);

// pitch (y-axis rotation)
    float sinp = +2.0 * (w * y - z * x);
    if (fabs(sinp) >= 1)
        pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

// yaw (z-axis rotation)
    float siny_cosp = +2.0 * (w * z + x * y);
    float cosy_cosp = +1.0 - 2.0 * (y * y + z * z);
    yaw = atan2(siny_cosp, cosy_cosp);
//    return yaw;
}

void to_yaw(const float x, const float y, const float z, const float w, float &yaw) {
// roll (x-axis rotation)
    float sinr_cosp = +2.0 * (w * x + y * z);
    float cosr_cosp = +1.0 - 2.0 * (x * x + y * y);

// pitch (y-axis rotation)
    float sinp = +2.0 * (w * y - z * x);


// yaw (z-axis rotation)
    float siny_cosp = +2.0 * (w * z + x * y);
    float cosy_cosp = +1.0 - 2.0 * (y * y + z * z);
    yaw = atan2(siny_cosp, cosy_cosp);
//    return yaw;
}


void yaw_to_quaternion(float yaw, double &qx, double &qy, double &qz, double &qw) {
    float roll = 0.0;
    float pitch = 0.0;

    qx = 0.0;
    qy = 0.0;
    qz = sin(yaw * 0.5f);
    qw = cos(yaw * 0.5f);

    qx = sin(roll / 2) * cos(pitch / 2) * cos(yaw / 2) - cos(roll / 2) * sin(pitch / 2) * sin(yaw / 2);
    qy = cos(roll / 2) * sin(pitch / 2) * cos(yaw / 2) + sin(roll / 2) * cos(pitch / 2) * sin(yaw / 2);
    qz = cos(roll / 2) * cos(pitch / 2) * sin(yaw / 2) - sin(roll / 2) * sin(pitch / 2) * cos(yaw / 2);
    qw = cos(roll / 2) * cos(pitch / 2) * cos(yaw / 2) + sin(roll / 2) * sin(pitch / 2) * sin(yaw / 2);

}

namespace ros_tool {
    class InteractiveTf {
        ros::NodeHandle nh_;
        boost::shared_ptr<interactive_markers::InteractiveMarkerServer> server_;

        void processFeedback(unsigned ind, const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback);

        visualization_msgs::InteractiveMarker int_marker_;

        geometry_msgs::Pose pose_;
        tf::TransformBroadcaster br_;
        std::string parent_frame_;
        std::string frame_;

        void updateTf(int, const ros::TimerEvent &event);

        ros::Timer tf_timer_;

        std::function<void(const geometry_msgs::Pose &)> m_callback;
        bool is_start = false;

    public:
        InteractiveTf(const std::string &t_parent_frame, const std::string &t_target_frame);

        ~InteractiveTf();

        void start(float scale = 1.0, bool pub_tf = false);

        void setInitPose(float x, float y, float yaw);

        geometry_msgs::Pose &getPose();

        void setCallBack(std::function<void(const geometry_msgs::Pose &)> &&);
    };


    void InteractiveTf::setCallBack(std::function<void(const geometry_msgs::Pose &)> &&t_cb) {
        m_callback = std::move(t_cb);
    }

    void InteractiveTf::setInitPose(float x, float y, float yaw) {
        pose_.position.x = x;
        pose_.position.y = y;
        pose_.position.z = 1.0;

        yaw_to_quaternion(yaw, pose_.orientation.x, pose_.orientation.y, pose_.orientation.z, pose_.orientation.w);

    }

    geometry_msgs::Pose &InteractiveTf::getPose() {
        return pose_;
    }

    InteractiveTf::InteractiveTf(const std::string &t_parent_frame, const std::string &t_target_frame) :
            parent_frame_(t_parent_frame),
            frame_(t_target_frame) {}

    void InteractiveTf::start(float scale, bool pub_tf) {

        if(is_start){
            std::cout << "InteractiveTf is already running" << std::endl;

            return;
        }

        server_.reset(new interactive_markers::InteractiveMarkerServer("interactive_tf"));

        // TODO(lucasw) need way to get parameters out- tf echo would work

        int_marker_.header.frame_id = parent_frame_;
        // http://answers.ros.org/question/262866/interactive-marker-attached-to-a-moving-frame/
        // putting a timestamp on the marker makes it not appear
        // int_marker_.header.stamp = ros::Time::now();
        int_marker_.name = "interactive_tf";
        int_marker_.description = "control a tf with 6dof";
        int_marker_.pose = pose_;
        int_marker_.scale = 1.0;

        {
            visualization_msgs::InteractiveMarkerControl control;

            // TODO(lucasw) get roll pitch yaw and set as defaults

            control.orientation.w = 1;
            control.orientation.x = 1;
            control.orientation.y = 0;
            control.orientation.z = 0;
            control.name = "rotate_x";
            control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_AXIS;
//            int_marker_.controls.push_back(control);
            control.name = "move_x";
            // TODO(lucasw) how to set initial values?
            // double x = 0.0;
            // ros::param::get("~x", x);
            // control.pose.position.x = x;
            control.interaction_mode = visualization_msgs::InteractiveMarkerControl::MOVE_AXIS;
            int_marker_.controls.push_back(control);
            // control.pose.position.x = 0.0;

            control.orientation.w = 1;
            control.orientation.x = 0;
            control.orientation.y = 0;
            control.orientation.z = 1;
            control.name = "rotate_y";
            control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_AXIS;
//            int_marker_.controls.push_back(control);
            control.name = "move_y";
            // double y = 0.0;
            // control.pose.position.z = ros::param::get("~y", y);
            control.interaction_mode = visualization_msgs::InteractiveMarkerControl::MOVE_AXIS;
            int_marker_.controls.push_back(control);
            // control.pose.position.y = 0.0;

            control.orientation.w = 1;
            control.orientation.x = 0;
            control.orientation.y = 1;
            control.orientation.z = 0;
            control.name = "rotate_z";
            control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_AXIS;
            int_marker_.controls.push_back(control);
            control.name = "move_z";
            // double z = 0.0;
            // control.pose.position.z = ros::param::get("~z", z);
            control.interaction_mode = visualization_msgs::InteractiveMarkerControl::MOVE_AXIS;
//            int_marker_.controls.push_back(control);
            // control.pose.position.z = 0.0;


        }

        server_->insert(int_marker_);
        server_->setCallback(int_marker_.name,
                             boost::bind(&InteractiveTf::processFeedback, this, 0, boost::placeholders::_1));
        // server_->setCallback(int_marker_.name, testFeedback);

        server_->applyChanges();

        if (pub_tf) {
            tf_timer_ = nh_.createTimer(ros::Duration(0.05),
                                        boost::bind(&InteractiveTf::updateTf, this, 0, boost::placeholders::_1));
        }
        is_start = true;
        std::cout << "InteractiveTf start" << std::endl;
    }

    InteractiveTf::~InteractiveTf() {
        server_.reset();
    }

    void InteractiveTf::updateTf(int, const ros::TimerEvent &event) {
        tf::Transform transform;
        transform.setOrigin(tf::Vector3(pose_.position.x, pose_.position.y, pose_.position.z));
        transform.setRotation(tf::Quaternion(pose_.orientation.x,
                                             pose_.orientation.y,
                                             pose_.orientation.z,
                                             pose_.orientation.w));
        br_.sendTransform(tf::StampedTransform(transform, ros::Time::now(),
                                               parent_frame_, frame_));
    }

    void InteractiveTf::processFeedback(
            unsigned ind,
            const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback) {
        ROS_DEBUG_STREAM(feedback->header.frame_id);
        pose_ = feedback->pose;
        if (m_callback) {
            m_callback(pose_);
        }
        ROS_DEBUG_STREAM(feedback->control_name);
        ROS_DEBUG_STREAM(feedback->event_type);
        ROS_DEBUG_STREAM(feedback->mouse_point);
        // TODO(lucasw) all the pose changes get handled by the server elsewhere?
        server_->applyChanges();
    }
}


// LaserScan to PointCloud

void createPointCloud2(sensor_msgs::PointCloud2 &cloud, const std::vector<std::string> &filed) {
//    cloud.header.frame_id = "map";
    cloud.height = 1;
    int filed_num = filed.size();

    cloud.point_step = 4 * filed_num;
    cloud.is_dense = false;
    cloud.is_bigendian = false;
    cloud.fields.resize(filed_num);

    for (int i = 0; i < filed_num; i++) {

        cloud.fields[i].name = filed[i];
        cloud.fields[i].offset = 4 * i;
        cloud.fields[i].datatype = sensor_msgs::PointField::FLOAT32;
        cloud.fields[i].count = 1;
    }
#if 0
    cloud.fields[0].name = "x";
    cloud.fields[0].offset = 0;
    cloud.fields[0].datatype = sensor_msgs::PointField::FLOAT32;
    cloud.fields[0].count = 1;

    cloud.fields[1].name = "y";
    cloud.fields[1].offset = 4;
    cloud.fields[1].datatype = sensor_msgs::PointField::FLOAT32;
    cloud.fields[1].count = 1;

    cloud.fields[2].name = "z";
    cloud.fields[2].offset = 8;
    cloud.fields[2].datatype = sensor_msgs::PointField::FLOAT32;
    cloud.fields[2].count = 1;

    cloud.fields[3].name = "intensity";
    cloud.fields[3].offset = 12;
    cloud.fields[3].datatype = sensor_msgs::PointField::FLOAT32;
    cloud.fields[3].count = 1;
#endif

}

/*
 fill cloud filed [x,y]
 */
void LaserScanToPointCloud2(const std::vector<float> &scan_points, int point_num, sensor_msgs::PointCloud2 &cloud) {
    int point_num_ = 0.5 * (scan_points.size());
    if (point_num_ <= point_num) {
        return;
    }
    cloud.row_step = point_num * cloud.point_step;
    cloud.data.resize(cloud.row_step);
    cloud.width = point_num;
    for (int i = 0; i < point_num; i++) {
        uint8_t *data_pointer = &cloud.data[0] + i * cloud.point_step;
        *(float *) data_pointer = scan_points[i + i];
        data_pointer += 4;
        *(float *) data_pointer = scan_points[i + i + 1];
        data_pointer += 4;
        *(float *) data_pointer = 0.0;

    }
}

void LaserScanToPclPointCloud(const std::vector<float> &scan_points, int point_num,
                              pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_ptr) {


    auto &cloud = *cloud_ptr;
    // Fill in the cloud data
    cloud.width = point_num;
    cloud.height = 1;
//    cloud.is_dense = true;
    cloud.resize(cloud.width * cloud.height, pcl::PointXYZ{0.0, 0.0, 0.0});

    for (int i = 0; i < point_num; i++) {
        cloud[i].x = scan_points[i + i];
        cloud[i].y = scan_points[i + i + 1];

    }

}

void LaserScanToPclPointCloud(const std::vector<float> &scan_points, int point_num,
                              pcl::PointCloud<pcl::PointNormal>::Ptr cloud_ptr) {


    auto &cloud = *cloud_ptr;
    // Fill in the cloud data
    cloud.width = point_num;
    cloud.height = 1;
//    cloud.is_dense = true;
    cloud.resize(cloud.width * cloud.height, pcl::PointNormal{0.0, 0.0, 0.0});

    for (int i = 0; i < point_num; i++) {
        cloud[i].x = scan_points[i + i];
        cloud[i].y = scan_points[i + i + 1];

    }

}

// PointCloud to
void PclPointCloudToPointCloud2(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_ptr, sensor_msgs::PointCloud2 &cloud) {
    cloud.width = cloud_ptr->width;

    int point_num = cloud_ptr->size();

    cloud.row_step = point_num * cloud.point_step;
    cloud.data.resize(cloud.row_step);
    cloud.width = point_num;
    for (int i = 0; i < point_num; i++) {
        uint8_t *data_pointer = &cloud.data[0] + i * cloud.point_step;
        *(float *) data_pointer = cloud_ptr->at(i).x;
        data_pointer += 4;
        *(float *) data_pointer = cloud_ptr->at(i).y;
        data_pointer += 4;
        *(float *) data_pointer = 0.0;

    }


}

// PointCloud to
void PclPointCloudToPointCloud2(pcl::PointCloud<pcl::PointNormal>::Ptr cloud_ptr, sensor_msgs::PointCloud2 &cloud) {
    cloud.width = cloud_ptr->width;

    int point_num = cloud_ptr->size();

    cloud.row_step = point_num * cloud.point_step;
    cloud.data.resize(cloud.row_step);
    cloud.width = point_num;
    for (int i = 0; i < point_num; i++) {
        uint8_t *data_pointer = &cloud.data[0] + i * cloud.point_step;
        *(float *) data_pointer = cloud_ptr->at(i).x;
        data_pointer += 4;
        *(float *) data_pointer = cloud_ptr->at(i).y;
        data_pointer += 4;
        *(float *) data_pointer = 0.0;
    }
}

void PclPointCloudToPoseArray(pcl::PointCloud<pcl::PointNormal>::Ptr cloud_ptr, geometry_msgs::PoseArray &pose_array) {


    int point_num = cloud_ptr->size();

    pose_array.poses.resize(point_num);

    float yaw = 0.0;
//    std::cout << "\nshow all yaw:\n";
    for (int i = 0; i < point_num; i++) {
        pose_array.poses[i].position.x = cloud_ptr->points[i].x;
        pose_array.poses[i].position.y = cloud_ptr->points[i].y;
        pose_array.poses[i].position.z = cloud_ptr->points[i].z;

//        pose_array.poses[i].orientation.x = 0.0;
//        pose_array.poses[i].orientation.y = 0.0;
//        pose_array.poses[i].orientation.z = 0.0;
//        pose_array.poses[i].orientation.w = 1.0;


        yaw = atan2(cloud_ptr->points[i].normal_y, cloud_ptr->points[i].normal_x);
//        std::cout << "[" << yaw << ", " << cloud_ptr->points[i].normal_y << ", " << cloud_ptr->points[i].normal_x << "], ";
        yaw_to_quaternion(yaw, pose_array.poses[i].orientation.x, pose_array.poses[i].orientation.y,
                          pose_array.poses[i].orientation.z, pose_array.poses[i].orientation.w);

    }
//    std::cout << "\nend show all yaw\n";

}


struct PointsStamped {
    std::vector<float> points;
    ros::Time stamp;
};

typedef pcl::PointXYZ PointType;
typedef pcl::PointCloud<PointType> Cloud;
typedef Cloud::ConstPtr CloudConstPtr;
typedef Cloud::Ptr CloudPtr;


#include <fstream>

//https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
bool loadFileToStr(const std::string &filename, std::string &data) {
    std::ifstream t(filename.c_str());

    if (!t.is_open()) {
        return false;
    }
    std::stringstream buffer;
    buffer << t.rdbuf();
    data = buffer.str();
    return true;
}

bool dumpStrToFile(const std::string &filename, const std::string &data) {
    std::ofstream t(filename.c_str());
    t << data;

    return true;

}


struct MovementCheck {
    float no_move_translation_epsilon = 5e-3;
    float no_move_rotation_epsilon = 5e-3;
    float final_no_move_translation_epsilon = 1e-2;
    float final_no_move_rotation_epsilon = 1e-2;
    float no_move_check_ms = 200.0;
    transform::Transform2d last_pose_inv;
    transform::Transform2d start_check_pose_inv;

    transform::Transform2d movement;

    common::Time last_time;

    bool start_check = false;
    bool still = false;

    bool check(const transform::Transform2d &new_pose) {

        movement = last_pose_inv * new_pose;

        bool no_move = std::abs(movement.x()) < no_move_translation_epsilon &&
                       std::abs(movement.y()) < no_move_translation_epsilon
                       && std::abs(movement.yaw()) < no_move_rotation_epsilon;

        movement = start_check_pose_inv * new_pose;

        no_move = no_move && std::abs(movement.x()) < final_no_move_translation_epsilon &&
                  std::abs(movement.y()) < final_no_move_translation_epsilon
                  && std::abs(movement.yaw()) < final_no_move_rotation_epsilon;
        if ((no_move && !start_check) || !no_move) {
            last_time = common::FromUnixNow();
            start_check_pose_inv = new_pose.inverse();
        }
        last_pose_inv = new_pose.inverse();

        start_check = no_move;
        still = common::ToMillSeconds(common::FromUnixNow() - last_time) > no_move_check_ms;


        return still;

    }

    bool isStill() const{
        return still;
    }


};


namespace pcl{
    /*
             pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
        pcl::search::KdTree<pcl::PointXYZ> kdtree_2;
        pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> octree (resolution);

     */

    template<typename PointT>
    void createTree(typename pcl::PointCloud<PointT>::ConstPtr t_input_cloud, pcl::KdTreeFLANN<PointT>& t_tree){
        t_tree.setInputCloud(t_input_cloud);
    }

    template<typename PointT>
    void createTree(typename pcl::PointCloud<PointT>::ConstPtr t_input_cloud, pcl::search::KdTree<PointT>& t_tree){
        t_tree.setInputCloud(t_input_cloud);
    }

    template<typename PointT>
    void createTree(typename pcl::PointCloud<PointT>::ConstPtr t_input_cloud, pcl::octree::OctreePointCloudSearch<PointT>& t_tree){
        t_tree.setInputCloud(t_input_cloud);
        t_tree.addPointsFromInputCloud ();
    }


    template<typename PointT, typename TreeType>
    class Normal2dEstimation{

    public:
        using PointCloud = pcl::PointCloud<PointT>;
        using PointCloudPtr = typename PointCloud::Ptr;
        using PointCloudConstPtr = typename PointCloud::ConstPtr;
        using Scalar = float;

    private:
        const TreeType& m_tree;
        float m_query_radius = 0.1;
        PointCloudConstPtr m_input_cloud;

        std::vector<int> m_query_indices;
        std::vector<float> m_query_distance;

    public:
        Normal2dEstimation(const TreeType& t_tree, float radius = 0.1, int num = 20):
                m_tree(t_tree),
                m_query_radius(radius),
                m_query_indices(num),
                m_query_distance(num){

        }

        inline void
        setInputCloud(const PointCloudConstPtr& t_input_cloud)
        {
            m_input_cloud = t_input_cloud;
        }
        void setRadius(){

        }
        void setTree(){

        }
        void setInputCloud(){

        }

        void compute( const pcl::PointCloud<pcl::PointNormal>::Ptr&  output){

            int input_point_num = m_input_cloud->points.size();
            output->points.resize(input_point_num,pcl::PointNormal(0.0,0.0,0.0,0.0,0.0,0.0));
            output->height = m_input_cloud->height;
            output->width = m_input_cloud->width;
            output->is_dense = true;

            int rt = 0;
            Eigen::Matrix<Scalar, 1, 5, Eigen::RowMajor> accu = Eigen::Matrix<Scalar, 1, 5, Eigen::RowMajor>::Zero ();
            Eigen::Matrix<Scalar, 2, 1> K(0.0, 0.0);
            Eigen::Matrix<Scalar, 4, 1> centroid;
            Eigen::Matrix<Scalar, 2, 2> covariance_matrix;
            Eigen::SelfAdjointEigenSolver<Eigen::Matrix2f> eig_solver(2);

            for(int i = 0 ; i < input_point_num;i++){


                auto& query_point = m_input_cloud->at(i);
                rt = m_tree.radiusSearch (query_point, m_query_radius, m_query_indices, m_query_distance);

                std::size_t point_count;
                point_count = m_query_indices.size ();
                if(point_count <=3){
                    output->points[i].normal_x  = output->points[i].normal_y  = output->points[i].normal_z  = output->points[i].curvature = std::numeric_limits<float>::quiet_NaN ();
                    continue;
                }


                K.x() = m_input_cloud->at(m_query_indices[0]).x;
                K.y() = m_input_cloud->at(m_query_indices[0]).y;


                for (const auto &index : m_query_indices)
                {
                    Scalar x = m_input_cloud->at(index).x - K.x(), y = m_input_cloud->at(index).y - K.y();
                    accu [0] += x * x;
                    accu [1] += x * y;
                    accu [2] += y * y;
                    accu [3] += x;
                    accu [4] += y;
                }


                {
                    accu /= static_cast<Scalar> (point_count);
                    centroid[0] = accu[3] + K.x(); centroid[1] = accu[4] + K.y(); centroid[2] = 0.0;
                    centroid[3] = 1;
                    covariance_matrix.coeffRef (0) = accu [0] - accu [3] * accu [3];//xx
                    covariance_matrix.coeffRef (1) = accu [1] - accu [3] * accu [4];//xy
                    covariance_matrix.coeffRef (3) = accu [2] - accu [4] * accu [4];//yy
                    covariance_matrix.coeffRef (2) = covariance_matrix.coeff (1);//yx



                    eig_solver.compute(covariance_matrix);
#if 0
                    {
                        Eigen::MatrixX2f m(m_query_indices.size(),2);
                        int index = 0;
                        for(int j :  m_query_indices){
                            m(index,0) = m_input_cloud->at(j).x ;
                            m(index,1) = m_input_cloud->at(j).y ;
                            index++;
                        }

                        Eigen::VectorXf mean_vector = m.colwise().mean();

                        Eigen::MatrixXf centered = m.rowwise() - mean_vector.transpose();

                        Eigen::MatrixXf cov = (centered.adjoint() * centered) / ( m.rows() - 1 ) ;
                        eig_solver.compute(cov);

                    }
#endif






                    auto& eigen_values = eig_solver.eigenvalues();
                    auto& eigen_vectors = eig_solver.eigenvectors() ;
                    auto& nx =  output->points[i].normal_x;
                    auto& ny =  output->points[i].normal_y;
                    auto& nz =  output->points[i].normal_z;
                    nx = eigen_vectors(0,0);
                    ny = eigen_vectors(1,0);
#if 0
                    if(std::abs(eigen_values(0)) < std::abs(eigen_values(1))){
                        nx = eigen_vectors(0,0);
                        ny = eigen_vectors(1,0);
                    }else{
                        nx = eigen_vectors(0,1);
                        ny = eigen_vectors(1,1);
                    }
#endif
                    nz = 0.0;

                    Eigen::Matrix <float, 2, 1> normal (nx, ny);
                    Eigen::Matrix <float, 2, 1> vp ( - query_point.x, - query_point.y);

                    // Dot product between the (viewpoint - point) and the plane normal
                    float cos_theta = vp.dot (normal);
                    // Flip the plane normal
                    if (cos_theta < 0)
                    {
                        nx *= -1;
                        ny *= -1;
                    }


//                    std::cout << __LINE__ << "eigenvalues:\n" << eigen_values << std::endl;
//                    std::cout << __LINE__ << "eigenvectors:\n" << eigen_vectors << std::endl;
                }


            }

        }

    };
}


/*
use 2d/3d pointcloud to solve robot pose;

 main function
 1. filter input cloud
 2. match with reference cloud
 3. update map use input cloud and exist reference cloud


 details:
 1.1 voxel grid filter, radius filter, condition filter

 2.1 point to plane icp, weights

 3.1 use icp solved pose and cloud to update reference
 3.2 due to noise in icp solved pose, add every input cloud to reference cloud may lead to mess
 3.3 take movement into consideration, only use input cloud in standstill state


 */
class PointCloudPoseSolver {
public:
    enum class Mode {
        mapping,
        localization
    };

    struct FilterReadingConfig {
        float voxel_grid_size = 0.05;
        float radius_rmv_radius = 0.09;
        int radius_rmv_nn = 3;
        float cond_curve = 0.1;

    };

    struct FilterReferenceConfig {
        float voxel_grid_size = 0.1;
        float radius_rmv_radius = 0.2;
        int radius_rmv_nn = 4;
    };

    struct NormEstConfig {
        float radius = 0.06;
    };

    struct IcpConfig {

        float max_match_distance = 0.2;
        float ransac_outlier = 0.03;
        int max_iter = 20;
        float transformation_epsilon = 1e-4;
        float rotation_epsilon = 1e-4;


    };

    struct MoveCheckConfig {
        float no_move_translation_epsilon = 5e-3;
        float no_move_rotation_epsilon = 5e-3;
        float final_no_move_translation_epsilon = 1e-2;
        float final_no_move_rotation_epsilon = 1e-2;
        float no_move_check_ms = 200.0;

    };

    FilterReadingConfig &getFilterReadingConfig() {

        return m_filter_reading_config;
    }

    FilterReferenceConfig &getFilterReferenceConfig() {

        return m_filter_reference_config;
    }

    NormEstConfig &getNormEstConfig() {
        return m_norm_est_config;
    }

    IcpConfig &getIcpConfig() {
        return m_icp_config;
    }

    MoveCheckConfig &getMoveCheckConfig() {
        return m_move_check_config;
    }


private:
    FilterReadingConfig m_filter_reading_config;
    FilterReferenceConfig m_filter_reference_config;
    NormEstConfig m_norm_est_config;
    IcpConfig m_icp_config;
    MoveCheckConfig m_move_check_config;

    Mode mode = Mode::mapping;

    // cloud reference frame pose, in /map frame
    transform::Transform2d origin;
    // sensor pose in /base_link frame
    transform::Transform2d sensor_relative_pose;
    // sensor pose in /origin frame
    transform::Transform2d sensor_absolute_pose;
    // robot pose in origin frame
    transform::Transform2d robot_relative_pose;
    // robot pose in /map frame
    transform::Transform2d robot_absolute_pose;

    // odom to base pose
    transform::Transform2d robot_odom_pose;

    // map to odom
    transform::Transform2d map_odom_pose;


    bool is_first_cloud = true;
    // use fist robot pose to compute origin
    bool is_origin_computed = false;

    bool is_sensor_pose_set = false;

    bool is_icp_init = true;


    // cloud as map
    pcl::PointCloud<pcl::PointNormal>::Ptr cloud_reference;
    pcl::PointCloud<pcl::PointXYZ>::Ptr temp_cloud_1;
    pcl::PointCloud<pcl::PointXYZ>::Ptr temp_cloud_2;

    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_reading_filtered;
    pcl::PointCloud<pcl::PointNormal>::Ptr cloud_reading_filtered_norm;
    pcl::PointCloud<pcl::PointNormal>::Ptr cloud_align;
    pcl::PointCloud<pcl::PointNormal>::Ptr temp_cloud_norm;


    // filter reading
    pcl::RadiusOutlierRemoval<pcl::PointXYZ> m_radius_outlier_removal;
    pcl::VoxelGrid<pcl::PointXYZ> m_approximate_voxel_grid;
    pcl::ConditionAnd<pcl::PointNormal>::Ptr m_view_cond;

    pcl::ConditionalRemoval<pcl::PointNormal> m_cond_rem;


    // filter reference
    pcl::RadiusOutlierRemoval<pcl::PointXYZ> m_refe_radius_outlier_removal;
    pcl::VoxelGrid<pcl::PointXYZ> m_refe_approximate_voxel_grid;


    // icp
    pcl::registration::WarpPointRigid3D<pcl::PointNormal, pcl::PointNormal>::Ptr m_warp_fcn_pl;
    pcl::IterativeClosestPointWithNormals<pcl::PointNormal, pcl::PointNormal> m_pl_icp;


//    pcl::registration::TransformationEstimationPointToPlaneWeighted<pcl::PointNormal, pcl::PointNormal>::Ptr m_pl_te;
    pcl::registration::TransformationEstimationPointToPlane<pcl::PointNormal, pcl::PointNormal>::Ptr m_pl_te;

    // norm est
    pcl::search::KdTree<pcl::PointXYZ>::Ptr m_norm_tree;

    pcl::octree::OctreePointCloudSearch<pcl::PointNormal> m_octree_reading ;
    pcl::octree::OctreePointCloudSearch<pcl::PointNormal> m_octree_reference ;
    pcl::Normal2dEstimation<pcl::PointNormal, pcl::octree::OctreePointCloudSearch<pcl::PointNormal>> m_norm_est_reading;
    pcl::Normal2dEstimation<pcl::PointNormal, pcl::octree::OctreePointCloudSearch<pcl::PointNormal>> m_norm_est_reference;


    Normal2dEstimation m_norm_estim;

    MovementCheck m_movement_check;


public:
    PointCloudPoseSolver() : cloud_reference(new pcl::PointCloud<pcl::PointNormal>),
                             temp_cloud_1(new pcl::PointCloud<pcl::PointXYZ>),
                             temp_cloud_2(new pcl::PointCloud<pcl::PointXYZ>),

                             cloud_reading_filtered(new pcl::PointCloud<pcl::PointXYZ>),
                             cloud_reading_filtered_norm(new pcl::PointCloud<pcl::PointNormal>),
                             cloud_align(new pcl::PointCloud<pcl::PointNormal>),
                             temp_cloud_norm(new pcl::PointCloud<pcl::PointNormal>),

                             m_view_cond(new pcl::ConditionAnd<pcl::PointNormal>()),

                             m_warp_fcn_pl
                                     (new pcl::registration::WarpPointRigid3D<pcl::PointNormal, pcl::PointNormal>),
                             m_norm_tree(new pcl::search::KdTree<pcl::PointXYZ>),

                             m_pl_te(
                                     new pcl::registration::TransformationEstimationPointToPlane<pcl::PointNormal, pcl::PointNormal>) ,
                             m_octree_reading(0.05),
                             m_octree_reference(0.05),m_norm_est_reading(m_octree_reading),m_norm_est_reference(m_octree_reference){


        config();

    }


    // config filter, norm_est, icp
    void config() {

        m_approximate_voxel_grid.setLeafSize(m_filter_reading_config.voxel_grid_size,
                                             m_filter_reading_config.voxel_grid_size,
                                             m_filter_reading_config.voxel_grid_size);
        m_radius_outlier_removal.setRadiusSearch(m_filter_reading_config.radius_rmv_radius);
        m_radius_outlier_removal.setMinNeighborsInRadius(m_filter_reading_config.radius_rmv_nn);


        m_view_cond->addComparison(pcl::FieldComparison<pcl::PointNormal>::ConstPtr(
                new pcl::FieldComparison<pcl::PointNormal>("curvature", pcl::ComparisonOps::GT,
                                                           m_filter_reading_config.cond_curve)));
        m_cond_rem.setCondition(m_view_cond);

        //        m_cond_rem.setKeepOrganized(true);


        m_refe_approximate_voxel_grid.setLeafSize(m_filter_reference_config.voxel_grid_size,
                                                  m_filter_reference_config.voxel_grid_size,
                                                  m_filter_reference_config.voxel_grid_size);
        m_refe_radius_outlier_removal.setRadiusSearch(m_filter_reference_config.radius_rmv_radius);
        m_refe_radius_outlier_removal.setMinNeighborsInRadius(m_filter_reference_config.radius_rmv_nn);


        m_norm_estim.setSearchMethod(m_norm_tree);
        m_norm_estim.setRadiusSearch(m_norm_est_config.radius);

        m_pl_te->setWarpFunction(m_warp_fcn_pl);


        m_pl_icp.setTransformationEstimation(m_pl_te);

        m_pl_icp.setMaxCorrespondenceDistance(m_icp_config.max_match_distance);

        m_pl_icp.setRANSACOutlierRejectionThreshold(m_icp_config.ransac_outlier);

        m_pl_icp.setMaximumIterations(m_icp_config.max_iter);
        m_pl_icp.setTransformationEpsilon(1e-5);
        m_pl_icp.setEuclideanFitnessEpsilon(1e-5);
        m_pl_icp.setTransformationRotationEpsilon(1e-5);


        m_movement_check.no_move_check_ms = m_movement_check.no_move_check_ms;
        m_movement_check.no_move_translation_epsilon = m_movement_check.no_move_translation_epsilon;
        m_movement_check.no_move_rotation_epsilon = m_movement_check.no_move_rotation_epsilon;
        m_movement_check.final_no_move_translation_epsilon = m_movement_check.final_no_move_translation_epsilon;
        m_movement_check.final_no_move_rotation_epsilon = m_movement_check.final_no_move_rotation_epsilon;


    }


    pcl::PointCloud<pcl::PointNormal>::Ptr getCloudReading() const {
        return cloud_reading_filtered_norm;
    }

    pcl::PointCloud<pcl::PointNormal>::Ptr getCloudMatched() const {
        return cloud_align;
    }

    pcl::PointCloud<pcl::PointNormal>::Ptr getCloudReference() const {
        return cloud_reference;
    }

    void setMode(const Mode &m) {
        mode = m;
    }

    // voxel_grrid_filter
    // radius_remove_filter
    void
    filterReading(pcl::PointCloud<pcl::PointXYZ>::Ptr t_cloud, pcl::PointCloud<pcl::PointXYZ>::Ptr t_cloud_filtered) {
        m_approximate_voxel_grid.setInputCloud(t_cloud);
        m_approximate_voxel_grid.filter(*temp_cloud_1);

        m_radius_outlier_removal.setInputCloud(temp_cloud_1);
        m_radius_outlier_removal.filter(*t_cloud_filtered);
    }


    float icp_match(pcl::PointCloud<pcl::PointNormal>::Ptr t_reading, pcl::PointCloud<pcl::PointNormal>::Ptr t_cloud,
                    pcl::PointCloud<pcl::PointNormal>::Ptr t_align,
                    transform::Transform2d &est_pose) {


        Eigen::Matrix4f initial_guess(Eigen::Matrix4f::Identity ());
        initial_guess(0, 0) = est_pose.matrix[0][0];
        initial_guess(0, 1) = est_pose.matrix[0][1];
        initial_guess(1, 0) = est_pose.matrix[1][0];
        initial_guess(1, 1) = est_pose.matrix[1][1];
        initial_guess(0, 3) = est_pose.matrix[0][2];
        initial_guess(1, 3) = est_pose.matrix[1][2];


        m_pl_icp.setInputSource(t_reading);
        m_pl_icp.setInputTarget(t_cloud);
        m_pl_icp.align(*t_align, initial_guess);


        Eigen::Matrix4f result_pose = m_pl_icp.getFinalTransformation();

        est_pose.matrix[0][0] = result_pose(0, 0);
        est_pose.matrix[0][1] = result_pose(0, 1);
        est_pose.matrix[1][0] = result_pose(1, 0);
        est_pose.matrix[1][1] = result_pose(1, 1);
        est_pose.matrix[0][2] = result_pose(0, 3);
        est_pose.matrix[1][2] = result_pose(1, 3);

        est_pose.matrix[2][0] = 0.0;
        est_pose.matrix[2][1] = 0.0;
        est_pose.matrix[2][2] = 1.0;

        std::cout << "m_pl_icp has converged:" << m_pl_icp.hasConverged() << " score: " <<
                  m_pl_icp.getFitnessScore() << std::endl;
        std::cout << "getFinalTransformation:\n" << m_pl_icp.getFinalTransformation() << "\n";


        return m_pl_icp.getFitnessScore();

    }

    // update reference with aligned pointcloud
    // filter
    void updateReference(const pcl::PointCloud<pcl::PointNormal>::Ptr t_reading) {

        if (cloud_reference->size() == 0) {
            (*cloud_reference) += (*t_reading);
            return;
        }


        (*cloud_reference) += (*t_reading);

#if 1

        pcl::copyPointCloud(*cloud_reference, *temp_cloud_1);

        m_refe_approximate_voxel_grid.setInputCloud(temp_cloud_1);
        m_refe_approximate_voxel_grid.filter(*temp_cloud_2);

        m_refe_radius_outlier_removal.setInputCloud(temp_cloud_2);
        m_refe_radius_outlier_removal.filter(*temp_cloud_1);

        computeNorm(temp_cloud_1, cloud_reference,
                    pcl::PointXYZ(sensor_absolute_pose.x(), sensor_absolute_pose.y(), 0.0));
#endif



#if 0
        pcl::copyPointCloud(*cloud_reference, *temp_cloud_1);

        m_approximate_voxel_grid.setInputCloud(temp_cloud_1);
        m_approximate_voxel_grid.filter(*cloud_reading_filtered);

        m_radius_outlier_removal.setInputCloud(cloud_reading_filtered);
        m_radius_outlier_removal.filter(*temp_cloud_1);
        computeNorm(temp_cloud_1, cloud_reference);

#endif

    }

    // compute norm
    void computeNorm(pcl::PointCloud<pcl::PointXYZ>::Ptr t_cloud, pcl::PointCloud<pcl::PointNormal>::Ptr t_cloud_norm,
                     const pcl::PointXYZ &view_point = pcl::PointXYZ(0.0, 0.0, 0.0)) {

        pcl::copyPointCloud(*t_cloud, *t_cloud_norm);
        m_norm_estim.setInputCloud(t_cloud);
        m_norm_estim.compute(t_cloud_norm);
        m_norm_estim.setViewPoint(view_point);
    }

    void removeReadingShadow(pcl::PointCloud<pcl::PointNormal>::Ptr t_cloud,
                             pcl::PointCloud<pcl::PointNormal>::Ptr t_cloud_norm) {

        m_cond_rem.setInputCloud(t_cloud);
        m_cond_rem.filter(*t_cloud_norm);
    }


    void matchUpdate(const transform::Transform2d &odom_pose) {

//        std::cout << __FILE__ << ":" << __LINE__ << " , odom_pose : " << odom_pose << "\n";

        bool update = m_movement_check.check(odom_pose);
//        std::cout << __FILE__ << ":" << __LINE__ << " , update : " << update << ", size : " << cloud_reference->size()   << "\n";


        if (update || cloud_reference->size() == 0) {

//            std::cout << __FILE__ << ":" << __LINE__ << " call updateReference \n";

            updateReference(cloud_align);

        }

    }

    void match() {

        if (!cloud_reference->empty()) {
            if (!m_movement_check.isStill()) {
                std::cout << " perform icp\n";
                float score = icp_match(cloud_reading_filtered_norm, cloud_reference, cloud_align,
                                        sensor_absolute_pose);
            } else {
                std::cout << " bypass icp\n";

                Eigen::Matrix4f initial_guess(Eigen::Matrix4f::Identity ());
                initial_guess(0, 0) = sensor_absolute_pose.matrix[0][0];
                initial_guess(0, 1) = sensor_absolute_pose.matrix[0][1];
                initial_guess(1, 0) = sensor_absolute_pose.matrix[1][0];
                initial_guess(1, 1) = sensor_absolute_pose.matrix[1][1];
                initial_guess(0, 3) = sensor_absolute_pose.matrix[0][2];
                initial_guess(1, 3) = sensor_absolute_pose.matrix[1][2];
                pcl::transformPointCloud(*cloud_reading_filtered_norm, *cloud_align, initial_guess);

            }

        } else {
            (*cloud_reference) += (*cloud_reading_filtered_norm);

        }

        robot_relative_pose = sensor_absolute_pose * sensor_relative_pose.inverse();
        robot_absolute_pose = origin * robot_relative_pose;


    }

    void match_v2() {

        if(!is_icp_init){

            std::cout << "icp init_pose is not set in loc mode" << std::endl;

            return;
        }

        if (!cloud_reference->empty()) {

            float score = icp_match(cloud_reading_filtered_norm, cloud_reference, cloud_align, sensor_absolute_pose);


        } else {
            (*cloud_reference) += (*cloud_reading_filtered_norm);

        }

        robot_relative_pose = sensor_absolute_pose * sensor_relative_pose.inverse();
        robot_absolute_pose = origin * robot_relative_pose;


    }

    bool matchReading() {

        if (cloud_reference->size() == 0) {
            //updateReference(cloud_reading_filtered_norm);
            (*cloud_reference) += (*cloud_reading_filtered_norm);

            return true;
        }

        float score = icp_match(cloud_reading_filtered_norm, cloud_reference, cloud_align, sensor_absolute_pose);

        std::cout << "sensor_absolute_pose:\n" << sensor_absolute_pose << std::endl;
#if 0
        updateReference(cloud_align);
#endif

        return true;
    }


    // in loc mode
    // cloud_reference should be loadded from file
    int loadFromFile(const std::string & file_dir, const std::string& stamp) {

        fs::path output_dir(file_dir);
        fs::path output_stamp_dir = output_dir / stamp;

        fs::path path_pose = output_stamp_dir / "origin.json";
        fs::path path_pcd = output_stamp_dir / "cloud.pcd";

        if(! (fs::exists(path_pose)  && fs::exists(path_pcd) ) ){
            std::cerr << "solver load file error, " << path_pose.c_str() << ", " << path_pcd.c_str() << " not found"<< std::endl;

            return -1;
        }

        if (pcl::io::loadPCDFile<pcl::PointNormal> (path_pcd.c_str(), *cloud_reference) == -1) //* load the file
        {
            PCL_ERROR ("Couldn't read file %s \n",path_pcd.c_str());
            return (-1);
        }

        std::string raw_data;
        loadFileToStr(path_pose,raw_data);
        nlohmann::json json_data = nlohmann::json::parse(raw_data);


        bool rt = serialization::from_json(json_data,"origin",origin);
        if(!rt){
            std::cout << "prase error: raw_data:\n" << raw_data << std::endl;

            return -1;
        }

        is_origin_computed = true;
        is_icp_init = false;

        std::cout << "load from file ok ; origin:\n" << origin << "\ncloud_reference size :  " << cloud_reference->size() << std::endl;


        return 0;
    }

    // in mapping mode
    // cloud_reference should be dumped to fle
    void dumpToFile(const std::string &file_dir) {

        std::string stamp = common::getCurrentDateTime("%Y-%m-%d-%H-%M-%S");

        fs::path output_dir(file_dir);

        if (!fs::exists(output_dir)) {
            fs::create_directories(output_dir);
        }
        fs::path output_stamp_dir = output_dir / stamp;

        if (!fs::exists(output_stamp_dir)) {
            fs::create_directories(output_stamp_dir);
        }



        fs::path path_pose = output_stamp_dir / "origin.json";
        fs::path path_pcd = output_stamp_dir / "cloud.pcd";


        nlohmann::json json_data;
        serialization::to_json(json_data,"origin",origin);

        dumpStrToFile(path_pose,json_data.dump() );

        pcl::io::savePCDFileASCII(path_pcd.c_str(), *cloud_reference);
        std::cerr << "Saved " << cloud_reference->size() << " data points to " << path_pcd.c_str() << std::endl;

    }



    // add pointcloud with   movement assumption
    void addSensorReading(pcl::PointCloud<pcl::PointXYZ>::Ptr t_cloud_reading,
                          const transform::Transform2d &movement = transform::Transform2d()) {

        // input_cloud ->filter -> filtered_input_cloud
        filterReading(t_cloud_reading, cloud_reading_filtered);

#if 1
        // filtered_input_cloud + norm for icp
        computeNorm(cloud_reading_filtered, temp_cloud_norm);

        //
        removeReadingShadow(temp_cloud_norm, cloud_reading_filtered_norm);
#endif

#if 0
        // filtered_input_cloud + norm for icp
        computeNorm(cloud_reading_filtered, cloud_reading_filtered_norm);

        //
//        removeReadingShandow(temp_cloud_norm, cloud_reading_filtered_norm);
#endif


    }

    // set origin pose directly
    void setOrigin(const transform::Transform2d &t) {
        origin = t;
    }

    const transform::Transform2d &getOrigin() {
        return origin;
    }

    void setSensorRelativePose(const transform::Transform2d &t) {
        sensor_relative_pose = t;
        is_sensor_pose_set = true;
    }

    bool isOriginCompute(){
        return is_origin_computed;
    }

    void setRobotAbsolutePoseOrigin(const transform::Transform2d &t) {
        if (is_sensor_pose_set && !is_origin_computed) {
            robot_absolute_pose = t;
            origin = robot_absolute_pose * robot_relative_pose.inverse();

            is_origin_computed = true;
        }
    }

    void setRobotAbsolutePoseInitIcp(const transform::Transform2d &t) {
        if (is_sensor_pose_set &&  is_origin_computed && !is_icp_init) {
            robot_absolute_pose = t;
            robot_relative_pose = origin.inverse() * robot_absolute_pose;

            sensor_absolute_pose = robot_relative_pose *sensor_relative_pose;
            is_icp_init = true;
        }
    }

    const transform::Transform2d &getRobotAbsolutePose() {
        return robot_absolute_pose;
    }

    const transform::Transform2d &getRobotRelativePose() {
        return robot_relative_pose;
    }

    const transform::Transform2d &solveMapOdom(const transform::Transform2d &t_odom_base) {

        robot_relative_pose = sensor_absolute_pose * sensor_relative_pose.inverse();
        robot_absolute_pose = origin * robot_relative_pose;
        map_odom_pose = robot_absolute_pose * t_odom_base.inverse();
        return map_odom_pose;
    }

};







int main(int argc, char **argv) {


    std::vector<PointsStamped> laser_points_cache;

    //==== ros
    ros::init(argc, argv, "scan_matching");
    ros::NodeHandle nh;
    ros::NodeHandle nh_private("~");


    std::string file_dir = "data";
    std::string file_dir_param = "file_dir";

    std::string load_dir ;
    std::string load_dir_param = "load_dir";

    std::string mode = "map";
    const char* MODE_MAP = "map";
    const char* MODE_LOC = "loc";

    const char* dump_data_param = "dump_data";
    bool dump_data = false;


    std::string mode_param = "mode";


    float range_max = 30.0;
    std::string range_max_param = "range_max";
    float range_min = 1.0;
    std::string range_min_param = "range_min";

    float pcl_voxel_leaf_x = 0.05;
    float pcl_voxel_leaf_y = 0.05;
    float pcl_voxel_leaf_z = 0.05;
    std::string pcl_voxel_leaf_x_param = "pcl_voxel_leaf_x";
    std::string pcl_voxel_leaf_y_param = "pcl_voxel_leaf_y";
    std::string pcl_voxel_leaf_z_param = "pcl_voxel_leaf_z";
    int pcl_radius_neighbors = 3;
    float pcl_radius_radius = 0.1;

    float scan_point_jump = 0.06;
    float scan_noise_angle = 0.06;

    std::string scan_point_jump_param = "scan_point_jump";
    std::string scan_noise_angle_param = "scan_noise_angle";

    std::string pcl_radius_neighbors_param = "pcl_radius_neighbors";
    std::string pcl_radius_radius_param = "pcl_radius_radiusSearch";

    float pcl_norm_radius = 0.8;
    std::string pcl_norm_radius_param = "pcl_norm_radius";


    float pcl_ref_voxel_leaf_x = 0.08;
    float pcl_ref_radius_radius = 0.1;
    int pcl_ref_radius_neighbors = 3;
    std::string pcl_ref_voxel_leaf_x_param = "pcl_ref_voxel_leaf_x";
    std::string pcl_ref_radius_radius_param = "pcl_ref_radius_radius";
    std::string pcl_ref_radius_neighbors_param = "pcl_ref_radius_neighbors";

    float pcl_icp_max_match_distance = 0.3;
    std::string pcl_icp_max_match_distance_param = "pcl_icp_max_match_distance";
    int pcl_icp_max_iter = 30;
    std::string pcl_icp_max_iter_param = "pcl_icp_max_iter";

    float no_move_translation = 5e-3;
    float no_move_rotation = 5e-3;

    float final_no_move_translation = 5e-3;
    float final_no_move_rotation = 5e-3;
    float no_move_ms = 500.0;


    std::string no_move_translation_param = "no_move_translation";
    std::string no_move_rotation_param = "no_move_rotation";
    std::string final_no_move_translation_param = "final_no_move_translation";
    std::string final_no_move_rotation_param = "final_no_move_rotation";
    std::string no_move_ms_param = "no_move_ms";


    nh_private.getParam(range_max_param, range_max);
    nh_private.getParam(range_min_param, range_min);
    nh_private.getParam(pcl_voxel_leaf_x_param, pcl_voxel_leaf_x);
    nh_private.getParam(pcl_voxel_leaf_y_param, pcl_voxel_leaf_y);
    nh_private.getParam(pcl_voxel_leaf_z_param, pcl_voxel_leaf_z);

    nh_private.getParam(pcl_radius_neighbors_param, pcl_radius_neighbors);
    nh_private.getParam(pcl_radius_radius_param, pcl_radius_radius);

    nh_private.getParam(scan_point_jump_param, scan_point_jump);
    nh_private.getParam(scan_noise_angle_param, scan_noise_angle);

    nh_private.getParam(pcl_norm_radius_param, pcl_norm_radius);

    nh_private.getParam(pcl_ref_voxel_leaf_x_param, pcl_ref_voxel_leaf_x);
    nh_private.getParam(pcl_ref_radius_radius_param, pcl_ref_radius_radius);
    nh_private.getParam(pcl_ref_radius_neighbors_param, pcl_ref_radius_neighbors);

    nh_private.getParam(pcl_icp_max_match_distance_param, pcl_icp_max_match_distance);

    nh_private.getParam(pcl_icp_max_iter_param, pcl_icp_max_iter);

    //
    nh_private.getParam(no_move_translation_param, no_move_translation);
    nh_private.getParam(no_move_rotation_param, no_move_rotation);
    nh_private.getParam(final_no_move_translation_param, final_no_move_translation);
    nh_private.getParam(final_no_move_rotation_param, final_no_move_rotation);
    nh_private.getParam(no_move_ms_param, no_move_ms);

    nh_private.getParam(file_dir_param, file_dir);
    nh_private.getParam(load_dir_param, load_dir);


    nh_private.getParam(mode_param, mode);

    nh_private.getParam(dump_data_param,dump_data);




    std::string cloud_filtered_topic = "/scan_matching/cloud_filtered";
    std::string cloud_matched_topic = "/scan_matching/cloud_matched";

    std::string cloud_reference_topic = "/scan_matching/cloud_reference";
    std::string cloud_norm_topic = "/scan_matching/cloud_norm";

    std::string icp_pose_topic = "/scan_matching/robot_pose";
    std::string initialpose_topic = "/initialpose";

    std::string pose_trj_topic = "/scan_matching/robot_pose_array";


    // ==== publisher
    // PointCloud2
    // tf: map_to_odom

    bool tf_compute_map_odom = false;
    ros::Publisher cloud_filtered_pub = nh.advertise<sensor_msgs::PointCloud2>(cloud_filtered_topic, 1);
    ros::Publisher cloud_reference_pub = nh.advertise<sensor_msgs::PointCloud2>(cloud_reference_topic, 1);
    ros::Publisher cloud_matched_pub = nh.advertise<sensor_msgs::PointCloud2>(cloud_matched_topic, 1);

    ros::Publisher icp_pose_pub = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>(icp_pose_topic, 1);
    ros::Publisher inital_pose_pub = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>(initialpose_topic, 1);

    ros::Publisher robot_pose_trj_pub = nh.advertise<geometry_msgs::PoseArray>(pose_trj_topic, 1);

    ros::Publisher cloud_norm_pub = nh.advertise<geometry_msgs::PoseArray>(cloud_norm_topic, 1);


    geometry_msgs::PoseArray robot_pose_array;
    robot_pose_array.header.frame_id = "map";


    geometry_msgs::PoseWithCovarianceStamped initialpose_msg;
    initialpose_msg.header.frame_id = "map";
    initialpose_msg.header.stamp = ros::Time::now();
//    initialpose_msg.pose.pose.position.x = json_process_data["instructionData"]["positionX"];
//    initialpose_msg.pose.pose.position.y = json_process_data["instructionData"]["positionY"];
//    initialpose_msg.pose.pose.position.z = 0.0; // json_process_data["instructionData"]["positionX"];
//    initialpose_msg.pose.pose.orientation.x = json_process_data["instructionData"]["orientationX"];
//    initialpose_msg.pose.pose.orientation.y =  json_process_data["instructionData"]["orientationY"];
//    initialpose_msg.pose.pose.orientation.z = json_process_data["instructionData"]["orientationZ"];
//    initialpose_msg.pose.pose.orientation.w = json_process_data["instructionData"]["orientationW"];

    initialpose_msg.pose.covariance.fill(0.0);

    initialpose_msg.pose.covariance[6 * 0 + 0] = 0.5 * 0.5;
    initialpose_msg.pose.covariance[6 * 1 + 1] = 0.5 * 0.5;
    initialpose_msg.pose.covariance[6 * 5 + 5] = M_PI / 12.0 * M_PI / 12.0;


    sensor_msgs::PointCloud2 cloud_filtered, cloud_reference, cloud_matched;
    geometry_msgs::PoseArray cloud_norm;


    cloud_norm.header.frame_id = "map";
    cloud_filtered.header.frame_id = "map";
    cloud_reference.header.frame_id = "map";
    cloud_matched.header.frame_id = "map";

    createPointCloud2(cloud_filtered, {"x", "y", "z"});
    createPointCloud2(cloud_reference, {"x", "y", "z"});
    createPointCloud2(cloud_matched, {"x", "y", "z"});


    //==== pcl cloud
    pcl::PointCloud<pcl::PointXYZ>::Ptr pcl_cloud_raw(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr pcl_cloud_voxel(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr pcl_cloud_radius(new pcl::PointCloud<pcl::PointXYZ>);
    pcl::PointCloud<pcl::PointXYZ>::Ptr pcl_cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>);

    pcl::PointCloud<pcl::PointNormal>::Ptr pcl_cloud_norm(new pcl::PointCloud<pcl::PointNormal>);
    pcl::PointCloud<pcl::PointNormal>::Ptr pcl_cloud_norm_ref(new pcl::PointCloud<pcl::PointNormal>);


    //==== pcl
    pcl::RadiusOutlierRemoval<pcl::PointXYZ> radiusOutlierRemoval;
    pcl::ApproximateVoxelGrid<pcl::PointXYZ> approximateVoxelGrid;

    approximateVoxelGrid.setLeafSize(pcl_voxel_leaf_x, pcl_voxel_leaf_y, pcl_voxel_leaf_z);

    radiusOutlierRemoval.setRadiusSearch(pcl_radius_radius);
    radiusOutlierRemoval.setMinNeighborsInRadius(pcl_radius_neighbors);

//    radiusOutlierRemoval.setKeepOrganized(false);


    // icp
    pcl::PointCloud<pcl::PointXYZ> Final;
    pcl::PointCloud<pcl::PointNormal> Final_Norm;

    Eigen::Matrix4f initial_guess(Eigen::Matrix4f::Identity ());
//    initial_guess(0,3) = 0.03;
//    initial_guess(1,3) = 0.02;


    pcl::IterativeClosestPointNonLinear<PointType, PointType> icp;

    pcl::registration::WarpPointRigid3D<PointType, PointType>::Ptr warp_fcn
            (new pcl::registration::WarpPointRigid3D<PointType, PointType>);



    // Create a TransformationEstimationLM object, and set the warp to it
    pcl::registration::TransformationEstimation2D<PointType, PointType>::Ptr te_2d(
            new pcl::registration::TransformationEstimation2D<PointType, PointType>);



    // TransformationEstimationLM with warp_fcn is faster than TransformationEstimation2D
    pcl::registration::TransformationEstimationLM<PointType, PointType>::Ptr te(
            new pcl::registration::TransformationEstimationLM<PointType, PointType>);
    te->setWarpFunction(warp_fcn);


    // Pass the TransformationEstimation objec to the ICP algorithm
    icp.setTransformationEstimation(te);



//    icp.setMaximumIterations (10);
//    icp.setMaxCorrespondenceDistance (0.05);
//    icp.setRANSACOutlierRejectionThreshold (0.05);


// pl icp
    pcl::registration::WarpPointRigid3D<pcl::PointNormal, pcl::PointNormal>::Ptr warp_fcn_pl
            (new pcl::registration::WarpPointRigid3D<pcl::PointNormal, pcl::PointNormal>);
    pcl::IterativeClosestPointWithNormals<pcl::PointNormal, pcl::PointNormal> pl_icp;

    pcl::registration::TransformationEstimationPointToPlane<pcl::PointNormal, pcl::PointNormal>::Ptr pl_te(
            new pcl::registration::TransformationEstimationPointToPlane<pcl::PointNormal, pcl::PointNormal>);

    pl_te->setWarpFunction(warp_fcn_pl);
    pl_icp.setTransformationEstimation(pl_te);



    // norm est
    pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);

    Normal2dEstimation norm_estim;
    norm_estim.setSearchMethod(tree);
    norm_estim.setRadiusSearch(pcl_norm_radius);
//    norm_estim.setKSearch(5);




// pcl tree
    float resolution = 0.05;



    pcl::octree::OctreePointCloudSearch<pcl::PointNormal> octree (resolution);



    // build the filter


    // ==== listener
    // LaserScan

    // tf: odom_to_base_link, base_link_to_base_laser


    bool scan_get_data = false;
    bool tf_get_base_laser = false;
    bool tf_get_odom_base = false;

    sensor::ScanToPoints scan_handler;

    scan_handler.scan_max_jump = scan_point_jump;
    scan_handler.scan_noise_angle = scan_noise_angle;


    tf::TransformListener tl_;
    tf::TransformBroadcaster tf_br;
    tf::StampedTransform transform;

    tf::StampedTransform pub_map_odom_tf;
    tf::StampedTransform pub_reference_origin_tf;

    bool start_pub_tf = false;

    ros::Duration transform_tolerance_;
    transform_tolerance_.fromSec(1.0);

    transform::Transform2d tf_base_laser;
    transform::Transform2d tf_odom_base;
    transform::Transform2d tf_map_base;


    std::string amcl_tf_broadcast = "/amcl/tf_broadcast";


    std::string base_frame = "base_link";
    std::string odom_frame = "odom";
    std::string laser_frame = "base_laser";
    std::string map_frame = "map";
    std::string reference_frame = "icp_origin";


    bool is_origin_compute = false;
    transform::Transform2d interactive_origin;
    ros_tool::InteractiveTf interactive_tf(map_frame, reference_frame);


    ros::Time scan_time;

    auto laser_cb = [&](const sensor_msgs::LaserScanConstPtr &msg) {
        laser_frame.assign(msg->header.frame_id);
        scan_time = msg->header.stamp;
        scan_get_data = true;


        scan_handler.getLocalPoints(msg->ranges, msg->angle_min, msg->angle_increment, range_min, range_max);


    };


    ros::Subscriber sub = nh.subscribe<sensor_msgs::LaserScan>("scan", 1, laser_cb);
    nh.setParam(amcl_tf_broadcast, true);


    transform::Transform2d tmp_icp(0.1, 0.05, 0.08);
    initial_guess(0, 0) = tmp_icp.matrix[0][0];
    initial_guess(0, 1) = tmp_icp.matrix[0][1];
    initial_guess(1, 0) = tmp_icp.matrix[1][0];
    initial_guess(1, 1) = tmp_icp.matrix[1][1];
    initial_guess(0, 3) = tmp_icp.matrix[0][2];
    initial_guess(1, 3) = tmp_icp.matrix[1][2];

//    tmp_icp = tmp_icp.inverse();


    PointCloudPoseSolver solver;

    auto &filter_reading_config = solver.getFilterReadingConfig();
    filter_reading_config.voxel_grid_size = pcl_voxel_leaf_x;
    filter_reading_config.radius_rmv_radius = pcl_radius_radius;
    filter_reading_config.radius_rmv_nn = pcl_radius_neighbors;

    auto &norm_est_config = solver.getNormEstConfig();
    norm_est_config.radius = pcl_norm_radius;

    auto &filter_reference_config = solver.getFilterReferenceConfig();
    filter_reference_config.voxel_grid_size = pcl_ref_voxel_leaf_x;
    filter_reference_config.radius_rmv_radius = pcl_ref_radius_radius;
    filter_reference_config.radius_rmv_nn = pcl_ref_radius_neighbors;

    auto &icp_config = solver.getIcpConfig();

    icp_config.max_match_distance = pcl_icp_max_match_distance;
    icp_config.max_iter = pcl_icp_max_iter;


    auto &move_check_config = solver.getMoveCheckConfig();
    move_check_config.final_no_move_rotation_epsilon = final_no_move_rotation;
    move_check_config.final_no_move_translation_epsilon = final_no_move_translation;
    move_check_config.no_move_rotation_epsilon = no_move_rotation;
    move_check_config.no_move_translation_epsilon = no_move_translation;
    move_check_config.no_move_check_ms = no_move_ms;

#if 0
    solver.m_filter_reading_config.voxel_grid_size = pcl_voxel_leaf_x;
    solver.m_filter_reading_config.radius_rmv_radius = pcl_radius_radius;
    solver.m_filter_reading_config.radius_rmv_nn = pcl_radius_neighbors;

    solver.m_norm_est_config.radius = pcl_norm_radius;

    solver.m_filter_reference_config.voxel_grid_size = pcl_ref_voxel_leaf_x;
    solver.m_filter_reference_config.radius_rmv_radius = pcl_ref_radius_radius;
    solver.m_filter_reference_config.radius_rmv_nn = pcl_ref_radius_neighbors;

    solver.m_icp_config.max_match_distance = pcl_icp_max_match_distance;
#endif


    solver.config();
    if(std::strcmp(mode.c_str(), MODE_LOC ) == 0){

        int rt = solver.loadFromFile(file_dir, load_dir);

        if(rt == -1){
            std::cerr << "solver load file error, exit" << std::endl;

            return 0;
        }
    }


    nlohmann::json origin_json;
    transform::Transform2d load_origin_tf;

    interactive_tf.setCallBack([&](auto &pose) {
        if (solver.isOriginCompute()) {
            float yaw;
            to_yaw(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w, yaw);
            interactive_origin.set(pose.position.x, pose.position.y, yaw);
            solver.setOrigin(interactive_origin);
        }

    });

    tf::Transform temp_transform;
    tf::Quaternion temp_q;


    bool get_odom_base_tf = false;
    bool get_map_base_tf = false;


    common::TaskManager task_manager;
    common::TaskManager scan_task_manager;

    bool is_new_pose_compute = false;

    task_manager.addTask([&] {

        auto solver_cloud_reading = solver.getCloudReading();
        if(!solver_cloud_reading->empty()  ){
            PclPointCloudToPointCloud2(solver_cloud_reading, cloud_filtered);
            cloud_filtered.header.stamp = scan_time;
            cloud_filtered.header.frame_id.assign(laser_frame);
            cloud_filtered_pub.publish(cloud_filtered);

            PclPointCloudToPoseArray(solver_cloud_reading, cloud_norm);
            cloud_norm.header.stamp = scan_time;
            cloud_norm.header.frame_id.assign(laser_frame);
            cloud_norm_pub.publish(cloud_norm);

        }




        auto solver_cloud_matched = solver.getCloudMatched();
        if(!solver_cloud_matched->empty()){

            PclPointCloudToPointCloud2(solver_cloud_matched, cloud_matched);
            cloud_matched.header.stamp = scan_time;
            cloud_matched.header.frame_id.assign(reference_frame);
            cloud_matched_pub.publish(cloud_matched);
        }


        auto solver_cloud_reference = solver.getCloudReference();
        if(!solver_cloud_reference->empty()){

            PclPointCloudToPointCloud2(solver_cloud_reference, cloud_reference);
            cloud_reference.header.stamp = ros::Time::now() ;
            cloud_reference.header.frame_id.assign(reference_frame);
            cloud_reference_pub.publish(cloud_reference);

        }

        auto &map_cloud_origin_tf = solver.getOrigin();



        if (solver.isOriginCompute() && !is_origin_compute) {
            is_origin_compute = true;
            interactive_tf.setInitPose(map_cloud_origin_tf.x(), map_cloud_origin_tf.y(),
                                       map_cloud_origin_tf.yaw());
            interactive_tf.start(1.0, false);
            robot_pose_array.poses.clear();
        }

        if(is_origin_compute){
            temp_transform.setOrigin(tf::Vector3(map_cloud_origin_tf.x(), map_cloud_origin_tf.y(), 0.0));
            temp_q.setRPY(0, 0, map_cloud_origin_tf.yaw());
            temp_transform.setRotation(temp_q);

            pub_reference_origin_tf = tf::StampedTransform(temp_transform,
                                                           ros::Time::now() +
                                                           transform_tolerance_,
                                                           map_frame, reference_frame);

            tf_br.sendTransform(pub_reference_origin_tf);



            if(is_new_pose_compute){
                is_new_pose_compute = false;
                if (robot_pose_array.poses.empty()) {
                    robot_pose_array.poses.push_back(initialpose_msg.pose.pose);
                } else {
                    float yaw, yaw_last;
                    to_yaw(robot_pose_array.poses.back().orientation.x, robot_pose_array.poses.back().orientation.y,
                           robot_pose_array.poses.back().orientation.z, robot_pose_array.poses.back().orientation.w, yaw_last);
                    to_yaw(initialpose_msg.pose.pose.orientation.x, initialpose_msg.pose.pose.orientation.y,
                           initialpose_msg.pose.pose.orientation.z, initialpose_msg.pose.pose.orientation.w, yaw);

                    if (std::abs(initialpose_msg.pose.pose.position.x - robot_pose_array.poses.back().position.x) > 0.03
                        ||
                        std::abs(initialpose_msg.pose.pose.position.y - robot_pose_array.poses.back().position.y) > 0.03
                        || std::abs(yaw_last - yaw) > 0.01
                            ) {
                        robot_pose_array.poses.push_back(initialpose_msg.pose.pose);
                    }
                }

                if (robot_pose_array.poses.size() > 200) {
                    robot_pose_array.poses.erase(robot_pose_array.poses.begin(), robot_pose_array.poses.begin() + 100);
                }
                robot_pose_array.header.stamp = ros::Time::now();
                if(!robot_pose_array.poses.empty()){
                    robot_pose_trj_pub.publish(robot_pose_array);

                }
            }



        }







        return true;
    }, 200.0);



    scan_task_manager.addTask([&]{





        return true;
        }, 5.0);

    while (ros::ok()) {
        ros::spinOnce();

        if (!tf_get_base_laser && scan_get_data) {
            try {
                tl_.lookupTransform(base_frame, laser_frame, ros::Time(0), transform);
                tf_get_base_laser = true;
                tf_base_laser.set(transform.getOrigin().x(), transform.getOrigin().y(),
                                  tf::getYaw(transform.getRotation()));

                solver.setSensorRelativePose(tf_base_laser);
            } catch (tf::TransformException &ex) {
                ROS_ERROR("%s", ex.what());
                continue;
            }
        }

        if (scan_get_data) {


            // 1.0 process sequence
            // 1.1 filter input(map&loc)
            // 1.2 match(map&loc)
            // 1.3 look up odom_base(map&loc)
            // 1.4 update solver odom_base and map(map)
            // 1.5 compute map_odom(loc)





            // LaserScan to PointCloud

//            LaserScanToPointCloud2(scan_handler.local_xy_points,scan_handler.range_valid_num,cloud_filtered);





            common::Time t1 = common::FromUnixNow();

            LaserScanToPclPointCloud(scan_handler.local_xy_points, scan_handler.range_valid_num, pcl_cloud_raw);


            solver.addSensorReading(pcl_cloud_raw);
            if(std::strcmp(mode.c_str(), MODE_MAP) ==0 ){
                solver.match_v2();

            }

            std::cout << "preprocess time : " << common::ToMillSeconds(common::FromUnixNow() - t1) << " ms"
                      << std::endl;


            get_odom_base_tf = false;
            get_map_base_tf = false;
            // lookup odom base_link
            try {
                tl_.waitForTransform(odom_frame, base_frame, scan_time, ros::Duration(0.05));

                tl_.lookupTransform(odom_frame, base_frame, scan_time, transform);
                tf_get_odom_base = true;
                tf_odom_base.set(transform.getOrigin().x(), transform.getOrigin().y(),
                                 tf::getYaw(transform.getRotation()));
                get_odom_base_tf = true;

            } catch (tf::TransformException &ex) {
                ROS_ERROR("%s", ex.what());
            }

            try {
                tl_.waitForTransform(map_frame, base_frame, scan_time, ros::Duration(0.05));

                tl_.lookupTransform(map_frame, base_frame, scan_time, transform);
                tf_map_base.set(transform.getOrigin().x(), transform.getOrigin().y(),
                                tf::getYaw(transform.getRotation()));
                get_map_base_tf = true;

            } catch (tf::TransformException &ex) {
                ROS_ERROR("%s", ex.what());
            }


            if (  std::strcmp(mode.c_str(), MODE_MAP) ==0  && get_odom_base_tf) {
                solver.matchUpdate(tf_odom_base);
            }

            if (std::strcmp(mode.c_str(), MODE_MAP) ==0  &&  get_map_base_tf) {
                solver.setRobotAbsolutePoseOrigin(tf_map_base);
//                start_pub_tf = (  std::strcmp(mode.c_str(), MODE_LOC) ==0   ) ;

            }
            if (std::strcmp(mode.c_str(), MODE_LOC) ==0  &&  get_map_base_tf) {
                solver.setRobotAbsolutePoseInitIcp(tf_map_base);
                start_pub_tf = (  std::strcmp(mode.c_str(), MODE_LOC) ==0   ) ;
            }

            if(std::strcmp(mode.c_str(), MODE_LOC) ==0 ){
                solver.match_v2();

            }

            {
                // get map odom tf and publish
                auto &map_odom_tf = solver.solveMapOdom(tf_odom_base);

                std::cout << "map_odom_tf:\n" << map_odom_tf << std::endl;

                if (start_pub_tf) {
                    temp_transform.setOrigin(tf::Vector3(map_odom_tf.x(), map_odom_tf.y(), 0.0));
                    temp_q.setRPY(0, 0, map_odom_tf.yaw());
                    temp_transform.setRotation(temp_q);

                    pub_map_odom_tf = tf::StampedTransform(temp_transform,
                                                           ros::Time::now() +
                                                           transform_tolerance_,
                                                           map_frame, odom_frame);

//                pub_map_odom_tf.stamp_ = ros::Time::now() + transform_tolerance_;
                    tf_br.sendTransform(pub_map_odom_tf);

                    nh.setParam(amcl_tf_broadcast, false);

                }


                auto &solved_pose = solver.getRobotAbsolutePose();
                initialpose_msg.pose.pose.position.x = solved_pose.x();
                initialpose_msg.pose.pose.position.y = solved_pose.y();

                tf::quaternionTFToMsg(tf::createQuaternionFromYaw(solved_pose.yaw()),
                                      initialpose_msg.pose.pose.orientation);
                initialpose_msg.header.stamp = scan_time;

                icp_pose_pub.publish(initialpose_msg);

                is_new_pose_compute = true;


            }



#if 0
            //            LaserScanToPclPointCloud(scan_handler.local_xy_points,scan_handler.range_valid_num, pcl_cloud_norm);



            approximateVoxelGrid.setInputCloud(pcl_cloud_raw);
            approximateVoxelGrid.filter(*pcl_cloud_voxel);
            radiusOutlierRemoval.setInputCloud(pcl_cloud_voxel);
            radiusOutlierRemoval.filter(*pcl_cloud_radius);

            PclPointCloudToPointCloud2(pcl_cloud_radius, cloud_filtered);

            tmp_icp.mul(scan_handler.local_xy_points, scan_handler.global_xy_points);
            LaserScanToPclPointCloud(scan_handler.global_xy_points, scan_handler.range_valid_num, pcl_cloud_voxel);

//            pcl_cloud_voxel->is_dense = true;
//            pcl_cloud_radius->is_dense = true;

            // Target =    initial_guess*Source
            // map = pose * reading
            icp.setInputSource(pcl_cloud_radius);
            icp.setInputTarget(pcl_cloud_voxel);
            icp.align(Final, initial_guess);
            std::cout << "icp has converged:" << icp.hasConverged() << " score: " <<
                      icp.getFitnessScore() << std::endl;
            std::cout << icp.getFinalTransformation() << std::endl;
            std::cout << "tmp_icp: " << tmp_icp << std::endl;






//            pcl::transformPointCloud (*data, *tmp, initial_guess);


            pcl::copyPointCloud(*pcl_cloud_radius, *pcl_cloud_norm);
            norm_estim.setInputCloud(pcl_cloud_radius);
            norm_estim.compute(pcl_cloud_norm);
            pcl::copyPointCloud(*pcl_cloud_voxel, *pcl_cloud_norm_ref);

            norm_estim.setInputCloud(pcl_cloud_voxel);
            norm_estim.compute(pcl_cloud_norm_ref);

            pl_icp.setInputSource(pcl_cloud_norm);
            pl_icp.setInputTarget(pcl_cloud_norm_ref);
            pl_icp.align(Final_Norm, initial_guess);
            std::cout << "pl_icp has converged:" << pl_icp.hasConverged() << " score: " <<
                      pl_icp.getFitnessScore() << std::endl;
            std::cout << pl_icp.getFinalTransformation() << std::endl;


            std::cout << "preprocess time : " << common::ToMillSeconds(common::FromUnixNow() - t1) << " ms"
                      << std::endl;


            cloud_filtered.header.stamp = scan_time;
            cloud_filtered.header.frame_id.assign(laser_frame);
            cloud_filtered_pub.publish(cloud_filtered);
#endif


#if 0
            cloud_norm.poses.resize(pcl_cloud_norm->points.size());

            float yaw = 0.0;
//            std::cout << "\nshow all yaw:\n";
            for (int i = 0; i < cloud_norm.poses.size(); i++) {
                cloud_norm.poses[i].position.x = pcl_cloud_norm->points[i].x;
                cloud_norm.poses[i].position.y = pcl_cloud_norm->points[i].y;
                cloud_norm.poses[i].position.z = pcl_cloud_norm->points[i].z;

                cloud_norm.poses[i].orientation.x = 0.0;
                cloud_norm.poses[i].orientation.y = 0.0;
                cloud_norm.poses[i].orientation.z = 0.0;
                cloud_norm.poses[i].orientation.w = 1.0;


                yaw = atan2(pcl_cloud_norm->points[i].normal_y, pcl_cloud_norm->points[i].normal_x);
//                std::cout << "[" << yaw << ", " << pcl_cloud_norm->points[i].normal_y << ", " << pcl_cloud_norm->points[i].normal_x << "], ";
                yaw_to_quaternion(yaw, cloud_norm.poses[i].orientation.x, cloud_norm.poses[i].orientation.y,
                                  cloud_norm.poses[i].orientation.z, cloud_norm.poses[i].orientation.w);

            }
//            std::cout << "end all yaw:\n" << std::endl;


            cloud_norm.header.stamp = scan_time;

            cloud_norm.header.frame_id.assign(laser_frame);

            cloud_norm_pub.publish(cloud_norm);
#endif

            scan_get_data = false;
        } else {

            task_manager.call();



            if (start_pub_tf) {

                pub_map_odom_tf.stamp_ = ros::Time::now() + transform_tolerance_;
//                tf_br.sendTransform(pub_map_odom_tf);
            }
        }


    }
    nh.setParam(amcl_tf_broadcast, true);

    if(std::strcmp(mode.c_str(), MODE_MAP) ==0 || ((std::strcmp(mode.c_str(), MODE_LOC) ==0) && dump_data) ){
        solver.dumpToFile(file_dir);
    }
}