#include "headfile.h"	//���ں��ҵ���ٶ�
#define righA PWM1P_P60 //����
#define righB PWM2P_P62
#define leftA PWM3P_P64 //����
#define leftB PWM4P_P66
//������⴫�������
#define hw1 P06
#define hw2 P04
#define hw3 P03
#define hw4 P05
#define hw5 P07
#define hw6 P02

//���Ὺ��
#define UP P24
#define STOPP P61
#define DWN P26
#define LFT P74
#define RHT P76
#define SET P75
#define BOTTOM 1
#define TOP 8
//���Ὺ��
//��Ļ��ʾ
int num[7] = {0, 0, 0, 0, 0, 0, 0};
uint8 k; //��Ļָ��
int i;
int j;
int time1;
int startcar = 1;
int start2 = 1;
//===================================================================
int tmp1, tmp2;
//��Ļ��ʾ�ַ�
char str[][12] = {"Delay:", "WT", "CT:", "Speed", "Small", "Midle:", "Big:", "Turn:"};
//========================================================================================
int delaytime;
float wt, ct;					   //�ٶȵ��ڲ�������
int sign;						   //·���жϱ�־
int error = 0;					   //�ٶ�������ֵ
int hw[7] = {0, 0, 0, 0, 0, 0, 0}; //�洢������״̬
int motor_speed;				   //С���ٶ�
int L_motor_speed;				   //���ں������ٶ�
int R_motor_speed;				   //���ں��ҵ���ٶ�

void sign_crossing();	//�ж�·������
void m_speed_change();	//����ٶȵ���
void turn();			//�������ת��
void straight_S_line(); //ֱ�ߺ�S��������Ƶ��PWM
//===========================================================================
//��Ļ��ʾ����
void showdata();
void keyboard();
void writedata(); // eeprom write
void readdata();  // eeprome read
//============================================================================================
void main()
{
	DisableGlobalIRQ();
	board_init();
	iap_init();				//��ʼ��EEPROM
	lcd_init();				// lcd��ʼ��
	pit_timer_ms(TIM_1, 3); //������
	// pit_timer_ms(TIM_1, 100);
	k = 1;
	tmp1 = 0;
	tmp2 = 8;

	//���ᰴ����������
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
	readdata(); //��ȡeeprom�е�����
	EnableGlobalIRQ();
	//������� ���ٶ�Ϊ0
	pwm_init(righA, 1000, 0);
	pwm_init(righB, 1000, 0);
	pwm_init(leftA, 1000, 0);
	pwm_init(leftB, 1000, 0);
	//����ǰ��Ļ��� ����
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
	//���Ӳ�����ʼ��
	motor_speed = num[3];
	wt = num[1];
	ct = num[2];
	delaytime = num[0];
	time1 = 0;
	pit_timer_ms(TIM_3, 1000);
	while (start2)
	{
		sign_crossing();   //�ж�·������
		m_speed_change();  //����ٶȵ���
		turn();			   //�������ת��
		straight_S_line(); //ֱ�� ������Ƶ��PWM
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
	{ //��eeprom��д������
		writedata();
	}
	else if (STOPP == 0)
	{
		startcar = 0;
	}
}
void writedata()
{
	iap_erase_page(0x00); //�����������
	iap_write_bytes(0X00, (uint8 *)num, (TOP - BOTTOM + 1) * sizeof(int));
}
void readdata()
{
	uint16 adddd = 0x00; //д����������
	iap_read_bytes(0x00, (uint8 *)num, (TOP - BOTTOM + 1) * sizeof(int));
}

void sign_crossing() //�ж�·�����ͣ������߼���ͬʱɨ�赽���ߣ��ұ߼���û��ɨ�赽���ߣ���Ϊ��ֱ��ת�䣬��֮��ֱ��ת�䡣signΪ·��״̬���
{
	//-----------------------------------------------ֱ��·���ж�----------------------------------------------------

	if ((hw[0] == 1) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{ //��90��·��  1 1 1  0 0 0    ��ת90��
		sign = 1;
	}
	else if ((hw[0] == 0) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{ //��90��·��  0 1 1  1 0 0    ��ת90��
		sign = 1;
	}
	else if ((hw[0] == 1) && (hw[1] == 1) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{ //��90��·��  1 1 1  1 0 0    ��ת90��
		sign = 1;
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 0))
	{ //��90��·�� 0 0 0  0 1 1     ��ת90��
		sign = 2;
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 1))
	{ //��90��·�� 0 0 0  1 1 1     ��ת90��
		sign = 2;
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 1))
	{ //��90��·�� 0 0 1   1 1 1     ��ת90��
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
void m_speed_change() //�ٶȵ��ڣ�error ���ٶȵ�����
{

	if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 1))
	{
		error = ((float)num[6] / 10) * ct; //          0 0 0 0 0 1  error ���ٶȵ�����
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 1) && (hw[5] == 0))
	{
		error = ((float)num[5] / 10) * ct; //          0 0 0 0 1 0
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = ((float)num[4] / 10) * wt; //          0 0 0   1 0 0  �м��������ұ�ɨ��������ٶȵ���һ���
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 1) && (hw[4] == 1) && (hw[5] == 0))
	{
		error = ((float)num[4] / 10) * wt; //          0 0 0   1 1 0  �м��������ұ�ɨ��������ٶȵ���һ���
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 1) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = 0; //         0 0 1   1 0 0  �м��������⴫�������С�ں��ߣ����ͬʱɨ����0���ֱ��
	}
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 1) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{
		error = -((float)num[4] / 10) * wt; //         0 0 1   0 0 0  �м����������ɨ�����ұ��ٶȵ���һ���
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
	//-----------------------------------------------С������״̬��Ӧ----------------------------------------------------//
	else if ((hw[0] == 0) && (hw[1] == 0) && (hw[2] == 0) && (hw[3] == 0) && (hw[4] == 0) && (hw[5] == 0))
	{											   // 0 0 0 0    0 0 0 0
												   /*������ǵ�С������ʱ������󲿷��Ǽ�ת��·��������
												   ������һ�����ұ�ɨ�赽�ߣ�˵������������ߣ���ô��
												   ���ҹյ��ٶȣ�������һ�������ɨ�赽�ߣ�˵��������
												   ���ұߣ���ô�Ӵ���յ��ٶ�*/
		if (error == (-((float)num[6] / 10) * ct)) //�ж��ϴ����������ϴ����ұ�ɨ�赽�ߣ��Ӵ��ҹյ��ٶ�
		{
			error = (-3 * ct);
		}
		else if (error == (((float)num[6] / 10) * ct)) //�ж��ϴ����������ϴ������ɨ�赽�ߣ��Ӵ���յ��ٶ�
		{
			error = 3 * ct;
		}
	}
}
void turn() //�������ת��
{

	if (sign == 1) // 90��·��    ��ת90��
	{
		pwm_duty(leftA, 255);
		pwm_duty(leftB, 255);

		pwm_duty(righA, num[7]);
		pwm_duty(righB, 0);
		delay_ms(delaytime);
		sign = 0; //ʹ·��״̬��Ǳ�Ϊһ��void turn () �ж���û�е�����ʹ������ת�򣬷��߻�һֱִ����ת90�ȣ�ԭ�ش�ת������ͬ��
	}
	if (sign == 2) // 90��·��    ��ת90��
	{
		pwm_duty(leftA, num[7]);
		pwm_duty(leftB, 0);

		pwm_duty(righA, 255);
		pwm_duty(righB, 255);
		delay_ms(delaytime);
		sign = 0; // sign = 0������ת��
	}
}
void straight_S_line() ////ֱ�ߺ�S��������Ƶ��PWM
{
	L_motor_speed = motor_speed + error; //�����������ٶ�
	R_motor_speed = motor_speed - error;

	if (L_motor_speed < -255) //���÷�Χ�����ⳬ��255ֵ
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
	else //����ٶ�ֵΪ��ֵ���������ת��
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
