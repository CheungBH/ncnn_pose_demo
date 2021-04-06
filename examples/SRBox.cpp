#include "SRBox.h"


      SRBox::SRBox(std::pair<double, double> loc, double h, double w){  // w and h are the width and height of each single box in pixel , pair --> x , y coord in unit of (0,0) , (0,1) , ... , (10,10)

          location = loc;
          height = h; 
          width = w;
		  center = { (loc.first + 0.5)*w, (loc.second + 0.5)*h };
		  xmin = loc.first * w, ymin = loc.second * h, xmax = (loc.first + 1)*w ,ymax = (loc.second + 1)*h;
		  exist = 0;
		  disappear = 0;
		  disappear_max = 20;
      }

	  void SRBox::clear() 
	  {
		  exist = 0;
		  disappear = 0;

	  }

	  void SRBox::update(int flag) {
		  if (flag == 0) {
			  ;
		  }
		  else if(flag == 1) { 
			  SRBox::update_exist(flag);
		  }
		  else if (flag == 2) {
			  SRBox::update_exist(flag);
		  }
		  else if (flag == -1) {

			  SRBox::update_disappear();
			  SRBox::update_exist(flag);
		  }

	  }

	  void SRBox::update_exist(int flag)
	  {
		  if (flag == 1) {
			  if (exist < exist_max) {
				  exist += 1;
			  }
		  }
		  else if (flag == -1) {
			  if (exist > 0) {
				  exist -= 1;
			  }
		  }
		  //What happen to flag == 2, is the value shld be kept unchanged?

	  }

      void SRBox::update_disappear()
      {
		  if (disappear < disappear_max) {
			  disappear += 1;
		  }
		  else {
			  SRBox::clear();
		  }

      }

	  bool SRBox::if_warning() 
	  {
		  return (exist > alarm_cnt);
	  }

	  void SRBox::visualize_srbox(cv::Mat image , cv::Scalar color)
	  {
		  cv::Rect rect(xmin, ymin, width, height);
		  cv::rectangle(image, rect, color);

	  }





      


