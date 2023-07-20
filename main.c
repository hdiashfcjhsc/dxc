/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Loongson 1B RT-Thread Sample main file
 */

#include <time.h>

#include "rtthread.h"
#include "ns16550.h"
#include "bsp.h"
#include "uart.h"
#include "i2c/gt1151.h"
#include "i2c/ads1015.h"
#include "ls1b_gpio.h"
#include "ds18b20.h"
#include "ls1x_i2c_bus.h"

#if defined(BSP_USE_FB)
  #include "ls1x_fb.h"
char LCD_display_mode[] = LCD_480x800;

 #endif

//-------------------------------------------------------------------------------------------------

extern void start_my_gui(void);
extern int can0_test_start(void);
extern int can1_test_start(void);
extern int i2c0_slaves_test_start(void);

char rbuf[256];
char at1[14] = "AT+MIPCALL=1\r\n"; //����IP
char at2[72] = "AT+TCDEVINFOSET=1,\"MHS6PDE6WM\",\"test\",\"K5l9VyWzuou0CB6tOtcyqw==\"\r\n";//����ƽ̨��Ϣ
char at3[31] = "AT+TCMQTTCONN=1,20000,240,1,1\r\n";//�������Ӳ���������
char at4[55] = "AT+TCMQTTSUB=\"$thing/down/property/MHS6PDE6WM/test\"\r\n";//�����ϱ��������Ա�ǩ
float hp_y0,hp_y1,tds_y0,tds_y1,ntu_y0,ntu_y1,tdsvalue_y0,tdsvalue_y1,wwd_y0,wwd_y1,hwd_y0,hwd_y1;//6�����ݵ�����ͼ ��ʼ�� �� ������
float ntu, tdsvalue, ph, tds, wwd = 22.4, hwd = 25.5;//�����ֵ
int cntbuf = 0;
int count;
char biaozhiwei=0;
int cut=0;
int t1=30,t2=30;
int a1;
int sxt_flag=1;
unsigned char buf[20] = {0};
//-------------------------------------------------------------------------------------------------




//����6������
void hp_shuzhi(char hp)   //phֵ
{
    hp_y1 = 721 - ph*10 ;
}
void tdsvalue_shuzhi(char tdsvalue)  //�絼��
{
    tdsvalue_y1 = 721 - tdsvalue / 10;
}
void ntu_shuzhi(float ntu)   //���ǳ̶�
{
    ntu_y1 = 721 - ntu / 10 ;
}
void tds_shuzhi(char tds)  //�����ܽ��
{
    tds_y1 = 721 - tds / 10 ;
}
void wwd_shuzhi(char wwd)//Һ���¶�
{
    wwd_y1 = 721 - wwd * 7 ;
}
void hwd_shuzhi(char hwd)  //�����¶�
{
    hwd_y1 = 721 - hwd * 7 ;
}





static void touchme(int x, int y)
{
    printk("touched at: x=%i, y=%i\r\n", x, y);
}


/* �����߳̿��ƿ� */
static rt_thread_t thread1 = RT_NULL;

/* �߳� 1 ����ں���(��Ļ��ʾ)*/
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0 ;
    //unsigned char buf[20] = {0};
    uint16_t adc_code0, adc_code1, adc_code2, adc_code3;//���յ�����
    float adc_v0, adc_v1, adc_v2, adc_v3;
    //char print_buf[]={""};
    float hp_shuju[400] = {0};
    float tds_shuju[400] = {0};
    float ntu_shuju[400] = {0};
    float tdsvalue_shuju[400] = {0};
    float wwd_shuju[400] = {0};
    float hwd_shuju[400] = {0};

    while(1)
    {
        display_clear( 85  ); //��ֵ��������
        display_clear( 140 );
        display_clear( 195 );
        display_clear( 250 );
        display_clear( 305 );
        display_clear( 360 );
/*****************************************�¶�*****************************************/
        wwd = DS18B20_Get_Temp1();	//��ȡ�¶�����
        sprintf(buf,"%.1f ",wwd);
        fb_textout(251, 315, buf);
        fb_textout(252, 315, buf);


        hwd = DS18B20_Get_Temp2();	//��ȡ�¶�����
        sprintf(buf,"%.1f ",hwd);
        fb_textout(251, 370, buf);
        fb_textout(252, 370, buf);
        
/*****************************************����*****************************************/
        adc_code0 = get_ads1015_adc(busI2C0, ADS1015_REG_CONFIG_MUX_SINGLE_0); // ��ͨ�� 0
        adc_v0 = 4096 * adc_code0 / 2048;
        adc_v0 = adc_v0 / 1000;
        tdsvalue = (133.42 * adc_v0 * adc_v0 * adc_v0 - 255.86 * adc_v0 * adc_v0 + 857.39 * adc_v0) * 0.5;

        tdsvalue=tdsvalue*(1+0.02*(wwd-25));  ///�¶�У׼
        sprintf((char *)buf, "%.1f ", tdsvalue); //�絼��
        //printk("��ǰ��ѹ = %f v \n", adc_v0);
        fb_textout(251, 205, buf);
        fb_textout(252, 205, buf);


        adc_code1 = get_ads1015_adc(busI2C0, ADS1015_REG_CONFIG_MUX_SINGLE_1); // ��ͨ�� 1
        adc_v1 = 4096 * adc_code1 / 2048;
        tds = tdsvalue * 0.55;
        sprintf((char *)buf, "%.1f ", tds); //�����ܽ��
        //printk("��ǰ��ѹ = %f mv \n", adc_v1);
        fb_textout(251, 260, buf);
        fb_textout(252, 260, buf);


        adc_code2 = get_ads1015_adc(busI2C0, ADS1015_REG_CONFIG_MUX_SINGLE_2); // ��ͨ�� 2
        adc_v2 = 4096 * adc_code2 / 2048;
        adc_v2 = adc_v2 / 1000;
        ph = -5.8887 * adc_v2 + 21.677;
        sprintf((char *)buf, "%.1f ", ph);  //phֵ
        //printk("��ǰ��ѹ = %f mv \n", adc_v2);
        fb_textout(251, 150, buf);
        fb_textout(252, 150, buf);


        adc_code3 = get_ads1015_adc(busI2C0, ADS1015_REG_CONFIG_MUX_SINGLE_3); // ��ͨ��3
        adc_v3 = 4096 * adc_code3 / 2048;
        ntu = -0.86568 * adc_v3 + 3291.3;
        sprintf((char *)buf, "%.0f ", ntu);  //���ǳ̶�
        //printk("��ǰ��ѹ = %f mm \n", adc_v3);
        fb_textout(251, 95, buf);
        fb_textout(252, 95, buf);

/*****************************************����ͼ***********************************************/
        t1 = t2;  //ʱ�����
        t2 += 1;
        if( t2 >= 430 )  //���䵽������ ���������ʱ������
        {
            //����
            display_wallpaper();
            //fb_fillrect(51 , 220 , 430 , 589 , cidxBLACK);
            t1=30;
            t2=32;
        }

        if(t1 == 30) //�趨��һ��ʱ����ʼ������
        {
            hp_y0       = 721 - ph * 10 ;       //PH
            tdsvalue_y0 = 721 - tdsvalue /10 ;  //�����ܽ��
            ntu_y0      = 721 - ntu / 10 ;      //���Ƕ�
            tds_y0      = 721 - tds / 10 ;      //�絼��
            wwd_y0      = 721 - wwd * 7 ;       //Һ���¶�
            hwd_y0      = 721 - hwd * 7 ;       //�����¶�
        }
        if(t1 != 30)   //������ʼ�������
        {
            hp_y0       = hp_y1;
            tdsvalue_y0 = tdsvalue_y1;
            ntu_y0      = ntu_y1;
            tds_y0      = tds_y1;
            wwd_y0      = wwd_y1;
            hwd_y0      = hwd_y1;
        }

        hp_shuzhi(ph);  //hp_y1���ݸ���
        tdsvalue_shuzhi(tdsvalue);
        ntu_shuzhi(ntu);
        tds_shuzhi(tds);
        wwd_shuzhi(wwd);
        hwd_shuzhi(hwd);

/*****************************���ݱ���Ͷ���*****************************/

        //��������
        hp_shuju[cut]   = hp_y0;
        hp_shuju[cut+1] = hp_y1;

        ntu_shuju[cut]   = ntu_y0;
        ntu_shuju[cut+1] = ntu_y1;

        tds_shuju[cut]   = tds_y0;
        tds_shuju[cut+1] = tds_y1;

        tdsvalue_shuju[cut]   = tdsvalue_y0;
        tdsvalue_shuju[cut+1] = tdsvalue_y1;

        wwd_shuju[cut]   = wwd_y0;
        wwd_shuju[cut+1] = wwd_y1;

        hwd_shuju[cut]   = hwd_y0;
        hwd_shuju[cut+1] = hwd_y1;

        cut++;  //�ƶ��±�
        if(t1==30)  //�ж�ʱ���Ƿ�����
        {
            cut=0;  //ʱ�����ã�������±�Ҳ����
        }

        //��������
        if(biaozhiwei == 1)
        {
            biaozhiwei=0;   //��־λ����
            t1=30;  //����ʱ��
            t2=31;
            for(cut=0 ; t2 <= 429 ; cut++)  //��ʾ�����������
            {
                fb_drawline( t1 , hp_shuju[cut]       , t2 , hp_shuju[cut+1]       , cidxBRTRED    ); //PH��ֵ
                fb_drawline( t1 , tdsvalue_shuju[cut] , t2 , tdsvalue_shuju[cut+1] , cidxGREEN     ); //�絼��
                fb_drawline( t1 , ntu_shuju[cut]      , t2 , ntu_shuju[cut+1]      , cidxBLACK     ); //���ǳ̶�
                fb_drawline( t1 , tds_shuju[cut]      , t2 , tds_shuju[cut+1]      , cidxBRTVIOLET ); //�����ܽ��;
                fb_drawline( t1 , wwd_shuju[cut]      , t2 , wwd_shuju[cut+1]      , cidxBRTBLUE   ); //Һ���¶�
                fb_drawline( t1 , hwd_shuju[cut]      , t2 , hwd_shuju[cut+1]      , cidxBRTYELLOW ); //�����¶�

                t1 = t2;  //ʱ�����
                t2 += 1;
            }
            delay_ms(5000); //�����������ʾ5��
            //����
            display_wallpaper();
            t1=30;  //����ʱ��
            t2=30;
        }
/*********************************************����ͼ**************************************************************/

        //��ʾʵ������
        fb_drawline(115 , 120 , 430 , 120 , cidxBRTRED    ); //PH��ֵ
        fb_drawline(115 , 175 , 430 , 175 , cidxGREEN     ); //�絼��
        fb_drawline(115 , 230 , 430 , 230 , cidxBLACK     ); //���ǳ̶�
        fb_drawline(115 , 285 , 430 , 285 , cidxBRTVIOLET ); //�����ܽ��
        fb_drawline(115 , 340 , 430 , 340 , cidxBRTBLUE   ); //Һ���¶�
        fb_drawline(115 , 395 , 430 , 395 , cidxBRTYELLOW ); //�����¶�


        fb_drawline(115 , 121 , 430 , 121 , cidxBRTRED    ); //PH��ֵ
        fb_drawline(115 , 176 , 430 , 176 , cidxGREEN     ); //�絼��
        fb_drawline(115 , 231 , 430 , 231 , cidxBLACK     ); //���ǳ̶�
        fb_drawline(115 , 286 , 430 , 286 , cidxBRTVIOLET ); //�����ܽ��
        fb_drawline(115 , 341 , 430 , 341 , cidxBRTBLUE   ); //Һ���¶�
        fb_drawline(115 , 396 , 430 , 396 , cidxBRTYELLOW ); //�����¶�



        //��ʾ����ͼ
        fb_drawline( t1 , hp_y0       , t2 , hp_y1       , cidxBRTRED    ); //PH��ֵ
        fb_drawline( t1 , tdsvalue_y0 , t2 , tdsvalue_y1 , cidxGREEN     ); //�絼��
        fb_drawline( t1 , ntu_y0      , t2 , ntu_y1      , cidxBLACK     ); //���ǳ̶�
        fb_drawline( t1 , tds_y0      , t2 , tds_y1      , cidxBRTVIOLET ); //�����ܽ��;
        fb_drawline( t1 , wwd_y0      , t2 , wwd_y1      , cidxBRTBLUE   ); //Һ���¶�
        fb_drawline( t1 , hwd_y0      , t2 , hwd_y1      , cidxBRTYELLOW ); //�����¶�

        memset(buf, 0, sizeof(buf));

        //������
        GT1151_Test();

        //��ʾ��������
        fb_drawrect(20,735,90,795,cidxBLACK);
        //30,745   50,40
        //392,745  74,40
        fb_drawrect(382,735,476,795,cidxBLACK);
        
        /* �߳� 1 ���õ����ȼ����У�һֱ��ӡ��ֵ */
        rt_kprintf("thread1 count: %d\r\n", count++);
        rt_thread_mdelay(500);
     
    }
}

/* �����߳̿��ƿ� */
static struct rt_thread thread2;
/* �����߳�ջ */
static unsigned char thread2_stack[1024];
/* �����߳̿�ջʱҪ�� RT_ALIGN_SIZE ���ֽڶ��� */
ALIGN(RT_ALIGN_SIZE)

/* �߳� 2 ����ں���(���ڴ���) */
static void thread2_entry(void *param)
{
    float ntu_temp, tdsvalue_temp, ph_temp, tds_temp, wwd_temp, hwd_temp;//������λ���м����
    float ph_max, ph_min, tdsvalue_max, tds_max, ntu_max;//������ֵ
    int ph_max_temp, ph_min_temp, tdsvalue_max_temp, tds_max_temp, ntu_max_temp;//������ֵ�м����
    int ph_max_flg, ph_min_flg, tdsvalue_max_flg, tds_max_flg, ntu_max_flg;//������ֵ��־λ
    rt_uint32_t count = 0;
    
    //�����ֵ��ֵ��ʼ��ֵ
    ph_max=10.2;
    ph_min=3.1;
    tdsvalue_max=100.5;
    tds_max=100.3;
    ntu_max=101.6;
    //�����ֵ��ֵ�м������ʼ��ֵ
    ph_max_temp=ph_max*10;
    ph_min_temp=ph_min*10;
    tdsvalue_max_temp=tdsvalue_max*10;
    tds_max_temp=tds_max*10;
    ntu_max_temp=ntu_max*10;
    //�����ֵ��ֵ��־λ��ʼ��ֵ
    ph_max_flg=ph_max*10;
    ph_min_flg=ph_min*10;
    tdsvalue_max_flg=tdsvalue_max*10;
    tds_max_flg=tds_max*10;
    ntu_max_flg=ntu_max*10;
    
    /* �� �� 2 ӵ �� �� �ߵ����ȼ��� �� ��ռ�� �� 1 �� �� �� ִ �� */
    while(1)
    {
/*****************************************����ͷ*****************************************/
        if (sxt_flag == 1)   //����������������ͷ
        {
            sxt();
        }
/*****************************************��ֵ����*****************************************/
        uartrs();//���ͨ�·�����
/*****************************************���ݴ�����·�*****************************************/
        ntu_temp=ntu;
        tdsvalue_temp=tdsvalue*10;
        ph_temp=ph*10;
        tds_temp=tds*10;
        hwd_temp=hwd*10;
        wwd_temp=wwd*10;

        //senddata(ph_temp,tds_temp,ntu_temp,tds_temp,hwd_temp,wwd_temp,&ph_max_temp,&ph_min_temp,&tdsvalue_max_temp,&tds_max_temp,&ntu_max_temp);
        if((ph_max_flg!=ph_max_temp)||(ph_min_flg!=ph_min_temp)||(tdsvalue_max_flg!=tdsvalue_max_temp)||(tds_max_flg!=tds_max_temp)||(ntu_max_flg!=ntu_max_temp))
        {
            ph_max_flg = ph_max_temp;
            ph_max = ph_max_temp/10.0;
        }
        sprintf((char *)buf, "(��λ��)phֵ����Ϊ%.1f ", ph_max);
        fb_textout(251, 660, buf);
        //printk("AT+TCMQTTPUB=\"$thing/up/property/MHS6PDE6WM/test\",1,\"{\\\"method\\\":\\\"report\\\",\\\"params\\\":{\\\"PH\\\":\\\"%.1f\\\",\\\"DD\\\":\\\"%.1f\\\",\\\"HZ\\\":\\\"%.1f\\\",\\\"GR\\\":\\\"%.1f\\\",\\\"HWD\\\":\\\"%.1f\\\",\\\"WWD\\\":\\\"%.1f\\\"}}\"\r\n",ph,tdsvalue,ntu,tds,hwd,wwd);

              
    }
}


/******************************************************************************
 * ��ȷ���������� ?
 ******************************************************************************/

int main(int argc, char** argv)
{
	rt_kprintf("\r\nWelcome to RT-Thread.\r\n\r\n");

    ls1x_drv_init();            		/* Initialize device drivers */

    rt_ls1x_drv_init();         		/* Initialize device drivers for RTT */

  //  install_3th_libraries();      		/* Install 3th libraies */

#ifdef GP7101_DRV
  //  set_lcd_brightness(70);    			/* ���� LCD ���� */
#endif


//#ifdef BSP_USE_FB
//	start_my_gui(); 					/* start simple gui */
//#endif
//#ifdef BSP_USE_CAN0
//	can0_test_start();
//#endif

//#ifdef BSP_USE_CAN1
//	can1_test_start();
//#endif

#if defined(ADS1015_DRV) || defined(MCP4725_DRV)
//	i2c0_slaves_test_start();
#endif

    rt_kprintf("main() is exit!\r\n");

	/*
	 * Finsh as another thread...
	 */
	 
    // �ر�LED1
    gpio_enable(54, DIR_OUT);
    gpio_write(54, 1);
    gpio_enable(2,DIR_OUT);//??
    gpio_write(2,1);
    gpio_enable(3,DIR_OUT);//??
    gpio_write(3,1);
    gpio_disable(51);
    gpio_disable(50);
    gpio_disable(42);
    gpio_disable(43);
// ��ʼ������framebuffer����
    fb_open();
    //�����ַ����ʹ�õ�ǰ��ɫ
    fb_set_fgcolor(cidxBLACK,cidxBLACK);

    //���ڳ�ʼ��
    ls1x_uart_init(devUART0, 115200);
    ls1x_uart_open(devUART0, NULL);
    ls1x_uart_init(devUART1, 115200);
    ls1x_uart_open(devUART1, NULL);
    ls1x_uart_init(devUART3, 115200);
    ls1x_uart_open(devUART3, NULL);
    ls1x_uart_init(devUART4, 115200);
    ls1x_uart_open(devUART4, NULL);
    ls1x_uart_init(devUART5, 115200);
    ls1x_uart_open(devUART5, NULL);
    
    GT1151_Init();
    DS18B20_Init();  //�¶ȴ�����
    
    //���ӹ��ͨ/*
    /*ls1x_uart_write(devUART4,at1,14,NULL);      //����IP
    delay_ms(200);
    ls1x_uart_write(devUART4,at1,14,NULL);      //����IP
    delay_ms(200);
    ls1x_uart_write(devUART4,at1,14,NULL);      //����IP
    delay_ms(200);

    ls1x_uart_write(devUART4,at2,72,NULL);      //����ƽ̨��Ϣ
    delay_ms(200);

    ls1x_uart_write(devUART4,at3,31,NULL);      //�������Ӳ���������
    delay_ms(200);

    ls1x_uart_write(devUART4,at4,55,NULL);      //�����ϱ��������Ա�ǩ
    delay_ms(200);*/
    
    //��ȡ��Ļ�ֱ���
    int xres,yres;
    xres = fb_get_pixelsx();
	yres = fb_get_pixelsy();
	printk("xres = %d,yres = %d\n",xres,yres);

    ls1x_i2c_initialize(busI2C0);
    ls1x_ads1015_ioctl((void *)busI2C0, IOCTL_ADS1015_DISP_CONFIG_REG, NULL);
	 
    // ����ʾ
    fb_open();
    delay_ms(200);

    //��ʾ����
    display_wallpaper();


    rt_kprintf("\r\nWelcome to RT-Thread.\r\n\r\n");

    /* �� �� �� �� 1�� �� �� �� thread1�� �� �� �� thread1_entry*/
    thread1 = rt_thread_create("thread1",		//�߳�����
                                thread1_entry,	//�߳���ں���
                                RT_NULL,		//�߳���ں�������
                                1024,			//�߳�ջ��С
                                11,				//�̵߳����ȼ�
                                10);			//�߳�ʱ��Ƭ
    /* �� �� �� �� �� �� �� �� �飬 �� �� �� �� �� �� */
   // if (thread1 != RT_NULL)
	rt_thread_startup(thread1);

    /* �� ʼ �� �� �� 2�� �� �� �� thread2�� �� �� �� thread2_entry */
    rt_thread_init( &thread2,					//�߳̿��ƿ�
                    "thread2",					//�߳�����
                    thread2_entry,				//�߳���ں���
                    RT_NULL,					//�߳���ں�������
                    &thread2_stack[0],			//�߳�ջ��ʼ��ַ
                    sizeof(thread2_stack),		//�߳�ջ��С
                    12,							//�̵߳����ȼ�
                    10);						//�߳�ʱ��Ƭ

    rt_thread_startup(&thread2);


    return 0;
}
    



/*
 * @@ End
 */
