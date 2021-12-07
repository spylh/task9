#include "parameter.h"

static double Euler_Method(double pos)
{
	//�@�͂����߂�
	double force = -k * (pos - natural_length) + mass * gravity;
	//�@�����x�����߂�
	accel = force / mass;
	//�@���x�����߂�
	velocity += accel * dt;
	//�@�ʒu�����߂�
	double result = pos + velocity * dt;
	return result;
}

static double Backstep_Euler_Method(double pos) {

	//�@�͂����߂�
	double force = -k * (Euler_Method(pos) - natural_length) + mass * gravity;
	//�@�����x�����߂�
	accel = force / mass;
	//�@���x�����߂�
	velocity += accel * dt;
	//�@�ʒu�����߂�
	double result = pos + velocity * dt;

	return result;
}

static double Runge_Kutta_Method(double pos) {

	double k1 = Euler_Method(pos);  //1�X�e�b�v��
	double k2 = Euler_Method(k1);   //2�X�e�b�v��

	double result = (k1 + k2) / 2;
	return result;

}