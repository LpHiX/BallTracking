#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

cv::VideoCapture cap(0);
cv::Mat3b img;
cv::Mat threshold_img;
std::vector<std::vector<cv::Point>> contours;
std::vector<cv::Vec4i> hierarchy;
const int max = 255;
int low_r = 68, low_g = 111, low_b = 215;
int max_r = max, max_g = max, max_b = max;


int main()
{
	cv::namedWindow("output", cv::WINDOW_AUTOSIZE);
	cv::createTrackbar("Low R", "output", &low_r, max);
	cv::createTrackbar("Max R", "output", &max_r, max);
	cv::createTrackbar("Low G", "output", &low_g, max);
	cv::createTrackbar("Max G", "output", &max_g, max);
	cv::createTrackbar("Low B", "output", &low_b, max);
	cv::createTrackbar("Max B", "output", &max_b, max);
	while (true)
	{
		cap.read(img);
		if (!img.empty())
		{
			cv::inRange(img, cv::Scalar(low_b, low_g, low_r), cv::Scalar(max_b, max_g, max_r), threshold_img);
			cv::findContours(threshold_img, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
			size_t max_int = 0;
			for (size_t i = 0; i< contours.size(); i++)
			{
				max_int = cv::contourArea(contours[i]) >= cv::contourArea(contours[max_int]) ? i : max_int;
			}
			if (contours.size() > 0)
			{
				cv::Point2f sphere_center;
				float sphere_radius;
				cv::minEnclosingCircle(cssontours[max_int], sphere_center, sphere_radius);
				cv::circle(img, sphere_center, sphere_radius, cv::Scalar(0, 255, 0), 2);
				cv::circle(threshold_img, sphere_center, sphere_radius, cv::Scalar(0, 255, 0), 2);
				std::cout << sphere_center.x << " " << sphere_center.y << " " << sphere_radius << std::endl;
			}
			cv::imshow("input", img);
			cv::imshow("output", threshold_img);
		}
		//cv::cvtColor(img, threshold_img, cv::COLOR_BGR2HSV);
		char key = (char)cv::waitKey(1);
		if (key == 27) break;
	}
}