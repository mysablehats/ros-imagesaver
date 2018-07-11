#include "ros/ros.h"
#include <std_srvs/Empty.h>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost::filesystem;

int main(int argc, char **argv)
{
  ros::init(argc, argv, "imagesavetrigger");
  ros::NodeHandle n("~");
  ros::ServiceClient client = n.serviceClient<std_srvs::Empty>("/imagesavernode/save");
  //get directory parameters
  std::string save_dir;
  int max_images;
  n.getParam("save_dir", save_dir);
  ROS_INFO("Saving to directory %s", save_dir.c_str());
  n.getParam("max_images", max_images);
  ROS_INFO("Maximum number of images %i", max_images);
  std_srvs::Empty srv;

  //std::string path = "/path/to/directory";
  //for (auto & p : fs::directory_iterator(path))

  path p(save_dir);

  int imagecounter;
  ros::Rate loop_rate(1);

  while (ros::ok())
  {
    ros::spinOnce();
    //slightly altered copypaste from stackexchange which is a copypaste from boosts docs
    imagecounter = 0;
    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
        if (!is_directory(i->path())&& !(i->path().extension().string().compare(".png"))) //we eliminate directories
        {
            //cout << i->path().filename().string() << " " <<  i->path().extension().string() << endl;
            imagecounter++;
        }
        else
            continue;
    }
    ROS_DEBUG_THROTTLE(60,"Number of images %i",imagecounter);
    //if there are too many images dont trigger it anymore
    if (imagecounter<max_images)
    {
      client.call(srv);
      ROS_DEBUG("Saving image stuff, because of reasons.");
      ROS_INFO_ONCE("First image seems to be saved alright.");
    }
    else
    {
      ROS_WARN_ONCE("Image cap of %i reached. not saving anymore. Is outimagepublisher running?",max_images);
    }

  }


  return 0;
}
