// publishes oldest image from out_dir in topic ???/dout
// deletes image and ini after it is read

#include <ros/ros.h>
#include <image_transport/image_transport.h>
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
  image_transport::ImageTransport it(nh);
  std::string out_dir;
  float readrate;
  std::string imagenameprefix;
  nh.getParam("out_dir", out_dir);
  nh.getParam("readrate", readrate);
  nh.getParam("imagenameprefix", imagenameprefix);
  ROS_INFO("Extracting images from directory %s", out_dir.c_str());
  ROS_INFO("Read rate is %f Hz.", readrate);
  ROS_INFO("Image name prefix is %s", imagenameprefix.c_str());

  path p(out_dir);
  path oldestpng;
  int oldestcounter = 9999; // should maybe read from image%04d definition instead of doing this. well, future improvement if necessary

  cv_bridge::CvImage cv_image;
  sensor_msgs::Image ros_image;
  ros::Rate loop_rate(readrate);
  image_transport::Publisher pub = it.advertise("dout", 1);

  while (ros::ok())
  {
    ros::spinOnce();
    if (directory_iterator(p)==directory_iterator())
    {
      //directory is empty! so sleep time...
      ROS_INFO_THROTTLE(60,"directory is empty. maybe lower outimagepublisher refresh rate?");
    }
    else
    {
      for (auto i = directory_iterator(p); i != directory_iterator(); i++)
      {
        path c = i->path();
        bool ispng = !(c.extension().string().compare(".png"));
        ROS_DEBUG("ispng %d",ispng);
        if (!is_directory(c)&& ispng) //we eliminate directories
        {
          std::string imagecounterstr = c.stem().string().substr (imagenameprefix.size(),4); // the number of digits here should be a parameter, hey?
          int imagecounter = std::stoi (imagecounterstr);
          if(imagecounter<oldestcounter)
          {
            oldestcounter = imagecounter;
            oldestpng = c;
            ROS_DEBUG("oldestpng %s",oldestpng.string().c_str());
          }

        }
        else
        continue;
      }

      std::string imagefile = oldestpng.string();
      ROS_DEBUG("reading image file: %s", imagefile.c_str());
      cv::Mat image = cv::imread(imagefile, CV_LOAD_IMAGE_COLOR);
      if (image.empty())
      {
        //ROS_ERROR("NO IMAGE!!!!");
        ROS_DEBUG("NO IMAGE!!!!");
      }
      sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", image).toImageMsg();
      pub.publish(msg);
      ROS_DEBUG("published image message");

      // removing the file
      ROS_DEBUG("prefix: %s", p.string().c_str());
      ROS_DEBUG("STUFF I WILL REMOVE: %s", oldestpng.c_str());
      try
      {
        boost::filesystem::remove(oldestpng);
      }
      catch (filesystem_error &e)
      {
        std::string errorstring = e.what();
        ROS_ERROR("%s", errorstring.c_str());
      }
      oldestcounter = 9999;
    }
    loop_rate.sleep();
  }
}
