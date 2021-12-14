#include <iostream>
#include <GL/glut.h>
#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduMatrix.h>



//ウインドウの初期化パラメータ
int WindowPositionX = 100;
int WindowPositionY = 100;
int WindowWidth = 512;
int WindowHeight = 512;
char WindowTitle[] = "Spring";

int buttonState;

GLfloat object_color[4] = { 0.0, 1.0, 0.0, 1.0 };	        //　重りの色
GLfloat line_color[4] = { 1.0, 1.0, 1.0, 1.0 };	        //　線の色
GLfloat sphere_color[4] = { 1.0, 0.0, 0.0, 1.0 };		//PHANTOMの先っちょの色
GLfloat light_color[4] = { 1.0, 1.0, 1.0, 1.0 };	        //　光源色
GLfloat light_position[4] = { 100.0, 100.0, 100.0, 1.0 };	//　光源位置

//空間のパラメータ
const double mass = 50;		//　質量
const double gravity = 9.8;	        //　重力加速度
const double dt = 0.001;		//　微小時間
const double k = 1000.0;	        //　ばね定数

//オブジェクトのサイズなどの設定
const double cube_size = 2.0;
const double sphere_size = 2.0;

//オブジェクトの力学的なパラメータ
const double natural_length = 0.0;	//　自然長
const double first_accel = gravity;	//　初期加速度
const double first_velocity = 0.0;	//　初期速度
const double first_position = 7.5;	//　初期位置
double position = first_position;	//　位置
double velocity = first_velocity;	//　速度
double accel = first_accel;		//　加速度
double force = 0.0;

//PHANTOM関連
static hduVector3Dd gCenterOfStylusSphere, gCenterOfGodSphere, gForce;
static HHD ghHD = HD_INVALID_HANDLE;
HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;

//使用する力学シミュレーション関数
#define METHOD Euler_Method(position)


//オイラー
double Euler_Method(double pos) {
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
//後退オイラー
double Backstep_Euler_Method(double pos) {
	//　力を求める
	force = -k * (Euler_Method(pos) - natural_length) + mass * gravity;
	//　加速度を求める
	accel = force / mass;
	//　速度を求める
	velocity += accel * dt;
	//　位置を求める
	double result = pos + velocity * dt;

	return result;
}
//ルンゲ・クッタ(二次)
double Runge_Kutta_Method(double pos) {

	double k1 = Euler_Method(pos);  //1ステップ目
	double k2 = Euler_Method(k1);   //2ステップ目

	double result = (k1 + k2) / 2;
	return result;
}

void updateEffectorPosition(void) {
	//hdGetIntegerv(HD_CURRENT_BUTTONS, &buttonState);
	double _force = Euler_Method(position);
	//箱に当たっている時に
	if (abs(gCenterOfStylusSphere[0])<sphere_size+cube_size && abs(gCenterOfStylusSphere[2])<sphere_size+cube_size) {
		if (gCenterOfStylusSphere[1] > position - cube_size && gCenterOfStylusSphere[1] < position + cube_size) {
			sphere_color[1] = 1.0;
			if (buttonState == 1) {
				sphere_color[0] = 0.0;
				printf("%f\n", _force);
				gForce.set(0.0, -_force * 0.1, 0.0);
				position = gCenterOfStylusSphere[1];
			}
			else {
				sphere_color[0] = 0.0;
			}
		}
	}
	else {
		sphere_color[1] = 0.0;
		sphere_color[0] = 1.0;
		gForce.set(0.0,0.0,0.0);
	}
}

HDCallbackCode HDCALLBACK ContactDB(void* data) {
	HHD hHD = hdGetCurrentDevice();
	hdBeginFrame(hHD);
	hdGetDoublev(HD_CURRENT_POSITION, gCenterOfStylusSphere);
	updateEffectorPosition();
	hdSetDoublev(HD_CURRENT_FORCE, gForce);
	hdEndFrame(hHD);
	return HD_CALLBACK_CONTINUE;
}

void exitHandler() {
	hdStopScheduler();
	if (ghHD != HD_INVALID_HANDLE) {
		hdDisableDevice(ghHD);
		ghHD = HD_INVALID_HANDLE;
	}
	printf("終了\n");
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

void display()
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
	
	//　力学的な計算
	position = METHOD;

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
	glutSolidCube(cube_size);
	glPopMatrix();
	glPopMatrix();

	//PHANTOMの先っちょの描画
	glPushMatrix();
	glColor4fv(sphere_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, sphere_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, sphere_color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sphere_color);
	glTranslated(gCenterOfStylusSphere[0],gCenterOfStylusSphere[1],-50+gCenterOfStylusSphere[2]);//描画上のオフセットをZ軸に
	printf("%f,%f,%f,%d\n", gCenterOfStylusSphere[0], gCenterOfStylusSphere[1], gCenterOfStylusSphere[2],buttonState);
	glutSolidSphere(sphere_size,20,5);
	glPopMatrix();
	glPopMatrix();

	glutSwapBuffers();
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '\033':
		exit(0);
		break;
	case 'w':
		printf("pushed!\n");
		buttonState = 1;
		break;
	default:
		buttonState = 0;
		break;
	}
}

int main(int argc, char** argv)
{
	HDErrorInfo error;
	printf("起動します\n");

	atexit(exitHandler);

	ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
	hdEnable(HD_FORCE_OUTPUT);
	hdEnable(HD_MAX_FORCE_CLAMPING);
	hdStartScheduler();

	glutInit(&argc, argv);
	glutInitWindowPosition(WindowPositionX, WindowPositionY);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow(WindowTitle);
	glutDisplayFunc(display);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);
	glutKeyboardFunc(Keyboard);
	Initialize();

	gSchedulerCallback = hdScheduleAsynchronous(ContactDB, NULL, HD_DEFAULT_SCHEDULER_PRIORITY);

	glutMainLoop();
	return 0;
}
