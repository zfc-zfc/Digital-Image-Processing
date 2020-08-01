#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Bool.h"
#include "std_msgs/Float32.h"

#include<geometry_msgs/Twist.h>
#include "sensor_msgs/Image.h"

#define LINEAR_X 0.0

using namespace std;
using namespace cv;
int vel_pre; //ǰһʱ�̵��ٶ�ϵ��
double num_max = 0;//��ɫ������ֵ
int image_num = 0;//֡��
int main(int argc, char **argv)
{
	VideoCapture capture;
	capture.open(0);//�� zed ���ROS_WARN("*****START");

	ROS_WARN("*****START");
	ros::init(argc, argv, "trafficLaneTrack");//��ʼ��ROS�ڵ�
	ros::NodeHandle n;

	ros::Rate loop_rate(10);//�����ٶȷ���Ƶ��
	ros::Publisher pub = n.advertise<geometry_msgs::Twist>("/smoother_cmd_vel", 5);//�����ٶȷ�����
	if (!capture.isOpened())
	{
		printf("����ͷû��������,���²�ι��ػ��ϵ�����ͷ\n");
		return 0;
	}
	waitKey(1000);
	Mat frame;
	while (ros::ok())
	{
		capture.read(frame);
		if (frame.empty())
		{
			break;
		}

		Mat frIn = frame(cv::Rect(0, 0, frame.cols / 2, frame.rows));//��ȡ zed ��ͼƬ;
		Mat HSV_Img;
		Mat frH;
		cvtColor(frIn, HSV_Img, COLOR_BGR2HSV); //��ͼƬת��ΪHSI
		inRange(HSV_Img, Scalar(43, 43, 46), Scalar(77, 255, 255), frH);
		imshow("H", frH);

		int Num[256] = { 0 };	//�Ҷ�ֵ����
		for (int i = 0; i < frH.rows; i++)//����ͼ�񣬶�Ӧ�Ҷ�ֵ��grayNum++
		{
			for (int j = 0; j < frH.cols; j++)
			{
				Num[frH.at<uchar>(i, j)]++;
			}
		}
		printf("num of 255=%d\n", Num[255]);
		printf("num of 0=%d\n", Num[0]);

		//ͳ�Ƹ�֡���ֵ���Ч��������Ŀ
		if (num_max < Num[255])
			num_max = Num[255];
		else
			num_max = num_max;
		image_num++;
		//ͳ����1��ɫ���صĸ���
		double Num_left1[256] = { 0 };
		for (int i = 0; i < frH.rows; i++)//����ͼ�񣬶�Ӧ�Ҷ�ֵ��grayNum++
		{
			for (int j = 0; j < frH.cols / 5; j++)
			{
				Num_left1[frH.at<uchar>(i, j)]++;
			}
		}
		double left1_pro = Num_left1[255] / Num[255];
		printf("left1 proporation=%lf\n", left1_pro);

		//ͳ����2��ɫ���صĸ���
		double Num_left2[256] = { 0 };
		for (int i = 0; i < frH.rows; i++)//����ͼ�񣬶�Ӧ�Ҷ�ֵ��grayNum++
		{
			for (int j = frH.cols / 5 + 1; (j < 2 * frH.cols / 5) && (j > frH.cols / 5); j++)
			{
				Num_left2[frH.at<uchar>(i, j)]++;
			}
		}
		double left2_pro = Num_left2[255] / Num[255];
		printf("left2 proporation=%lf\n", left2_pro);

		//ͳ���м��ɫ���صĸ���
		double Num_middle[256] = { 0 };
		for (int i = 0; i < frH.rows; i++)//����ͼ�񣬶�Ӧ�Ҷ�ֵ��grayNum++
		{
			for (int j = 2 * frH.cols / 5 + 1; (j < 3 * frH.cols / 5) && (j > 2 * frH.cols / 5); j++)
			{
				Num_middle[frH.at<uchar>(i, j)]++;
			}
		}
		double middle_pro = Num_middle[255] / Num[255];
		printf("middle proporation=%lf\n", middle_pro);

		//ͳ���ұ�1��ɫ���صĸ���
		double Num_right1[256] = { 0 };
		for (int i = 0; i < frH.rows; i++)//����ͼ�񣬶�Ӧ�Ҷ�ֵ��grayNum++
		{
			for (int j = 3 * frH.cols / 3 + 1; (j < 4 * frH.cols / 5) && (j > 3 * frH.cols / 5); j++)
			{
				Num_right1[frH.at<uchar>(i, j)]++;
			}
		}
		double right1_pro = Num_right1[255] / Num[255];
		printf("right1 proporation=%lf\n", right1_pro);

		//ͳ���ұ�2��ɫ���صĸ���
		double Num_right2[256] = { 0 };
		for (int i = 0; i < frH.rows; i++)//����ͼ�񣬶�Ӧ�Ҷ�ֵ��grayNum++
		{
			for (int j = 3 * frH.cols / 3 + 1; (j < 4 * frH.cols / 5) && (j > 3 * frH.cols / 5); j++)
			{
				Num_right2[frH.at<uchar>(i, j)]++;
			}
		}
		double right2_pro = Num_right2[255] / Num[255];
		printf("right2 proporation=%lf\n", right2_pro);

		double vel_forward;
		double vel_angle;
		if (num_max<9000) 
		{
			//��δ��⵽ͣ����־����ԭ����ת
			if (Num[255] > 60 && image_num > 50)//����ǰ��ʮ֡
				vel_forward = 1;
			else if (Num[255] < 60 && image_num < 500)
			{
				cout << "0000000000000000000000000000" << endl;
				geometry_msgs::Twist cmd_red;
				vel_angle = 2;
			}
			else
				vel_forward = 0;
		}
		else
		{
			if(Num[255] > 60 && image_num > 50)
				vel_forward = 1;
			else
				vel_forward = 0;
			//������ֹͣ����������һ�ξ��뵽��ͣ����־ǰ�����Ϸ�
			if (vel_forward == 0 && vel_pre != 0)
			{
				for (i = 0; i < 1000; i++) {
					cmd_red.linear.x = 0.15;
					pub.publish(cmd_red);
					waitKey(2);
				}
			}
		}

		vel_pre = vel_forward;

		
		if ((left1_pro >= 0.5) && (right1_pro <= 0.01) && (vel_forward != 0))
			vel_angle = 3;
		else if ((left1_pro >= 0.15) && (right1_pro < 0.01) && (vel_forward != 0))
			vel_angle = 2;
		else if ((left2_pro >= 0.5) && (right2_pro < 0.01) && (vel_forward != 0))
			vel_angle = 2;
		else if ((left2_pro >= 0.15) && (right2_pro < 0.01) && (vel_forward != 0))
			vel_angle = 1;
		else if ((right1_pro >= 0.15) && (left1_pro < 0.01) && (vel_forward != 0))
			vel_angle = -1;
		else if ((right1_pro >= 0.5) && (left1_pro < 0.01) && (vel_forward != 0))
			vel_angle = -2;
		else if ((right2_pro >= 0.15) && (left2_pro < 0.01) && (vel_forward != 0))
			vel_angle = -2;
		else if ((right2_pro >= 0.5) && (left2_pro < 0.01) && (vel_forward != 0))
			vel_angle = -3;
		else
			vel_angle = 0;

		geometry_msgs::Twist cmd_red;

		//�����ٶ�����ֵ
		cmd_red.linear.x = 0.1*vel_forward;
		cmd_red.linear.y = 0;
		cmd_red.linear.z = 0;
		cmd_red.angular.x = 0;
		cmd_red.angular.y = 0;
		cmd_red.angular.z = 0.2*vel_angle;

		pub.publish(cmd_red);
		ros::spinOnce();
		waitKey(5);
	}
	return 0;
}

