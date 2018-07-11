// read image that is older
//publish image and timestamp
//delete image?


///from answers.ros

#include <ros/ros.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost::filesystem;


int main(int argc, char** argv)
{
  ros::init(argc, argv, "image_publisher");
  ros::NodeHandle nh("~");
  std::string out_dir;
  float readrate;
  nh.getParam("out_dir", out_dir);
    nh.getParam("readrate", readrate);
  ROS_INFO("Extracting images from directory %s", out_dir.c_str());
  path p(out_dir);

  cv_bridge::CvImage cv_image;
  sensor_msgs::Image ros_image;
  ros::Rate loop_rate(readrate);
  ros::Publisher pub = nh.advertise<sensor_msgs::Image>("/static_image", 1);

  while (ros::ok())
  {
    ros::spinOnce();
    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
      auto c = i->path();
      auto pp = c.parent_path();
      auto b = pp/(i->path().stem().string()+".ini");
      if (!is_directory(i->path())&& !(i->path().extension().string().compare(".png"))&&boost::filesystem::exists(b)) //we eliminate directories
      {
        //cout << i->path().filename().string() << " " <<  i->path().extension().string() << endl;
        //imagecounter++;
        cv_image.image = cv::imread(i->path().filename().string(),CV_LOAD_IMAGE_COLOR);
        cv_image.encoding = "bgr8";

        cv_image.toImageMsg(ros_image);
        // removing the file

        //auto a = pp/(i->path().filename().string());
        auto a = pp/(i->path().stem().string()+".png");
        //auto b = pp/(i->path().stem().string()+".ini");
        //ROS_INFO("just path: %s", c.string().c_str());
        ROS_INFO("prefix: %s", pp.string().c_str());
        ROS_INFO("STUFF I WILL REMOVE: %s \n %s", a.c_str(),b.c_str());
        try
        {
          boost::filesystem::remove(b);
          boost::filesystem::remove(a);

        }
        catch (filesystem_error &e)
        {
          std::string errorstring = e.what();
          ROS_ERROR("%s", errorstring.c_str());
        }

        pub.publish(ros_image);
        break; ///once I found an image to display I will do it.
      }
      else
      continue;
    }





  }
}
