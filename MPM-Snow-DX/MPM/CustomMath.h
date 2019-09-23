#pragma once
#ifndef CUSTOM_MATH_H
#define CUSTOM_MATH_H

#include <math.h>
#include <Eigen\Dense>

static void loadIdentity(Eigen::Matrix3d& m)
{
	m(0, 0) = 1; m(0, 1) = 0; m(0, 2) = 0;
	m(1, 0) = 0; m(1, 1) = 1; m(1, 2) = 0;
	m(2, 0) = 0; m(2, 1) = 0; m(2, 2) = 1;
}


static void setData(Eigen::Vector3d& m, const double& x, const double& y, const double& z)
{
	m(0) = x; m(1) = y; m(2) = z;
}


static void setData(Eigen::Matrix3d& m, const double& c)
{
	m(0, 0) = c; m(0, 1) = c; m(0, 2) = c;
	m(1, 0) = c; m(1, 1) = c; m(1, 2) = c;
	m(2, 0) = c; m(2, 1) = c; m(2, 2) = c;
}


static void diagSum(Eigen::Matrix3d& m, const double& c)
{
	for (int i = 0; i < 3; i++)
	{
		m(i, i) += c;
	}
}


static void diagProduct(Eigen::Matrix3d& m, const Eigen::Vector3d& v)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m(i, j) *= v(i);
		}
	}
}


static void diagProduct_Inv(Eigen::Matrix3d& m, const Eigen::Vector3d& v)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m(i, j) /= v(i);
		}
	}
}


static double product(const Eigen::Vector3d& v)
{
	return v(0) * v(1) * v(2);
}


static double lengthSquared(const Eigen::Vector3d& v)
{
	return v(0)*v(0) + v(1)*v(1) + v(2)*v(2);
}


static Eigen::Vector3d division(const Eigen::Vector3d& a, const Eigen::Vector3d& b)
{
	return Eigen::Vector3d(a(0) / b(0), a(1) / b(1), a(2) / b(2));
}


static Eigen::Vector3d add_const(const Eigen::Vector3d& v, double c)
{
	return Eigen::Vector3d(v(0) + c, v(1) + c, v(2) + c);
}


static Eigen::Matrix3d outerProduct(const Eigen::Vector3d& a, const Eigen::Vector3d& b)
{
	Eigen::Matrix3d m;
	m(0, 0) = a(0) * b(0); m(0, 1) = a(0) * b(1); m(0, 2) = a(0) * b(2);
	m(1, 0) = a(1) * b(0); m(1, 1) = a(1) * b(1); m(1, 2) = a(1) * b(2);
	m(2, 0) = a(2) * b(0); m(2, 1) = a(2) * b(1); m(2, 2) = a(2) * b(2);

	return m;
}

static Eigen::Vector3d dot(const Eigen::Vector3d& a, const Eigen::Vector3d& b)
{
	return Eigen::Vector3d(a(0) * b(0), a(1) * b(1), a(2) * b(2));
}

#endif // !CUSTOM_MATH_H
