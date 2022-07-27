#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

struct Vector3
{
	float x, y, z;
	Vector3()
	{
		x, y, z = 0;
	}
	Vector3(const float& x, const float& y, const float& z)
		: x(x), y(y), z(z) {}
	Vector3 Add(const Vector3& other) const
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	Vector3 operator + (const Vector3& other) const
	{
		return Add(other);
	}
	Vector3 Subtract(const Vector3& other) const
	{
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	Vector3 operator- (const Vector3& other) const
	{
		return Subtract(other);
	}
	Vector3 Multiply(const float& a) const
	{
		return Vector3(x * a, y * a, z * a);
	}
	Vector3 Multiply(const Vector3& other) const
	{
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
 	Vector3 operator * (const float& a) const
	{
		return Multiply(a);
	}
	Vector3 operator * (const Vector3& other) const
	{
		return Multiply(other);
	}
	float MagnitudeSquared() const
	{
		return x * x + y * y + z * z;
	}
	Vector3 RotateX(float radians) const
	{
		return Vector3(x, y * cosf(radians) - z * sinf(radians), y * sinf(radians) + z * cosf(radians));
	}
};

std::ostream& operator<<(std::ostream& stream, const Vector3& other)
{
	stream << other.x << ", " << other.y << ", " << other.z;
	return stream;
}

cv::VideoCapture cap(0);
cv::Mat3b img;
cv::Mat threshold_img;
std::vector<std::vector<cv::Point>> contours;
std::vector<cv::Vec4i> hierarchy;

const int max = 255;
int low_r = 0, low_g = 90, low_b = 144;
int max_r = 113, max_g = 255, max_b = 255;
int low_rs = 0, low_gs = 90, low_bs = 144;
int max_rs = 113, max_gs = 255, max_bs = 255;

int cal_value = 100;

cv::Mat3b canvas(180, 300, cv::Vec3b(0, 255, 0));
cv::Rect cal_x(0, 0, 150, 60);
cv::Rect cal_y(0, 60, 150, 60);
cv::Rect cal_z(0, 120, 150, 60);
cv::Rect cal_0(150, 60, 150, 60);
cv::Rect cal_debug(150, 0, 150, 60);

float distance, x_angle;
Vector3 pos_raw, pos_centered, pos_scaled, pos_out;
Vector3 pos_offset(298.338, 274.38, 1153.29), pos_scale(0.697858, -0.725195, -0.380886);
Vector3 rawpos_0(298.338, 274.38, 1153.29), rawpos_x(441.633, 277.5, 1285.44), rawpos_y(308.845, 136.486, 1007.07), rawpos_z(284.182, 351, 890.746);

bool online = true;

static void on_low_r(int value, void*) { low_r = value; }
static void on_max_r(int value, void*) { max_r = value; }
static void on_low_g(int value, void*) { low_g = value; }
static void on_max_g(int value, void*) { max_g = value; }
static void on_low_b(int value, void*) { low_b = value; }
static void on_max_b(int value, void*) { max_b = value; }

void mouseClickFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		if (cal_0.contains(cv::Point(x, y)))
		{
			rectangle(canvas, cal_0, cv::Scalar(0, 255, 0), 2);
			rawpos_0 = pos_raw;
			pos_offset = pos_raw;
		}
		if (cal_x.contains(cv::Point(x, y)))
		{
			rectangle(canvas, cal_x, cv::Scalar(0, 255, 0), 2);
			rawpos_x = pos_raw = pos_raw;
			pos_scale.x = cal_value / pos_centered.x;
		}
		if (cal_y.contains(cv::Point(x, y)))
		{
			rectangle(canvas, cal_y, cv::Scalar(0, 255, 0), 2);
			rawpos_y = pos_raw = pos_raw;
			pos_scale.y = cal_value / pos_centered.y;
		}
		if (cal_z.contains(cv::Point(x, y)))
		{
			rectangle(canvas, cal_z, cv::Scalar(0, 255, 0), 2);
			rawpos_z = pos_raw = pos_raw;
			pos_scale.z = cal_value / pos_centered.z;
		}
		if (cal_debug.contains(cv::Point(x, y)))
		{
			rectangle(canvas, cal_debug, cv::Scalar(0, 255, 0), 2);
			std::cout << "Vector3 pos_offset(" << pos_offset << "), pos_scale(" << pos_scale << ");" << std::endl;
			std::cout << "Vector3 rawpos_0(" << rawpos_0 << "), rawpos_x(" << rawpos_x << "), rawpos_y(" << rawpos_y << "), rawpos_z(" << rawpos_z << ");" << std::endl;
		}
	}
	if (event == cv::EVENT_LBUTTONUP)
	{
		rectangle(canvas, cal_0, cv::Scalar(100, 100, 100), 2);
		rectangle(canvas, cal_x, cv::Scalar(100, 100, 100), 2);
		rectangle(canvas, cal_y, cv::Scalar(100, 100, 100), 2);
		rectangle(canvas, cal_z, cv::Scalar(100, 100, 100), 2);
		rectangle(canvas, cal_debug, cv::Scalar(100, 100, 100), 2);
	}
	cv::imshow("Control Panel", canvas);
}

void setupControlPanel()
{
	canvas(cal_x) = cv::Vec3b(100, 100, 200);
	canvas(cal_y) = cv::Vec3b(100, 200, 100);
	canvas(cal_z) = cv::Vec3b(200, 100, 100);
	canvas(cal_0) = cv::Vec3b(200, 200, 100);
	canvas(cal_debug) = cv::Vec3b(100, 100, 100);
	cv::putText(canvas(cal_x), "Cal X", cv::Point(30, 45), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));
	cv::putText(canvas(cal_y), "Cal Y", cv::Point(30, 45), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));
	cv::putText(canvas(cal_z), "Cal Z", cv::Point(30, 45), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));
	cv::putText(canvas(cal_0), "Cal 0", cv::Point(30, 45), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));
	cv::putText(canvas(cal_debug), "Debug", cv::Point(30, 45), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));
	rectangle(canvas, cal_0, cv::Scalar(100, 100, 100), 2);
	rectangle(canvas, cal_x, cv::Scalar(100, 100, 100), 2);
	rectangle(canvas, cal_y, cv::Scalar(100, 100, 100), 2);
	rectangle(canvas, cal_y, cv::Scalar(100, 100, 100), 2);
	rectangle(canvas, cal_z, cv::Scalar(100, 100, 100), 2);
	cv::imshow("Control Panel", canvas);
	cv::setMouseCallback("Control Panel", mouseClickFunc);

	cv::createTrackbar("Low R", "Control Panel", nullptr, max, on_low_r);
	cv::createTrackbar("Max R", "Control Panel", nullptr, max, on_max_r);
	cv::createTrackbar("Low G", "Control Panel", nullptr, max, on_low_g);
	cv::createTrackbar("Max G", "Control Panel", nullptr, max, on_max_g);
	cv::createTrackbar("Low B", "Control Panel", nullptr, max, on_low_b);
	cv::createTrackbar("Max B", "Control Panel", nullptr, max, on_max_b);
	cv::setTrackbarPos("Low R", "Control Panel", low_r);
	cv::setTrackbarPos("Max R", "Control Panel", max_r);
	cv::setTrackbarPos("Low G", "Control Panel", low_g);
	cv::setTrackbarPos("Max G", "Control Panel", max_g);
	cv::setTrackbarPos("Low B", "Control Panel", low_b);
	cv::setTrackbarPos("Max B", "Control Panel", max_b);
}

int main()
{
	setupControlPanel();
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		std::cout << "Winsock Startup failed: " << wsOk << ", starting offline mode." << std::endl;
		online = false;
	}
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(54000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);
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
				cv::minEnclosingCircle(contours[max_int], sphere_center, sphere_radius);
				cv::circle(img, sphere_center, sphere_radius, cv::Scalar(0, 255, 0), 1);
				//std::cout << sphere_center.x << " " << sphere_center.y << " " << sphere_radius << std::endl;

				float distance = 10000 / sphere_radius;
				pos_raw = Vector3(sphere_center.x, sphere_center.y, distance);
				pos_centered = pos_raw - pos_offset;
				pos_scaled = pos_centered * pos_scale;
				Vector3 scaled_z = (rawpos_z - pos_offset) * pos_scale;
				x_angle = atan2f(scaled_z.y, scaled_z.z);
				pos_out = pos_scaled.RotateX(x_angle);

				cv::putText(img, std::to_string(pos_out.x), cv::Point(10,50), cv::FONT_HERSHEY_SIMPLEX, 0.6,cv::Scalar(100, 255, 0), 1);
				cv::putText(img, std::to_string(pos_out.y), cv::Point(10,70), cv::FONT_HERSHEY_SIMPLEX, 0.6,cv::Scalar(100, 255, 0), 1);
				cv::putText(img, std::to_string(pos_out.z), cv::Point(10,90), cv::FONT_HERSHEY_SIMPLEX, 0.6,cv::Scalar(100, 255, 0), 1);

				if (online)
				{
					pos_out = pos_out.MagnitudeSquared() < 90000 ? pos_out : Vector3(0, 0, 0);
					std::string s = std::to_string(pos_out.x) + " " + std::to_string(pos_out.y) + " " + std::to_string(pos_out.z) + " ";
					int sendOk = sendto(out, s.c_str(), s.size() + 1, 0, (sockaddr*)&server, sizeof(server));
				}
			}
			cv::imshow("input", img);
			cv::imshow("output", threshold_img);
		}
		//cv::cvtColor(img, threshold_img, cv::COLOR_BGR2HSV);
		char key = (char)cv::waitKey(1);
		if (key == 27) break;
	}
}