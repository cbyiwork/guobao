#include "dsp.h"
#include "../../hardware/oled/oled.h"
#include "font.h"
#include "../../hardware/uart/uart.h"

u8 ram_index_x = 0;
u8 ram_index_y = 0;

u8 last_line=0;
u8 increase,decrease,up,down,left,right;
u8 WW[11][6];
// 曲线运算
u8 curve_buff[51];
u8 index_number=0;
u8 index_change;
char last_index;
char CH_index[3];

//通道运算
int operation[8];

char CH_middle[8];
u8 ratio_mode;
u8 CH_ratio[2][8];
u8 New_ratio[2][4]={60,60,60,60,60,60,60,60};
u8 CH_inverted[8];
u8 CH_middle_change=0;


u8 mixer_name;
u8 mixer1,mixer2,mixer3,mixer4;
u8 mixer1_same,mixer1_contrary;
u8 mixer2_same,mixer2_contrary;
u8 mixer3_same,mixer3_contrary;
u8 mixer4_same,mixer4_contrary;

u8 point_num=3,curve_num=0;
u8 turnning_point[5][7];
u16 throttle;

u8 IN_mapping[8];

u16 CH1_max=600,CH2_max=600,CH3_max=600,CH4_max=600;
u16 CH1_min=400,CH2_min=400,CH3_min=400,CH4_min=400;
u8 CH1_offset=51,CH2_offset=51,CH3_offset=51,CH4_offset=51;
u16 CH_value[4];

//PPM

u8 T_l,T_h;
u8 t_ppm=0,t_PPMout=0;
int PPM_DATA[8]={0,0,0,0,0,0,0,0};
int PPM_buff[8];
u16 PPMout[8]={500,600,800,200,1000,0,100,300};
u8 PPM_mapping[8]={0,1,2,3,4,5,6,7};

u8 train_switch=0;
u8 train_output;
u8 PPM_updata=0;
char PPM_offset=0;

//ADC
u16 adc[8],adc_buff[8];
u8 t_adc=0;
u8 Battery_1s=1,VLD1=35,VLD2=70;
u16 battery_voltage;
char v_trim1=0,v_trim2=0;
u8 USB_5V;

// 开关
u8 Switch_A,Switch_B,Switch_C,Switch_D,Switch_E;
u8 Combination_Switch=0,Com_Swt_IN1=0,Com_Swt_IN2=0;
u8 Com_Swt_out[6]={16,29,42,55,68,81};
u8 Com_Swt_mapping=4;
u8 Return_alarm_switch=0;

// 自动控制
u8 auto_switch=0,auto_mode=2;
char auto_middle=-100,auto_star=-50,auto_end=50;
u8 auto_speed=10;
int auto_value=0;
u16 auto_max,auto_min;
u8 auto_h,auto_l=0;

// 硬件配置
//u8 Right_throttle;

u8 throttle_lock,Throttle_check=1,Auto_lock,Trim_step;
u8 tt=0;
u8 middle_change_last;

u16 value0,value1;


//蜂鸣器
u8 Message_AlarmClock,Message_LowPower;
u8 Message_KeyTone=0,Message_DataReturn_bad=0;
u8 beep=0,sound=0;  
u8 Sound_enable=1;	

// 时钟
u8 event1,event2,event3;
u8 t1=0,t2=0,t3=0;

u8 flag_20ms;


u8 Master_clock,second=0,minute=0,hour=0;

u8 clock2,second2=0,minute2=0;
u8 clock_set=5,clock_switch=0;
u8 clock_go;


u16 throttle_idle=0;
u8 delay_time=0;
u8 flash_time=0;

void location(u8 x, u8 y) 
{
	ram_index_x = x;
	ram_index_y = y;
}

void LCD(u8 data) {
	OLED_GRAM[ram_index_x++][ram_index_y] = data;
	if (ram_index_x >= 128) {
			ram_index_x = 0;
	}
}

void LCD_clean(u8 x,u8 y,u8 length,u8 width)// 清空指定区域
{
	u8 i,j;
  for(i=0;i<width;i++)
  {
		location(x,i+y);
		for(j=0;j<length;j++)
	  LCD(0);
  }
}

void send(u8 k)
{
	u8 j;
	for(j=0;j<6;j++)
   	LCD(font[k-32][j]);		
}

void send2(u8 k,u8 x,u8 y)		 // 写大字母
{
	u8 i;
	k-=32;
	location(x,y);
	for(i=0;i<8;i++)
	{
		LCD(font_big[k][i]);	
	}
	location(x,y+1);
	for(i=8;i<16;i++)
	{
		LCD(font_big[k][i]);	
	}
}

void send3(u8 *p,u8 x,u8 y)		 //写汉字
{
	u8 i;
	location(x,y);
	for(i=0;i<12;i++)
	{
		LCD(*p),p++;	
	}
	location(x,y+1);
	for(i=0;i<12;i++)
	{
		LCD(*p),p++;	
	}
}

void write0(u8 *d,u8 x,u8 y)			// 显示汉字串
{

	while(*d)
	{		
		send3((u8*)hzk[*d],x,y);
		d++;x+=12;
	}

}

void display_vitical_trim1(u8 v,u8 base)// 显示竖微调
{
	u8 i;
	unsigned long L1,L2=0,L3,L4=0;
	L1=0x03e00000;v/=5;
	for(i=0;i<v;i++)
	{
		if(L1&1)
		{
			L1>>=1,L2>>=1;
			L2|=0x80000000;
		}
		else {
			L1>>=1,L2>>=1;
		}
	}
	L2|=0x40000000;

	if(v==0) {
		L3=0x00100000,L4=0;
	}	else if(v==50){
		L3=0,L4=0x00000100;
	}	else {
		L3=0x02080000;
		L4=0;
		for(i=1;i<v;i++)
		{
			if(L3&1)
			{
				L3>>=1,L4>>=1;
				L4|=0x80000000;
			} else {
				L3>>=1,L4>>=1;
			}
		}
	}
	L4|=0x40000000;

	location(base,7);LCD(L1>>24);LCD(L3>>24);LCD(0x03);LCD(L3>>24);LCD(L1>>24);
	location(base,6);LCD(L1>>16);LCD(L3>>16);LCD(0xff);LCD(L3>>16);LCD(L1>>16);
	location(base,5);LCD(L1>>8); LCD(L3>>8); LCD(0xff);LCD(L3>>8); LCD(L1>>8);
	location(base,4);LCD(L1);    LCD(L3);	 LCD(0xff);LCD(L3);    LCD(L1);
	location(base,3);LCD(L2>>24);LCD(L4>>24);LCD(0xff);LCD(L4>>24);LCD(L2>>24);
	location(base,2);LCD(L2>>16);LCD(L4>>16);LCD(0xff);LCD(L4>>16);LCD(L2>>16);
	location(base,1);LCD(L2>>8); LCD(L4>>8); LCD(0xff);LCD(L4>>8); LCD(L2>>8);  
	location(base,0);LCD(L2);	 LCD(L4);	 LCD(0xf8);LCD(L4);    LCD(L2);	   	 
}

void display_trim2(u8 v) // 显示横微调
{
	u8 i;
	char m;
	v=v/5+2;		
	for(i=0;i<55;i++)
	{		
		if(i==27) {
			LCD(0x7c);
		}	else {
			m=v-i;
			if(m==3) {
				LCD(0x38);
			} else if(m==2) {
				LCD(0x54);	
			}	else if(m==1) {
				LCD(0x54);
			} else if(m==0) {
				LCD(0x54);
			} else if(m==-1) {
				LCD(0x54);
			}	else if(m==-2) {
				LCD(0x54);
			}	else if(m==-3) {
				LCD(0x38);
			}	else {
				LCD(0x10);
			}
		}
		
	}
}

void display_throttle(u8 x)//显示油门
{
	u8 i;
	long L=0;
	x=x/8;
	for(i=0;i<x;i++)
	{
		L>>=1;
		L|=0x80000000;
	}
	L++;
	location(9,6);LCD(L>>24);LCD(L>>24);
	location(9,5);LCD(L>>16);LCD(L>>16);
	location(9,4);LCD(L>>8);LCD(L>>8);
	location(9,3);LCD(L);LCD(L);
}

void warning(u8 x,u8 y,u8 w)//显示警告
{
	location(x,y);
	if(w)
	{
		LCD(0x40);LCD(0x60);LCD(0x50);LCD(0x48);LCD(0x44);LCD(0x42);LCD(0x6d);
		LCD(0x42);LCD(0x44);LCD(0x48);LCD(0x50);LCD(0x60);LCD(0x40);	
	}
	else
	{
		LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);
		LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);LCD(0x00);
	}
	
}

void write_num100(int num,u8 x,u8 y,u8 c) //显示1000以内的数字，c 为类型选择
{
	if(c=='-'||c=='/'||c=='#')
	{
		if(num>0){
			send2('+',x-8,y);
		}	else if(num<0) {
			num=-num,send2('-',x-8,y);
		}	else {
			send2(' ',x-8,y);
		}
	}
	if(num>99) {
		send2(num/100+48,x,y);
	}	else {
		send2(' ',x,y);
	}
	send2(num%100/10+48,x+8,y);
	if(c=='.'||c=='#')
	{
		send2('.',x+16,y);
		send2(num%10+48,x+24,y);
		if(c=='#')send2('%',x+32,y);
		
	}
	else
	{
		send2(num%10+48,x+16,y);
		if(c=='%'||c=='/')send2('%',x+24,y);
	}
	
}

void write_num1000(int num,u8 c)// 显示10000以内的数字
{		
	
	if(num>999)
	{
		if(c==0)send(49);
		else num/=10;
		
		send(num%1000/100+48);							
		send(num%100/10+48);
			if(c=='.')LCD(0x00),LCD(0x40);			
		send(num%10+48);
	}
	else 
	{
		if(c==0)send(' ');
		else if(c=='-')
		{
			if(num>0)send('+');
			else if(num<0)num=-num,send('-');
			else send(' ');
		}
		send(num%1000/100+48);
			if(c=='.')LCD(0x00),LCD(0x40);				
		send(num%100/10+48);			
		send(num%10+48);
	}	
}

u8 move_cursor(u8 line,u8 line_max)// 光标移动
{
	if(last_line!=line) {
		LCD_clean(0,0,128,8);
	}
	last_line=line;
	
	if(up)
	{
		up=0;
		if(line>0) {
			line--;
		}	else {
			line=line_max;
		}
	}
	if(down)
	{
		down=0;
		if(line<line_max) {
			line++;
		} else {
			line=0;
		}
	}
	
	return line;
}

void display_menu(u8 line,u8 line_max)//????
{
	
	if(line_max<4)send3((u8*)hzk[1],0,line+line);
	else 
	{
		if(line<3)send3((u8*)hzk[1],0,line+line);
		else if(line==line_max)send3((u8*)hzk[1],0,6);
		else send3((u8*)hzk[1],0,4);
	}
	
	if(line<3)
	{
		if(line_max==2)
		{
			write0(WW[0],12,0);
			write0(WW[1],12,2);
			write0(WW[2],12,4);
		}
		else
		{
			write0(WW[0],12,0);
			write0(WW[1],12,2);
			write0(WW[2],12,4);
			write0(WW[3],12,6);	
		}
		
	}
	else if(line==line_max)
	{
		write0(WW[line-3],12,0);
		write0(WW[line-2],12,2);
		write0(WW[line-1],12,4);
		write0(WW[line],12,6);
	}
	else if(line<line_max)
	{
		write0(WW[line-2],12,0);
		write0(WW[line-1],12,2);
		write0(WW[line],12,4);
		write0(WW[line+1],12,6);
	}
	
	
	if(line_max>=9)
	{
		location(97,7);
		if(line>=9) {
			send('1'),send(line+49-10);
		}	else {
			send(' '),send(line+49);
		}		
		send('/');
		send('1');
		send(line_max+49-10);
	}
	else
	{
		location(109,7);
		send(line+49);
		send('/');
		send(line_max+49);
	}	
		
}

void display_mapping(u8 mode,u8 x,u8 y)// 显示映射对象
{
	switch(mode)
	{
		case 0:send3((u8*)hzk[96],x,y);send3((u8*)hzk[97],x+12,y);send2('1',x+24,y);break;
		case 1:send3((u8*)hzk[96],x,y);send3((u8*)hzk[97],x+12,y);send2('2',x+24,y);break;
		case 2:send3((u8*)hzk[96],x,y);send3((u8*)hzk[97],x+12,y);send2('3',x+24,y);break;
		case 3:send3((u8*)hzk[96],x,y);send3((u8*)hzk[97],x+12,y);send2('4',x+24,y);break;
		case 4:send3((u8*)hzk[100],x,y);send3((u8*)hzk[101],x+12,y);send2('1',x+24,y);break;
		case 5:send3((u8*)hzk[100],x,y);send3((u8*)hzk[101],x+12,y);send2('2',x+24,y);break;
		case 6:send3((u8*)hzk[100],x,y);send3((u8*)hzk[101],x+12,y);send2('3',x+24,y);break;
		case 7:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('A',x+24,y);break;
		case 8:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('B',x+24,y);break;
		case 9:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('C',x+24,y);break;
		case 10:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('D',x+24,y);break;
		case 11:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('E',x+24,y);break;
		
		case 12:send3((u8*)hzk[104],x,y);send3((u8*)hzk[105],x+12,y);send2('1',x+24,y);break;
/*		case 14:send3(hzk[104],x,y);send3(hzk[105],x+12,y);send2('2',x+24,y);break;*/
		case 13:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('1',x+24,y);break;
		case 14:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('2',x+24,y);break;
		case 15:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('3',x+24,y);break;
		case 16:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('4',x+24,y);break;
		case 17:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('5',x+24,y);break;
		case 18:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('6',x+24,y);break;
		case 19:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('7',x+24,y);break;
		case 20:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);send2('8',x+24,y);break;
	}
	
}

void display_receiver_mode(u8 mode,u8 x,u8 y)//显示接收机模式
{
	switch(mode)
	{
		case 0:send3((u8*)hzk[36],x,y);send3((u8*)hzk[37],x+12,y);break;
		case 1:send3((u8*)hzk[175],x,y);send3((u8*)hzk[180],x+12,y);break;
		case 2:send2('P',x,y);send2('W',x+8,y);send2('M',x+16,y);break;
		case 3:send2('P',x,y);send2('P',x+8,y);send2('M',x+16,y);break;
		case 4:send2('S',x,y);send2('.',x+8,y);send2('B',x+16,y);break;
	}	
}

void display_switch(u8 sw,u8 x,u8 y)//显示开关
{
	switch(sw)
	{
		case 0:send2(' ',x,y);send2('-',x+4,y);send3((u8*)hzk[10],x+12,y);send2('-',x+24,y);break;
		case 1:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('A',x+24,y);break;
		case 2:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('B',x+24,y);break;
		case 3:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('C',x+24,y);break;
		case 4:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('D',x+24,y);break;
		case 5:send3((u8*)hzk[58],x,y);send3((u8*)hzk[59],x+12,y);send2('E',x+24,y);break;
	}
}

void display_bar(int *p)// 显示条形图
{
	u8 i,j;
	for(i=0;i<8;i++)
	{
		location(20,i);
		LCD(0x3e);
		LCD(0x41);
		for(j=0;j<103;j++)
		{
			
			if(j==8) {
				LCD(0x7f);
			} else if(j==51) {
				LCD(0x7f);
			} else if(j==94) {
				LCD(0x7f);
			} else if(j>51) {
				if(j>*p/10){
					LCD(0x41);
				} else {
					LCD(0x5d);
				}
			} else if(j<51)
			{
				if(j>*p/10) {
					LCD(0x5d);
				}
				else {
					LCD(0x41);
				}
			}	
			
		}
		LCD(0x41);
		LCD(0x3e);
		p++;
	}
}

void point(u8 x,u8 z,u8 line)			  //画点
{
	u8 d0=0,d1=0,d2=0,d3=0,d4=0,d5=0,d6=0,d7=0;
	
	if(z<8)
	{
		d0=0x80;d0>>=z;			
	}
	else if(z<16)
	{
		d1=0x80;d1>>=z-8;	
	}
	else if(z<24)
	{
		d2=0x80;d2>>=z-16;	
	}
	else if(z<32)
	{
		d3=0x80;d3>>=z-24;	
	}
	else if(z<40)
	{
		d4=0x80;d4>>=z-32;	
	}
	else if(z<48)
	{
		d5=0x80;d5>>=z-40;	
	}
	else if(z<56)
	{
		d6=0x80;d6>>=z-48;	
	}
	else if(z<64)
	{
		d7=0x80;d7>>=z-56;	
	}
	
	if(x==line)
	{
		d0=0x3f,d1=0xff,d2=0xff,d3=0xff,d4=0xff,d5=0xff,d6=0xff,d7=0xf8;
	}
	if(x<3)
	{
		d0|=0x80;d7|=0x02;
	}
	else if(x>47)
	{
		d0|=0x80;d7|=0x02;
	} 
	
	
	d3|=0x01;
	
	if(x==0)
	{
		location(73,0);LCD(0x1e);LCD(0x02);
		location(73,7);LCD(0xf0);LCD(0x80);
		location(126,0);LCD(0x02);LCD(0x1e);
		location(126,7);LCD(0x80);LCD(0xf0);	
	}
	
	location(75+x,0);LCD(d7);
	location(75+x,1);LCD(d6);
	location(75+x,2);LCD(d5);
	location(75+x,3);LCD(d4);
	location(75+x,4);LCD(d3);
	location(75+x,5);LCD(d2);
	location(75+x,6);LCD(d1);
	location(75+x,7);LCD(d0);
}

void display_curve() //显示指数曲线
{
	u8 i;
		

	point(25,31,25);
	for(i=0;i<25;i++)
	{

		point(i+26,curve_buff[i]+31,25);
		point(24-i,31-curve_buff[i],25);		

	}
	
}

void display_curve2() //显示折线曲线
{
	u8 i,n;
	

	if(point_num==0)n=0;
	if(point_num==1)n=8;
	if(point_num==2)n=16;
	if(point_num==3)n=25;
	if(point_num==4)n=34;
	if(point_num==5)n=42;
	if(point_num==6)n=50;

	for(i=0;i<51;i++)
	{

		point(i,curve_buff[i]+6,n);
		
	}
	
}

void Xdata_check(int *p)//
{
	u8 i;
	for(i=0;i<8;i++)
	{
		if(*p>1023)*p=1023;
		if(*p<0)*p=0; 
		p++;
	}
	
}

u8 rounding(float x)// 四舍五入
{
	u8 y;
	y=x;
	if((x-y)>0.5)y++;
	return y;
}

void get_curve(float index) // 计算指数曲线
{
	u8 i;
	
	if(index>=0)
	{
		index=index/100+1;
		
		for(i=1;i<26;i++)
		{

			
			curve_buff[i-1]=rounding(pow(i*0.04,index)*25);

		}
	}
	else if(index<0)
	{
		index=-index/100+1;
		for(i=1;i<26;i++)
		{

			
			curve_buff[i-1]=rounding(25-pow(1-i*0.04,index)*25);

		}
	}
	
}

void get_curve2()// 油门曲线数据
{
	
	int i;
	u8 b[7];
	for(i=0;i<7;i++)
	{
		b[i]=turnning_point[curve_num][i]/2;
	}
	
	
	for(i=0;i<51;i++)
	{
		if(i<8)
		{

			curve_buff[i]=i*(b[1]-b[0])/8+b[0];
		}
		else if(i<16)
		{

			curve_buff[i]=(i-8)*(b[2]-b[1])/8+b[1];
		}
		
		else if(i<25)
		{

			curve_buff[i]=(i-16)*(b[3]-b[2])/9+b[2];
		}
		else if(i<34)
		{

			curve_buff[i]=(i-25)*(b[4]-b[3])/9+b[3];
		}
		else if(i<42)
		{

			curve_buff[i]=(i-34)*(b[5]-b[4])/8+b[4];
		}
		else
		{

			curve_buff[i]=(i-42)*(b[6]-b[5])/8+b[5];
		}			
	}
}

void function_size(u8 num,u8 mode)// 舵量函数
{
	u8 r0,r1;
	if(mode)
	{
		r0=New_ratio[0][num];
		r1=New_ratio[1][num];
	}
	else
	{
		r0=CH_ratio[0][num];
		r1=CH_ratio[1][num];
	}
	
	if(operation[num]>0)
	{
		operation[num]=(long)operation[num]*r1/100;
	}
	else
	{
		operation[num]=(long)operation[num]*r0/100;
	}
	
}

void function_inverted(u8 num)// 反转函数
{
	if(CH_inverted[num]==1) {
		operation[num]=-operation[num]+CH_middle[num];
	}
	else {
		operation[num]+=CH_middle[num];
	}
}

void function_mix(u8 same,u8 contrary)//混控函数
{
	int y1,y2;
	
	y1=operation[same]+operation[contrary];
	y2=operation[same]-operation[contrary];
	
	operation[same]=y1;
	operation[contrary]=y2;
			
}

void function_curve(u8 num)// 指数曲线
{
	float Index;int temp;
	
	if(num==2)num++;
	if(CH_index[num]>0)
	{
		Index=(float)CH_index[num]/100+1;
	
		temp=operation[num];
		if(temp<0)temp=-temp;

		temp=pow(0.0023474*temp,Index)*426;
		
		
		if(operation[num]<0)temp=-temp;
		
		operation[num]=temp;	
		
	}
	else if(CH_index[num]<0)
	{
		Index=(float)-CH_index[num]/100+1;
		
		temp=operation[num];
		if(temp<0)temp=-temp;

		temp=426-pow(1-0.0023474*temp,Index)*426;
		
		
		if(operation[num]<0)temp=-temp;
		
		operation[num]=temp;
	}
	
}

void function_curve2(u8 num)//多点曲线
{
	u16 temp;
	int b[7];
	u8 i;
	for(i=0;i<7;i++)
	{
		b[i]=turnning_point[num][i]*17/2;
	}
	

	
	if(num==0)temp=operation[2]+426;
	else temp=operation[num+3]+426;

	

		if(temp<136)
		{

			temp=(long)temp*(b[1]-b[0])/136+b[0];
		}
		else if(temp<272)
		{

			temp=(long)(temp-136)*(b[2]-b[1])/136+b[1];
		}
		
		else if(temp<426)
		{

			temp=(long)(temp-272)*(b[3]-b[2])/154+b[2];
		}
		else if(temp<580)
		{

			temp=(long)(temp-426)*(b[4]-b[3])/154+b[3];
		}
		else if(temp<716)
		{

			temp=(long)(temp-580)*(b[5]-b[4])/136+b[4];
		}
		else
		{

			temp=(long)(temp-716)*(b[6]-b[5])/136+b[5];
		}			
	
	
		
		
	if(num==0)throttle=temp,operation[2]=temp-426;
	else operation[num+3]=temp-426;
		
}

void function_mapping(u8 num)//输入映射
{

	switch(IN_mapping[num])
	{
		case 0:operation[num]=CH_value[0]*5/6-426;break;
		case 1:operation[num]=CH_value[1]*5/6-426;break;
		case 2:operation[num]=CH_value[2]*5/6-426;break;
		case 3:operation[num]=CH_value[3]*5/6-426;break;
		case 4:operation[num]=adc[1]*5/6-426;break;
		case 5:operation[num]=adc[2]*5/6-426;break;
		case 6:operation[num]=adc[3]*5/6-426;break;
		case 7:if(Switch_A==1)operation[num]=-426;
						else if(Switch_A==2)operation[num]=0;
						else operation[num]=426;break;
		case 8:if(Switch_B==1)operation[num]=-426;
						else if(Switch_B==2)operation[num]=0;
						else operation[num]=426;break;
		case 9:if(Switch_C==1)operation[num]=-426;
						else if(Switch_C==2)operation[num]=0;
						else operation[num]=426;break;
		case 10:if(Switch_D==1)operation[num]=-426;
						else if(Switch_D==2)operation[num]=0;
						else operation[num]=426;break;
		case 11:if(Switch_E==1)operation[num]=-426;
						else if(Switch_E==2)operation[num]=0;
						else operation[num]=426;break;
						
		case 12:operation[num]=auto_value-426;break;				
		case 13:operation[num]=PPM_DATA[0]-426;break;
		case 14:operation[num]=PPM_DATA[1]-426;break;
		case 15:operation[num]=PPM_DATA[2]-426;break;
		case 16:operation[num]=PPM_DATA[3]-426;break;
		case 17:operation[num]=PPM_DATA[4]-426;break;
		case 18:operation[num]=PPM_DATA[5]-426;break;
		case 19:operation[num]=PPM_DATA[6]-426;break;
		case 20:operation[num]=PPM_DATA[7]-426;break;
	
	}
}

void change_trim(u8 num,u8 direction)  // 按键微调
{
	if(direction)
	{
		if(CH_inverted[num]==0)
		{
			if(CH_middle[num]<125)
			{
				if(Trim_step)CH_middle[num]++;
				else if(CH_middle[num]%5)CH_middle[num]+=(5-CH_middle[num]%5);
				else CH_middle[num]+=5;	
				
		//		Data_change(2);
			}
		}
		else
		{
			if(CH_middle[num]>-125)
			{
				if(Trim_step)CH_middle[num]--;
				else if(CH_middle[num]%5)CH_middle[num]-=(CH_middle[num]%5);
				else CH_middle[num]-=5;
				
		//		Data_change(2);
			}
				
			
		}	
	}
	else
	{
		if(CH_inverted[num]==0)
		{
			if(CH_middle[num]>-125)
			{
				if(Trim_step)CH_middle[num]--;
				else if(CH_middle[num]%5)CH_middle[num]-=(CH_middle[num]%5);
				else CH_middle[num]-=5;
				
		//		Data_change(2);
			}
				
			
		}
		else
		{
			if(CH_middle[num]<125)
			{
				if(Trim_step)CH_middle[num]++;
				else if(CH_middle[num]%5)CH_middle[num]+=(5-CH_middle[num]%5);
				else CH_middle[num]+=5;
				
		//		Data_change(2);
			}
				
			
		}	
	}

	
	if(CH_middle[num]==0)Message_KeyTone=2;
	CH_middle_change=num,event1=1,t1=0;
}


int CH_calib(int x,u8 offset)				//修正摇杆行程
{

	x=512+(x-512)*51/offset;

	if(x>1023)x=1023;
	if(x<0)x=0;
	return x;		
}

void function_filter(u8 num)  //ADC滤波
{
	if(num==0)
	{
		adc[0]=(adc_buff[0]+(long)adc[0]*9/10);
	}
//	else adc[num]=(adc_buff[num]*5+adc[num]*5)/10;
	else adc[num]=adc_buff[num];
}

u8 Switch_Check(u8 num)// 获取开关状态
{
	switch(num)
	{
		case 1:num=Switch_A;break;
		case 2:num=Switch_B;break;
		case 3:num=Switch_C;break;
		case 4:num=Switch_D;break;
		case 5:num=Switch_E;break;
	}
	return num;
}

void photo(u8 *p,u8 x,u8 y,u8 length,u8 width)//显示指定大小的图片
{
	u8 i,j;
	for(i=0;i<width;i++)
	{
		location(x,i+y);
		for(j=0;j<length;j++)
	  	LCD(*p),p++;
	}  	
}

void LcdClear(u8 x, u8 y, u8 w, u8 h) {
	
	u8 i,j;
	for(i=0;i<h;i++)
	{
		location(x,i+y);
		for(j=0;j<w;j++)
	  	LCD(0x00);
	}  	
}

#define FONT_W 8
void DspVolNum(u8 x, u8 y, u8 num) {
	u8* pf;
	if (num == '%') {
		num = 10;
	}
	pf = (u8*)font_num16[num];
	photo(pf,x,y,FONT_W,2);
	
}

void DspVolNumStr(u8 x, u8 y, u8 percent) {

	//u8 idx = x;
	if (percent >= 100) {
		DspVolNum(x,y,1);
		percent%=100;
	}
	x+=FONT_W;
	DspVolNum(x,y,percent/10);
	x+=FONT_W;
	DspVolNum(x,y,percent%10);
	x+=FONT_W;
	DspVolNum(x,y,'%');
}

void DspBatteryLevel(u8 x, u8 y, u8 level) {
	if (level>=4) {
		photo((u8*)bat_full,x,y,18,2);
	} else if(level>=3) {
		photo((u8*)bat_80,x,y,18,2);
	} else if(level>=2) {
		photo((u8*)bat_50,x,y,18,2);
	} else if(level>=1) {
		photo((u8*)bat_20,x,y,18,2);
	} else {
		photo((u8*)bat_low,x,y,18,2);
	}
}

u32 getRevert(u32 dat) {

	u8 i = 0;
	u32 ret = 0;
	for(i=0;i<24;i++) {

		if ((dat>>i)&0x01) {
			ret |= (1<<(23-i));
		}
		
	}
	return ret;

}

void DspVerticalBar(u8 x, u8 y, u8 dir, u8 pow) {

	u8 rows = 0, i=0;
	u8 buf[24];
	u32 tmp = 0;

	memcpy(buf, vertical_bar,24);
	
	if (dir == 0) {
		rows = pow/4;
		if (rows > 24) rows = 24;		
		//tmp = (((1<<rows)-1)<<(24-rows));
		tmp = (0xffffff>>(24-rows));
		tmp = getRevert(tmp);
		
		for (i=0;i<4;i++) {
			buf[i+8]|=(tmp>>16)&0xff;
			buf[i+4]|=((tmp>>8)&0xff);
			buf[i]|=(tmp&0xff);
		}
	} else {
		rows = pow/4;
		if (rows > 24) rows = 24;	
		tmp = (0xffffff>>(24-rows));
		for (i=0;i<4;i++) { 
			buf[i+12+8]|=(tmp>>16);
			buf[i+12+4]|=((tmp>>8)&0xff);
			buf[i+12]|=(tmp&0xff);
		}
	}

	photo(buf,x,y,4,6);

}

void DspHorizontalBar(u8 x, u8 y,u8 dir, u8 pow) {

	u8 buf[48];
	u8 rows = 0, i=0;

	rows = pow/4;
	if (rows>24) rows = 24;

	memcpy(buf, horizontal_bar, 48);

	if (dir > 0) {

		for (i=0;i<rows;i++) {
			buf[24-i] |= 0x3c;
		}

	} else {
	
		for (i=0;i<rows;i++) {
			buf[i+24] |= 0x3c;
		}
	}
	
	photo(buf,x,y,48,1);
	
}

void DspQrudNum(u8 x, u8 y, u8 num) {
	u8* pf;
	if (num == '%') {
		num = 10;
	}
	pf = (u8*)font_num8[num];
	photo(pf,x,y,6,1);
	
}

void DspQrudStr(u8 x, u8 y, u8 percent) {

	//u8 idx = x;
	LcdClear(x,y,24,1);
	if (percent >= 100) {
		percent = 100;
		DspQrudNum(x,y,1);
		//percent%=100;
		x+=6;
	}
	if (percent>=10)
	{
		DspQrudNum(x,y,(percent%100)/10);
		x+=6;
	}
	DspQrudNum(x,y,percent%10);

	if(percent>0){
		x+=6;
		DspQrudNum(x,y,'%');
	}
}

void DspDirLeftRight(u8 x, u8 y,u8 dir, u8 pow) {

	u8* pf;

	if (!pow) {
		LcdClear(x,y,6,1);
		return;
	}

	if (dir > 0) {
		pf = (u8*)font_num8[13];
	} else {
		pf = (u8*)font_num8[14];
	} 
	
	photo(pf,x,y,6,1);
 	
}

void DspDirUpDown(u8 x, u8 y,u8 dir, u8 pow) {

	u8* pf;

	if (!pow) {
		LcdClear(x,y,6,1);
		return;
	}

	if (dir > 0) {
		pf = (u8*)font_num8[12];
	} else {
		pf = (u8*)font_num8[11];
	} 

	photo(pf,x,y,6,1);
 	
}

#define FUNC_START 30
#define H_W 12
#define DUMMY 5
#define SIGNAL_W 16
#define NUM_LEN 8
void dsp_test() {
	//photo((u8*)screen2,0,0,128,8);     // 模块错误
	//photo((u8*)screen1,0,0,128,8);
	photo((u8*)icon_h,FUNC_START,0,12,2);
	photo((u8*)icon_signal,FUNC_START+H_W+DUMMY,0,16,2);
	DspVolNumStr(FUNC_START+H_W+DUMMY+SIGNAL_W+DUMMY,0,100);
	DspBatteryLevel(FUNC_START+H_W+DUMMY+SIGNAL_W+DUMMY+NUM_LEN*4+DUMMY,0,4);
	//DspVerticalBar(2,1,1,0);
}

u8 pwr = 0,dir = 0;
void bar_test() {
	//dprint("dir=%d, pwr=%d\r\n",dir,pwr);
	DspVerticalBar(3,1,dir,pwr++);
	if (pwr>=100) {
		pwr = 0;
		dir = !dir;
	}
	

}




