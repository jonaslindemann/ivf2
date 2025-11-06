#pragma once

void collision3D(double R, double m1, double m2, double r1, double r2,
                 double& x1, double& y1,double& z1,
                 double& x2, double& y2, double& z2,
                 double& vx1, double& vy1, double& vz1,
                 double& vx2, double& vy2, double& vz2,
                 int& error);
                 
double random_number();
double random(double minValue, double maxValue);
