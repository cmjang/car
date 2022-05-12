#include "headfile.h"	//调节后右电机速度
#define righA PWM1P_P60 //右轮
#define righB PWM2P_P62
#define leftA PWM3P_P64 //左轮
#define leftB PWM4P_P66
//定义红外传感器针脚
#define hw1 P06
#define hw2 P04
#define hw3 P03
#define hw4 P05
#define hw5 P07
#define hw6 P02

//五轴开关
#define UP P24
#define STOPP P61
#define DWN P26
#define LFT P74
#define RHT P76
#define SET P75
#define BOTTOM 1
#define TOP 8
//五轴开关
//屏幕显示
int num[7] = {0, 0, 0, 0, 0, 0, 0};
uint8 k; //屏幕指针
int i;
int j;
int time1;
int startcar = 1;
int start2 = 1;
//===================================================================
int tmp1, tmp2;
//屏幕显示字符
char str[][12] = {"Delay:", "WT", "CT:", "Speed", "Small", "Midle:", "Big:", "Turn:"};
//========================================================================================
int delaytime;
float wt, ct;					   //速度调节参数设置
int sign;						   //路口判断标志
int error = 0;					   //速度误差调节值
int hw[7] = {0, 0, 0, 0, 0, 0, 0}; //存储传感器状态
int motor_speed;				   //小车速度
int L_motor_speed;				   //调节后左电机速度
int R_motor_speed;				   //调节后右电机速度

void sign_crossing();	//判断路口类型
void m_speed_change();	//电机速度调节
void turn();			//特殊情况转向
void straight_S_line(); //直线和S弯输出控制电机PWM
//===========================================================================
//屏幕显示函数
void showdata();
void keyboard();
void writedata(); // eeprom write
void readdata();  // eeprome read
//============================================================================================
void main()
{
	DisableGlobalIRQ();
	board_init();
	iap_init();				//初始化EEPROM
	lcd_init();				// lcd初始化
	pit_timer_ms(TIM_1, 3); //红外检测
	// pit_timer_ms(TIM_1, 100);
	k = 1;
	tmp1 = 0;
	tmp2 = 8;

	//五轴按键引脚设置
	gpio_mode(UP, GPI_IMPEDANCE);
	gpio_mode(SET, GPI_IMPEDANCE);
	gpio_mode(LFT, GPI_IMPEDANCE);
	gpio_mode(RHT, GPI_IMPEDANCE);
	gpio_mode(DWN, GPI_IMPEDANCE);
	gpio_mode(STOPP, GPI_IMPEDANCE);
	gpio_mode(hw1, GPI_IMPEDANCE);
	gpio_mode(hw2, GPI_IMPEDANCE);
	gpio_mode(hw3, GPI_IMPEDANCE);
	gpio_mode(hw4, GPI_IMPEDANCE);
	gpio_mode(hw5, GPI_IMPEDANCE);
	gpio_mode(hw6, GPI_IMPEDANCE);

	//=================================================================
	readdata(); //读取eeprom中的数据
	EnableGlobalIRQ();
	//电机启动 初速度为0
	pwm_init(righA, 1000, 0);
	pwm_init(righB, 1000, 0);
	pwm_init(leftA, 1000, 0);
	pwm_init(leftB, 1000, 0);
	//启动前屏幕检测 调参
	while (startcar)
	{
		uint8 temmp[6];
		keyboard();
		showdata();

		temmp[0] = '0' + (uint8)hw[0];
		temmp[1] = '0' + (uint8)hw[1];
		temmp[2] = '0' + (uint8)hw[2];
		temmp[3] = '0' + (uint8)hw[3];
		temmp[4] = '0' + (uint8)hw[4];
		temmp[5] = '0' + (uint8)hw[5];
		lcd_showstr(0, 9, temmp);
	}
	//车子参数初始化
	motor_speed = num[3];
	wt = num[1];
	ct = num[2];
	delaytime = num[0];
	time1 = 0;
	pit_timer_ms(TIM_3, 1000);
	while (start2)
	{
		sign_crossing();   //判断路口类型
		m_speed_change();  //电机速度调节
		turn();			   //特殊情况转向
		straight_S_line(); //直线 输出控制电机PWM
	}
}

void showdata()
{
	for (i = tmp1; i < tmp2; ++i)
	{
		lcd_showstr(0, i, str[i]);
		lcd_showint16(48, i, num[i]);
	}
	lcd_showstr(105, k - 1, "<-");
}
void keyboard()
{
	if (UP == 0)
	{
		lcd_showstr(105, k - 1, "  ");
		k = (k - BOTTOM + TOP - 1) % (TOP - BOTTOM + 1) + 1;
		tmp1 = k - 1;
		tmp2 = k;
		showdata();
		delay_ms(90);
	}
	else if (DWN == 0)
	{
		lcd_showstr(105, k - 1, "  ");
		k = (k + BOTTOM + TOP - 1) % TOP + 1;
		tmp1 = k - 1;
		tmp2 = k;
		showdata();
		delay_ms(90);
	}
	else if (LFT == 0)
	{
		num[k - 1]--;
		tmp1 = k - 1;
		tmp2 = k;
		showdata();
		delay_ms(60);
	}
	else if (RHT == 0)
	{
		num[k - 1]++;
		tmp1 = k - 1;
		tmp2 = k;
		showdata();
		delay_ms(60);
	}
	else if (SET == 0)
	{ //往eeprom中写入数据
		writedata();
	}
	else if (STOPP == 0)
	{
		startcar = 0;
	}
}
void writedata()
{
	iap_erase_page(0x00); //清除扇区数据
	iap_write_bytes(0X00, (uint8 *)num, (TOP - BOTTOM + 1) * sizeof(int));
}
void readdata()
{
	uint16 adddd = 0x00; //写入扇区数据
	iap_read_bytes(0x00, (uint8 *)num, (TOP - BOTTOM + 1) * sizeof(int));
}

void sign_crossing() //判断路口类型，如果左边几个同时扫描到黑线，右边几个没有扫描到黑线，就为左直角转弯，反之右直角转弯。sign为路口状态标记
{
	//-----------------------------------------------直角路口判断----------------------------------------------------

	if ((hw[0] == 1) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{ //左90度路口  1 1 1  0 0 0    左转90度
		sign = 1;
	}
	else if ((hw[0] == 0) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{ //左90度路口  0 1 1  1 0 0    左转90度
		sign = 1;
	}
	else if ((hw[0] == 1) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{ //左90度路口  1 1 1  1 0 0    左转90度
		sign = 1;
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 0))
	{ //右90度路口 0 0 0  0 1 1     右转90度
		sign = 2;
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 1))
	{ //右90度路口 0 0 0  1 1 1     右转90度
		sign = 2;
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 1))
	{ //右90度路口 0 0 1   1 1 1     右转90度
		sign = 2;
	}
	if (((hw[0] == 1) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 1)) && startcar == 0)
	{
		if (time1 > 6)
		{
			start2 = 0;
			// sign = 0;
			pwm_duty(righA, 255);
			pwm_duty(righB, 255);
			pwm_duty(leftA, 255);
			pwm_duty(leftB, 255);
		}
	}
}
void m_speed_change() //速度调节，error 是速度调节量
{

	if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 1))
	{
		error = ((float)num[6] / 10) * ct; //          0 0 0 0 0 1  error 是速度调节量
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 1) && (hw[5] == 0))
	{
		error = ((float)num[5] / 10) * ct; //          0 0 0 0 1 0
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = ((float)num[4] / 10) * wt; //          0 0 0   1 0 0  中间两个的右边扫到，左边速度调大一点点
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 0))
	{
		error = ((float)num[4] / 10) * wt; //          0 0 0   1 1 0  中间两个的右边扫到，左边速度调大一点点
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = 0; //         0 0 1   1 0 0  中间两个红外传感器宽度小于黑线，如果同时扫到，0误差直行
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = -((float)num[4] / 10) * wt; //         0 0 1   0 0 0  中间两个的左边扫到，右边速度调大一点点
	}
	else if ((hw[0] == 0) && (hw[1] == 1) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = -((float)num[5] / 10) * ct; //          0 1 0    0 0 0
	}
	else if ((hw[0] == 0) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = -((float)num[5] / 10) * ct; //          0 1 1    0 0 0
	}
	else if ((hw[0] == 1) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = -((float)num[6] / 10) * ct; //         1 0 0   0 0 0
	}
	//-----------------------------------------------小车出线状态反应----------------------------------------------------//
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{											   // 0 0 0 0    0 0 0 0
												   /*此情况是当小车出线时情况，大部分是急转弯路况发生，
												   如果最后一次是右边扫描到线，说明车是在线左边，那么加
												   大右拐的速度，如果最后一次是左边扫描到线，说明车是在
												   线右边，那么加大左拐的速度*/
		if (error == (-((float)num[6] / 10) * ct)) //判断上次情况，如果上次是右边扫描到线，加大右拐的速度
		{
			error = (-3 * ct);
		}
		else if (error == (((float)num[6] / 10) * ct)) //判断上次情况，如果上次是左边扫描到线，加大左拐的速度
		{
			error = 3 * ct;
		}
	}
}
void turn() //特殊情况转向
{

	if (sign == 1) // 90度路口    左转90度
	{
		pwm_duty(leftA, 255);
		pwm_duty(leftB, 255);

		pwm_duty(righA, num[7]);
		pwm_duty(righB, 0);
		delay_ms(delaytime);
		sign = 0; //使路口状态标记变为一个void turn () 判断里没有的数，使其跳出转向，否者会一直执行右转90度，原地打转。下面同理
	}
	if (sign == 2) // 90度路口    右转90度
	{
		pwm_duty(leftA, num[7]);
		pwm_duty(leftB, 0);

		pwm_duty(righA, 255);
		pwm_duty(righB, 255);
		delay_ms(delaytime);
		sign = 0; // sign = 0，跳出转向
	}
}
void straight_S_line() ////直线和S弯输出控制电机PWM
{
	L_motor_speed = motor_speed + error; //根据误差调节速度
	R_motor_speed = motor_speed - error;

	if (L_motor_speed < -255) //设置范围，避免超过255值
	{
		L_motor_speed = -255;
	}
	if (L_motor_speed > 255)
	{
		L_motor_speed = 255;
	}
	if (R_motor_speed < -255)
	{
		R_motor_speed = -255;
	}
	if (R_motor_speed > 255)
	{
		R_motor_speed = 255;
	}

	if (R_motor_speed > 0)
	{
		pwm_duty(righA, R_motor_speed);
		pwm_duty(righB, 0);
	}
	else //如果速度值为负值，电机反向转。
	{
		pwm_duty(righA, 0);
		pwm_duty(righB, -R_motor_speed);
	}

	if (L_motor_speed > 0)
	{
		pwm_duty(leftA, L_motor_speed);
		pwm_duty(leftB, 0);
	}
	else
	{
		pwm_duty(leftA, 0);
		pwm_duty(leftB, -L_motor_speed);
	}
}
