export ROS_ROOT=~/ros/ros
export PATH=$ROS_ROOT/bin:$PATH
export PYTHONPATH=$ROS_ROOT/core/roslib/src:$PYTHONPATH
if [ ! "$ROS_MASTER_URI" ] ; then export ROS_MASTER_URI=http://localhost:11311 ; fi
export ROS_PACKAGE_PATH=~/develop/omniclops/ros:~/ros/pkgs

source $ROS_ROOT/tools/rosbash/rosbash
