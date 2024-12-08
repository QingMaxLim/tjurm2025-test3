#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

//计算两个轮廓之间的最小距离
//计算最近点
Point closestPoint(const Point& point,const vector<Point>& contour){
    double minDistance = numeric_limits<double>::max();// 初始化一个极大的值
    Point closestPoint;
    for(const auto& contourPoint:contour){
        double distance = norm(point - contourPoint);
        if(distance < minDistance){
            minDistance = distance;
            closestPoint = contourPoint;
        }
    }
    return closestPoint;
}

//计算两个轮廓之间的最小距离
double minDistanceBetweenCountours(const vector<Point>& contour1,const vector<Point>& contour2){
    double minDistance = numeric_limits<double>::max();//初始化一个极大的值
    for(const auto& point : contour1){
        Point nearestPoint = closestPoint(point,contour2);
        double distance = norm(point - nearestPoint);
        if(distance < minDistance){
            minDistance = distance;
        }
    }
    return minDistance;
}

//按两个轮廓的面积升序排序
bool compareContours(const vector<Point>& contour1,const vector<Point>& contour2){
    double area1 = contourArea(contour1);
    double area2 = contourArea(contour2);
    return area1 < area2;
}

//按两个轮廓的周长升序排序
bool comparePerimeter(const vector<Point>& contour1,const vector<Point>& contour2){
    double perimeter1 = arcLength(contour1,true);
    double perimeter2 = arcLength(contour2,true);
    return perimeter1 < perimeter2;
}

//计算轮廓重心
Point caculateControl(const vector<Point>& contour){
    Moments m = moments(contour);//计算轮廓的三个矩
    double x = m.m10 / m.m00;//计算轮廓在x上的平均位置
    double y = m.m01 / m.m00;//计算y上的
    return Point(static_cast<int>(x),static_cast<int>(y));
}
//m00 是轮廓的零阶矩，表示轮廓的总质量（如果每个点都被视为具有相同的质量）
//m10 是关于 x 轴的一阶矩，表示轮廓的质量中心在 x 轴上的位置
//m01 是关于 y 轴的一阶矩，表示轮廓的质量中心在 y 轴上的位。

//计算点与直线的上下左右关系
int pointLinePosition(Point p,Point a,Point b){
    Point dir = b - a;//线段的方向向量
    Point pos = p - a;//点相对于起点a的方向向量
    double crossProduct = dir.x*pos.y - dir.y*pos.x;//叉乘

    if(crossProduct < 0){//点在线左侧
        return -1;
    }else if(crossProduct > 0){//点在线右侧
        return 1;
    }else{//点在线段或线段垂直且点在线段上
        double dotProduct = dir.x*pos.x + dir.y*pos.y;//点乘
        if(dotProduct < 0){//点在线段下面
            return -1;
        }else{//点在线段上面
            return 0;
        }
    }
}

//计算两点的欧式距离
double caculateDistnace(Point2f p1,Point2f p2){
    return sqrt(pow(p1.x - p2.x,2) + pow(p1.y-p2.y,2));
}