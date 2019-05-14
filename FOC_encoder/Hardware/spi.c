#include "spi.h"
#include "delay.h"
/*
  *              ͨ�ţ�SPI PA4:SPI1_NSS
  *                        PA5:SPI1_SCK
  *                        PA6:SPI1_MISO
  *                        PA7:SPI1_MOSI
  */
void gpio_mode_af(GPIO_TypeDef* GPIOx,uint16_t pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&GPIO_InitStruct);
}
void gpio_mode_out(GPIO_TypeDef* GPIOx,uint16_t pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&GPIO_InitStruct);
}
//�½�����master����slave���յ�ʱ�̣���������salve����master���յ�ʱ��
//��16��ʱ�����ں��������cs����ʹĳЩ���ָ�λ
void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	gpio_mode_af(GPIOA,GPIO_Pin_7);//MOSI
	gpio_mode_af(GPIOA,GPIO_Pin_6);//MISO
	gpio_mode_af(GPIOA,GPIO_Pin_5);//SCK
	gpio_mode_out(GPIOA,GPIO_Pin_4);//cs
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;//36/4=9M,���ͨ������10M��������
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;//ż�����زɼ�����
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;//����ʱ��Ϊ�͵�ƽ
	SPI_InitStruct.SPI_CRCPolynomial = 15;//��ȡ16λ������һλ��CRCУ��λ
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_16b;//16λ����
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//����˫��ȫ������
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//оƬ���ø�λ����
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//��Ƭ����������ģʽ
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;//����������������������Ƭѡ�ź�
	
	SPI_Init(SPI1,&SPI_InitStruct);
	SPI_Cmd(SPI1, ENABLE);
	//�������ų�ʼ��оƬ
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	
}
//����ʱ��ͼ��ÿ�ζ�ȡǰ��Ҫ����Ƭѡ�ź�������ʹоƬ��ʼ��
//��ȡ�Ķ���16λ���������Ըĳ�uint16_t
uint16_t spi1_read_write_byte(uint16_t txc)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	while((SPI1->SR&SPI_SR_TXE)==0);
	SPI1->DR = txc;
	while((SPI1->SR&SPI_SR_RXNE)==0);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	return SPI1->DR;	
}
// as5048��������־λ
uint16_t ClearAndNop(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    spi1_read_write_byte(0x4001);              // ����żУ��Ĵ����־λ�������
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
    delay_us(10);              // ��������֮����350ns�ļ����Դ�Թٷ�datasheet
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    spi1_read_write_byte(0xc000);               // ����żУ��Ĵ����־λ�������
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
struct as5048_data CollectData(void)
{
    uint16_t anglereg = 0, magreg = 0, agcreg = 0;
    uint16_t mag = 0, value = 0;
    double angle = 0.0;
    uint8_t agc = 0;
    struct as5048_data Temp = {1,0,0,0,0.0};

    spi1_read_write_byte(CMD_ANGLE);
	  spi1_read_write_byte(CMD_ANGLE);
    anglereg = spi1_read_write_byte(CMD_MAG); 
	  value = anglereg & 0x3fff;
    magreg = spi1_read_write_byte(CMD_AGC);  
	  mag = magreg & 0x3fff;
    agcreg = spi1_read_write_byte(CMD_NOP);   
	  agc = (uint8_t)agcreg & 0x00ff;
    angle = (value * 360.0)/16384.0;
    if ((anglereg & 0x4000) | (magreg & 0x4000) | (agcreg & 0x4000))
    {
        ClearAndNop();
        //rt_kprintf("There is and error!\n");
        Temp.iserror = 1;
    }
    else
    {
        Temp.iserror = 0;
        Temp.angle = angle;
        Temp.mag = mag;
        Temp.agc = agc;
        Temp.value = value;
    }
    return Temp;
}





