#include <iostream>
#include <opencv2/opencv.hpp>
#include "caculate.h"

using namespace std;
using namespace cv;

int main(int,char**){

    //定义侧面的大拐角和四个锚点角
    vector<Point> MaxTriangleContour;
    vector<Point> cornerFirst;
    vector<Point> cornerSecond;
    vector<Point> cornerThird;
    vector<Point> cornerFourth;
    //定义特殊点边上的两个小锚点
    vector<Point> anchorUp;
    vector<Point> anchorDown;
    
    Mat src = imread("../res/oreTrough/20211839746.jpg");
    // imshow("src",src);

    //如果图片读取失败则退出
    if(src.empty()){
        cout << "Failed to read image" << endl;
        return EXIT_FAILURE;
    }

    Mat Hsv;
    cvtColor(src,Hsv,COLOR_BGR2HSV);
    // imshow("Hsv",Hsv);

    //设置红色阈值
    Scalar lowerRed(0,40,40);
    Scalar upperRed(20,255,255);

    //提取红色
    Mat mask;
    inRange(Hsv,lowerRed,upperRed,mask);//获取掩膜，符合阈值变白，反之变黑
    // imshow("mask",mask);
    Mat red;
    bitwise_and(src,src,red,mask = mask);//获取红色图像，与掩膜白色重合部分保留，其余变黑
    // imshow("red",red);

    //灰度化
    Mat gray;
    cvtColor(red,gray,COLOR_BGR2GRAY);
    // imshow("gray",gray);

    //闭运算(先膨胀，再腐蚀)(可选)
    // Mat morphologyImage;
    // Mat kernel = getStructuringElement(MORPH_RECT,Size(5,5));
    // morphologyEx(gray,morphologyImage,MORPH_CLOSE,kernel);
    // imshow("morphologyImage",morphologyImage);

    //中值滤波处理
    Mat blur;
    // GaussianBlur(gray,blur,Size(5,5),1);//高斯滤波
    medianBlur(gray,blur,5);//降噪
    // imshow("blur",blur);

    //边缘检测
    Mat canny;
    Canny(blur,canny,60,200);
    // imshow("canny",canny);

    //查找最外层轮廓
    vector<vector<Point>> contours;//所有检测到的轮廓
    vector<Vec4i> hierachy;//存储轮廓的层次关系
    findContours(canny,contours,hierachy,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE,Point(0,0));

    // //描绘轮廓
    // Mat Contours = Mat::zeros(src.size(),CV_8UC3);//创建一个包含三个8位无符号整数通道的矩阵用于存储图像
    // RNG rng(100);//初始化随机数生成器
    // for(int i = 0;i < hierachy.size();i++){
    //     Scalar color = Scalar(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));//随机生成一种颜色
    //     drawContours(Contours,contours,i,color,1,8,hierachy,0,Point());//绘制
    // }
    // imshow("Contours",Contours);

    //按面积大小升序排列
    // sort(contours.begin(),contours.end(),compareContours);
    
    //按周长的大小升序排列
    sort(contours.begin(),contours.end(),comparePerimeter);
    
    //如果轮廓数!= 6，则侧面有大拐角，取出并从contours中删除
    if(contours.size() != 6){
        MaxTriangleContour = contours.back();
        contours.pop_back();//从contours列表中删除最后一个元素
    }

    //如果特殊顶点边上只有一个小锚点 则取出拐角轮廓
    if(contours.size() == 6){
        if(minDistanceBetweenCountours(contours[0],contours[1]) > 50){
            MaxTriangleContour = contours.back();
            contours.pop_back();
        }
    }

    //找出特殊点边上的两个锚点
    if(contours.size() == 6){
        if(minDistanceBetweenCountours(contours[0],contours[1]) < 50){
            Rect rect0 = boundingRect(contours[0]);
            Rect rect1 = boundingRect(contours[1]);
            if(rect0.y > rect1.y){
                anchorUp = contours[1];
                anchorDown = contours[0];
            }else{
                anchorUp = contours[0];
                anchorDown = contours[1];
            }
        }

        //从轮廓剩下的轮廓集中删除两个小锚点
        contours.erase(contours.begin());
        contours.erase(contours.begin());
        //找出特殊点，从剩下的轮廓集中删除
        for(int i = 0;i < contours.size();i++){
            double minDistanceUp = minDistanceBetweenCountours(anchorUp,contours[i]);
            double minDistanceDown = minDistanceBetweenCountours(anchorDown,contours[i]);
            if(minDistanceUp < 30 && minDistanceDown < 30){
                cornerFirst = contours[i];
                contours.erase(contours.begin() + i);
            }
        }

        //找出特殊点对角的轮廓
        if(minDistanceBetweenCountours(cornerFirst,contours[0]) > minDistanceBetweenCountours(cornerFirst ,contours[1])){
            if(minDistanceBetweenCountours(cornerFirst,contours[0]) > minDistanceBetweenCountours(cornerFirst,contours[2])){
                cornerThird = contours[0];
                contours.erase(contours.begin());
            }else{
                cornerThird = contours[2];
                contours.pop_back();
            }
        }else{
            if(minDistanceBetweenCountours(cornerFirst,contours[1]) > minDistanceBetweenCountours(cornerFirst,contours[2])){
                cornerThird = contours[1];
                contours.erase(contours.begin() + 1);
            }else{
                cornerThird = contours[2];
                contours.pop_back();
            }
        }

        //计算四边形四个拐角轮廓的重心
        Point cornerFirstMoment = caculateControl(cornerFirst);
        Point cornerThirdMoment = caculateControl(cornerThird);
        Point contours0Moment = caculateControl(contours[0]);
        Point contours1Moment = caculateControl(contours[1]);

        //找第二个拐角
        int position = pointLinePosition(contours1Moment,cornerFirstMoment,cornerThirdMoment);
        switch (position){
        case -1:
            //点在线段的左侧和下方
            cornerSecond = contours[1];
            contours.pop_back();
            break;
    
        case 1:
            //点在线段的右侧和上方
            cornerSecond = contours[0];
            contours.erase(contours.begin());
            break;
        }

        //第四个拐角
        cornerFourth = contours[0];
        Point cornerSecondMomnet = caculateControl(cornerSecond);
        Point cornerFourthMoment = caculateControl(cornerFourth);

        //标记四个拐角
        circle(src,cornerFirstMoment,5,Scalar(255,200,0),-1);
        putText(src,"1",cornerFirstMoment,FONT_HERSHEY_SIMPLEX,1,Scalar(255,200,0),LINE_4);
        circle(src,cornerSecondMomnet,5,Scalar(255,200,0),-1);
        putText(src,"2",cornerSecondMomnet,FONT_HERSHEY_SIMPLEX,1,Scalar(255,200,0),LINE_4);
        circle(src,cornerThirdMoment,5,Scalar(255,200,0),-1);
        putText(src,"3",cornerThirdMoment,FONT_HERSHEY_SIMPLEX,1,Scalar(255,200,0),LINE_4);
        circle(src,cornerFourthMoment,5,Scalar(255,200,0),-1);
        putText(src,"4",cornerFourthMoment,FONT_HERSHEY_SIMPLEX,1,Scalar(255,200,0),LINE_4);
        
        // imshow("src",src);
    }

    //标记大拐角的点
    if(!MaxTriangleContour.empty()){
        //定义点变量
        Point upOrLeft;
        Point downOrRight;
        Point middle;
        Point middleLeftOrUP;
        Point middleRightOrDown;

        //与源图像src大小相同，但数据类型为无符号8位整数（灰度值），且为单通道的新图像
        Mat drawImage = Mat::zeros(src.size(),CV_8UC1);
        //定义逼近精确度，值越小
        const double epsilon = 0.01 * arcLength(MaxTriangleContour,true);
        vector<Point> approx;
        //使用逼近曲线函数消除轮廓的锯齿
        approxPolyDP(MaxTriangleContour,approx,epsilon,true);
        //画出轮廓并进行多边形逼近，使其成为整体
        drawContours(drawImage,vector<vector<Point>>{approx},-1,Scalar(255,255,255),15);
        // imshow("dst",drawImage);

        //对画出的图形进行腐蚀操作，使图片变细
        Mat kernel = getStructuringElement(MORPH_RECT,Size(11,11));//用于创建一个宽11、长11的矩形结构元素(内核)
        // dilate(drawImage,drawImage,kernel);//膨胀
        erode(drawImage,drawImage,kernel);
        // imshow("erode",drawImage);

        //图像进行角点检测，找出两端的点和中间的点
        vector<Point> corners;
        goodFeaturesToTrack(drawImage,corners,3,0.1,30);//用于检测角点(特征点)

        // cout << corners.size() << endl;

        //如果检测出的角点个数是轮廓的两个端点，则打出这两个点的位置关系，并通过这两个点的位置关系，找出中间的点
        if(corners.size() == 2){
            if(abs(corners[0].x - corners[1].x) > abs(corners[0].y - corners[1].y)){
                if(corners[0].x > corners[1].x){
                    upOrLeft = corners[1];
                    corners.pop_back();
                }else{
                    upOrLeft = corners[0];
                    corners.erase(corners.begin());
                }
            }else{
                if(corners[0].y > corners[1].y){
                    upOrLeft = corners[1];
                    corners.pop_back();
                }else{
                    upOrLeft = corners[0];
                    corners.erase(corners.begin());
                }
            }

            downOrRight = corners[0];
            
            middleLeftOrUP = upOrLeft;//初始化
            middleRightOrDown = upOrLeft;//初始化

            if(abs(upOrLeft.x - downOrRight.x) < (upOrLeft.y - downOrRight.y)){
                for(const auto& point : MaxTriangleContour){
                    if(point.y < middleLeftOrUP.y) middleLeftOrUP = point;
                    if(point.y > middleRightOrDown.y) middleRightOrDown = point;
                }
                if(norm(middleLeftOrUP - upOrLeft) < 20 || norm(middleLeftOrUP - downOrRight) < 20){
                    middle = middleRightOrDown;
                }else{
                    middle = middleLeftOrUP;
                }
            }else{
                for(const auto& point : MaxTriangleContour){
                    if(point.x < middleLeftOrUP.x) middleLeftOrUP = point;
                    if(point.x > middleRightOrDown.x) middleRightOrDown = point;
                }
                if(norm(middleLeftOrUP - upOrLeft) < 20 || norm(middleLeftOrUP - downOrRight) < 20){
                    middle = middleRightOrDown;
                }else{
                    middle = middleLeftOrUP;
                }
            }
        }

        //如果检测出的角点是包括两个端点和拐角处的点在内的三个点，则根据这三个点的距离计算出位置关系
        if(corners.size() == 3){
            Point2f point1 = corners[0];
            Point2f point2 = corners[1];
            Point2f point3 = corners[2];

            //计算所有可能的距离对
            double d12 = caculateDistnace(point1,point2);
            double d13 = caculateDistnace(point1,point3);
            double d23 = caculateDistnace(point2,point3);

            //找出最远距离
            double maxDistance = d12;
            Point2f farPoint1 = point1;
            Point2f farPoint2 = point2;
            Point2f tempM = point3;

            if(d13 > maxDistance){
                maxDistance = d13;
                farPoint1 = point1;
                farPoint2 = point3;
                tempM = point2;
            }

            if(d23 > maxDistance){
                maxDistance = d23;
                farPoint1 = point2;
                farPoint2 = point3;
                tempM = point1;
            }

            upOrLeft = farPoint1;
            downOrRight = farPoint2;
            middle = tempM;

            //判断轮廓是横的还是竖的，并计算两个端点的位置关系
            if(abs(upOrLeft.x - downOrRight.x) > abs(upOrLeft.y - downOrRight.y)){
                if(upOrLeft.x > downOrRight.x){
                    Point temp = upOrLeft;
                    upOrLeft = downOrRight;
                    downOrRight = temp;
                }
            }else{
                if(upOrLeft.y > downOrRight.y){
                    Point temp = upOrLeft;
                    upOrLeft = downOrRight;
                    downOrRight = temp;
                }
            }
        }
        
        //标记点
        circle(src,upOrLeft,5,cv::Scalar(255,200,0),-1);
        putText(src,"1",upOrLeft,cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255,200,0),cv::LINE_4);        
        circle(src,downOrRight,5,cv::Scalar(255,200,0),-1);
        putText(src,"3",downOrRight,cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255,200,0),cv::LINE_4);
        circle(src,middle,5,cv::Scalar(255,200,0),-1);
        putText(src,"2",middle,cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(255,200,0),cv::LINE_4);

        // imshow("drawImage",drawImage);
    }

    imshow("src",src);

    //相机的内参矩阵
    Mat cameraMatrix = (Mat_<double>(3,3) << 1.521928836685752e+03,0,9.504930579948792e+02,0,1.521695508574793e+03,6.220985582938733e+02,0,0,1);

    //相机的畸变系数
    Mat distCoeffs = Mat::zeros(5,1,DataType<double>::type);
    distCoeffs.at<double>(0,0) = -0.157095872989630;
    distCoeffs.at<double>(1,0) = 0.166823029778507;
    distCoeffs.at<double>(2,0) = 1.356728774532785e-04;
    distCoeffs.at<double>(3,0) = 2.266474993725451e-04;
    distCoeffs.at<double>(4,0) = -0.070807947517560;

    //图像坐标点
    // cout << cornerThirdMoment.x << " " << cornerThirdMoment.y << endl;
    vector<Point2f> imagePoint = {Point2f()};

    //世界坐标点的三维坐标
    vector<Point3f> objectPoint = {Point3f(0,0,0),Point3f(0,0,288),Point3f(288,0,0)};//O,A,B

    //相机位姿态估计
    Mat rvec;//旋转向量
    Mat tvec;//平移向量

    //pnp解算
    solvePnP(objectPoint,imagePoint,cameraMatrix,distCoeffs,rvec,tvec);

    waitKey(0);

    return 0;
}