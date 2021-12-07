#include "parameter.h"

static double Euler_Method(double pos)
{
	//　力を求める
	double force = -k * (pos - natural_length) + mass * gravity;
	//　加速度を求める
	accel = force / mass;
	//　速度を求める
	velocity += accel * dt;
	//　位置を求める
	double result = pos + velocity * dt;
	return result;
}

static double Backstep_Euler_Method(double pos) {

	//　力を求める
	double force = -k * (Euler_Method(pos) - natural_length) + mass * gravity;
	//　加速度を求める
	accel = force / mass;
	//　速度を求める
	velocity += accel * dt;
	//　位置を求める
	double result = pos + velocity * dt;

	return result;
}

static double Runge_Kutta_Method(double pos) {

	double k1 = Euler_Method(pos);  //1ステップ目
	double k2 = Euler_Method(k1);   //2ステップ目

	double result = (k1 + k2) / 2;
	return result;

}