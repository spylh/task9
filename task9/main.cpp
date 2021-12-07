

#include <iostream>
#include <GL/glut.h>

int WindowPositionX = 100;
int WindowPositionY = 100;
int WindowWidth = 512;
int WindowHeight = 512;
char WindowTitle[] = "Spring";

GLfloat object_color[4] = { 0.0, 1.0, 0.0, 1.0 };	        //　重りの色
GLfloat line_color[4] = { 1.0, 1.0, 1.0, 1.0 };	        //　線の色
GLfloat light_color[4] = { 1.0, 1.0, 1.0, 1.0 };	        //　光源色
GLfloat light_position[4] = { 100.0, 100.0, 100.0, 1.0 };	//　光源位置

//空間のパラメータ
const double mass = 1;		//　質量
const double gravity = 9.8;	        //　重力加速度
const double dt = 0.001;		//　微小時間
const double k = 100.0;	        //　ばね定数

//オブジェクトのパラメータ
const double natural_length = 0.0;	//　自然長
const double first_accel = gravity;	//　初期加速度
const double first_velocity = 0.0;	//　初期速度
const double first_position = 7.5;	//　初期位置

//計算に用いるパラメータ
double position = first_position;	//　位置
double velocity = first_velocity;	//　速度
double accel = first_accel;		//　加速度


double Euler_Method(double pos){
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

double Backstep_Euler_Method(double pos) {
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

double Runge_Kutta_Method(double pos) {

	double k1 = Euler_Method(pos);  //1ステップ目
	double k2 = Euler_Method(k1);   //2ステップ目

	double result = (k1 + k2) / 2;
	return result;
}

void Initialize()
{
	glClearColor(0.3, 0.3, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, light_color);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
}

void Idle()
{
	glutPostRedisplay();
}

void Reshape(int x, int y)
{
	WindowWidth = x;
	WindowHeight = y;
	if (WindowWidth < 1) WindowWidth = 1;
	if (WindowHeight < 1) WindowHeight = 1;
}

void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (double)WindowWidth / (double)WindowHeight, 0.1,
		1000.0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, WindowWidth, WindowHeight);
	glPushMatrix();
	//　視点の描画
	gluLookAt(
		0.0, 0.0, -50.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);
	//　オイラー法
	position =  Runge_Kutta_Method(position);
	//　固定点の描画
	glPushMatrix();
	glColor4fv(line_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, line_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, line_color);
	glTranslated(0.0, 10.0, 0.0);
	glutSolidSphere(1.0, 5, 5);
	glPopMatrix();
	//　線の描画
	glLineWidth(5.0);
	glBegin(GL_LINES);
	glColor4fv(line_color);
	glVertex3d(0.0, 10.0, 0.0);
	glVertex3d(0.0, position, 0.0);
	glEnd();
	//　物体の描画
	glPushMatrix();
	glColor4fv(object_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, object_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, object_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, object_color);
	glTranslated(0.0, position, 0.0);
	glutSolidCube(2);
	glPopMatrix();
	glPopMatrix();
	glutSwapBuffers();
}

void updateEffectorPosition(void) {
	int buttonState;
	double m_currentDistance;
	hduVector3Dd m_centerToEffector = gCenterOfStylusSphere;
	gCenterOfGodSphere = gCenterOfStylusSphere;

	//黒板の範囲内で押し込んだ場合
	if (m_centerToEffector[2] < SSR && abs(m_centerToEffector[0]) <  / 2 && abs(m_centerToEffector[1]) < BOARDSIZE / 2) {
		//力覚フィードバック
		gCenterOfGodSphere[2] = SSR;
		m_currentDistance = SSR - m_centerToEffector[2];
		gForce.set(0.0, 0.0, STIFFNESS * m_currentDistance);
		//お絵描き
		hdGetIntegerv(HD_CURRENT_BUTTONS, &buttonState);
		if (buttonState == 1) {
			//添字リセット
			if (point_index >= 30000)
			{
				point_index = 0;
				loop_counter = 29999;
			}
			else {
				++point_index;
				if (loop_counter != 29999)loop_counter = point_index;
			}
			line_points[point_index][0] = (GLdouble)gCenterOfGodSphere[0];
			line_points[point_index][1] = (GLdouble)gCenterOfGodSphere[1];
			line_points[point_index][2] = SSR + 0.01;
			line_points[point_index][3] = 1;
		}
		else if (buttonState == 2) {
			for (int i = 0; i < loop_counter; i++) {
				if (sqrt(pow(line_points[i][0] - gCenterOfGodSphere[0], 2) + pow(line_points[i][1] - gCenterOfGodSphere[1], 2)) < SSR) {
					line_points[i][3] = 0;
				}
			}

		}

	}
	//そうでない場合
	else {
		gForce.set(0.0, 0.0, 0.0);
	}

}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '\033':
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(WindowPositionX, WindowPositionY);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow(WindowTitle);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);
	glutKeyboardFunc(Keyboard);
	Initialize();
	glutMainLoop();
	return 0;
}
