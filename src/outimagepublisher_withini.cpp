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
  nh.getParam("out_dir", out_dir);
  nh.getParam("readrate", readrate);
  ROS_INFO("Extracting images from directory %s", out_dir.c_str());
  ROS_INFO("Read rate is %f Hz.", readrate);
  path p(out_dir);
  //path pp = p.parent_path();
  path oldestpng;
  path oldestini;

  cv_bridge::CvImage cv_image;
  sensor_msgs::Image ros_image;
  ros::Rate loop_rate(readrate);
  //ros::Publisher pub = nh.advertise<sensor_msgs::Image>("/static_image", 1);
  image_transport::Publisher pub = it.advertise("dout", 1);

  while (ros::ok())
  {
    ros::spinOnce();
    std::time_t t = std::time(nullptr);
    std::localtime(&t);
    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
      path c = i->path();
      std::time_t clastwritetime = boost::filesystem::last_write_time(c);
      path stempathstr = p/c.stem();
      path a(stempathstr.string()+".png");
      path b(stempathstr.string()+".ini");
      ROS_DEBUG("a %s",a.string().c_str());
      ROS_DEBUG("b %s",b.string().c_str());
      bool ispng = !(c.extension().string().compare(".png"));
      bool doesiniexist = boost::filesystem::exists(b);
      ROS_DEBUG("ispng %d",ispng);
      ROS_DEBUG("doesiniexist %d",doesiniexist);

      if (!is_directory(c)&& ispng &&doesiniexist) //we eliminate directories
      {
            ROS_DEBUG("t %ld",t);
            ROS_DEBUG("clastwritetime %ld",clastwritetime);
            if(t>clastwritetime)
            {
              t = clastwritetime;
              oldestpng = a;
              oldestini = b;
              ROS_DEBUG("oldestpng %s",a.string().c_str());
              ROS_DEBUG("ini from oldestpng %s",b.string().c_str());
              //ROS_DEBUG("REACHED!!!!");
            }

      }
      else
      continue;
    }
    //cout << i->path().filename().string() << " " <<  i->path().extension().string() << endl;
    //imagecounter++;
    //cv_image.image = cv::imread(i->path().filename().string(),CV_LOAD_IMAGE_COLOR);
    //cv_image.encoding = "bgr8";
    //cv_image.toImageMsg(ros_image);
    //pub.publish(ros_image);
    std::string imagefile = oldestpng.string();
    ROS_DEBUG("reading image file: %s", imagefile.c_str());
    cv::Mat image = cv::imread(imagefile, CV_LOAD_IMAGE_COLOR);
    if (image.empty())
    {
      ROS_ERROR("NO IMAGE!!!!");
    }
    sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", image).toImageMsg();
    pub.publish(msg);
    ROS_DEBUG("published image message");
    // removing the file

    //auto a = pp/(i->path().filename().string());

    //auto b = pp/(i->path().stem().string()+".ini");
    //ROS_INFO("just path: %s", c.string().c_str());
    ROS_DEBUG("prefix: %s", p.string().c_str());
    ROS_DEBUG("STUFF I WILL REMOVE: %s %s", oldestpng.c_str(),oldestini.c_str());
    try
    {
      boost::filesystem::remove(oldestpng);
      boost::filesystem::remove(oldestini);

    }
    catch (filesystem_error &e)
    {
      std::string errorstring = e.what();
      ROS_ERROR("%s", errorstring.c_str());
    }

    loop_rate.sleep();




  }
}
