#include "pwm.h"
/******************** hardware introduction ***********************************
* 
* TIMER --- TIM1
* PWM1 --- TIM1->CH1/N --- PA8/PB13 --- A 
* PWM2 --- TIM1->CH2/N --- PA9/PB14 --- B
* PWM3 --- TIM1->CH3/N --- PA10/PB15 --- C
*
* enable PB0-->C
* enable PB1-->B
* enable PB4-->A
* 
* ADC1&ADC2
* Curent A --- channel 3 --- PA3
* Curent B --- channel 6 --- PA6
* Curent C --- channel 14 --- PC4
*******************************************************/
void PWM_Motor_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM1_TimeBaseStructure;
	TIM_OCInitTypeDef TIM1_OCInitStructure;
	TIM_BDTRInitTypeDef TIM1_BDTRInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	{   /* GPIO, TIM1 clocks enabling -----------------------------*/	
		/* Enable GPIOA, GPIOC, GPIOE, AFIO clocks */
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
		/* Enable TIM1 clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO, ENABLE);
    }

	{/* ADC1, ADC2, PWM pins configurations -------------------------------------*/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 ;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_0);
		GPIO_SetBits(GPIOB,GPIO_Pin_1);
		GPIO_SetBits(GPIOB,GPIO_Pin_4);
		GPIO_PinLockConfig(GPIOA, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);
    }   
	{   /* TIM1 Peripheral Configuration -------------------------------------------*/
		/* TIM1 Registers reset */
		TIM_DeInit(TIM1);
		TIM_TimeBaseStructInit(&TIM1_TimeBaseStructure);
		/* Time Base configuration */
		TIM1_TimeBaseStructure.TIM_Prescaler = 0x0;//�����з�Ƶ��߿��ƾ���
		TIM1_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;//���Ķ���ģʽ���պö�ӦSVPWM����
		TIM1_TimeBaseStructure.TIM_Period = PWM_PERIOD;//��ʱ����Ƶ�� 1800//72 000 000---40kHZ
		TIM1_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV2;//�����������������˲�����ʱ��Ƶ��
		/*           
		 *       /\
		 *      /  \
		 *     /    \
		 *    /      \ �����Ķ���ģʽ��,cnt ++��ARR�����������¼���Ȼ��cnt--��0���������¼��������ظ��Ĵ�������Ϊ1
		 *             ����cnt->arr->0һ��ѭ�����ڲŲ���һ�θ����¼�                                                 
		 */
		TIM1_TimeBaseStructure.TIM_RepetitionCounter = 1;//������������REP_RATE+1������¼����������һ�θ����¼�
		TIM_TimeBaseInit(TIM1, &TIM1_TimeBaseStructure);
		/* Channel 1, 2,3 in PWM mode */
		/*           
		 * PWM2     -1-1-   PWM1 -1-1-1-       -1-1-1-
		 *         |     |              |     |
		 *         |     |              |     |
		 *  -0-0-0-       -0-0-0-        -0-0-
		 * PWM1��2����������cntС��PulseʱPWM1Ϊ�߶�pwm2Ϊ�ͣ����PWM1ģʽ�¸ߵ�ƽʱ���������¼�����PWM2�µ͵�ƽʱ���������¼�
         * �����¼�ʱ�����жϿ��Կ���adc���������ݲ���Ҫ��ѡ��ߵ�ƽʱ��PWM1ģʽ		 
		 */
		TIM1_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
		TIM1_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//ʹ���������PWM�� 
		TIM1_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;                  
		TIM1_OCInitStructure.TIM_Pulse = 0x505; //dummy value
		/*           
		 * PWM2&��  ---   PWM1&�� ----     ----  PWM2&�� ----     ---- PWM1&��   ---
		 *         |   |              |   |                  |   |              |   |
		 *         |   |              |   |                  |   |              |   |
		 *     ----     ----           ---                    ---           ----     ----        
		 *  ��pwm1��pwm2ģʽ��ͬ�������յĲ��Σ������������Ϊ����1==�ߵ�ƽ����֮1==�͵�ƽ
		 */
		TIM1_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
		TIM1_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;         
		TIM1_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
		TIM1_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;          

		TIM_OC1Init(TIM1, &TIM1_OCInitStructure); 
		TIM_OC2Init(TIM1, &TIM1_OCInitStructure);
		TIM_OC3Init(TIM1, &TIM1_OCInitStructure); 

//		TIM_OCStructInit(&TIM1_OCInitStructure);
		/* Channel 4 Configuration in OC */
		/*           
		 * PWM2&��  ---   
		 *         |   |            
		 *         |   |            
		 *     ----     ----          
		 *  ���ĸ�ͨ�����������
		 */
//		TIM1_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;  
//		TIM1_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
//		TIM1_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;                  
//		TIM1_OCInitStructure.TIM_Pulse = PWM_PERIOD - 1; 

//		TIM1_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
//		TIM1_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;         
//		TIM1_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
//		TIM1_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;            

//		TIM_OC1Init(TIM1, &TIM1_OCInitStructure);
//		TIM_OC2Init(TIM1, &TIM1_OCInitStructure);
//		TIM_OC3Init(TIM1, &TIM1_OCInitStructure);
        //ʹ���Զ���װ��CCx�Ĵ�����ֵ��������ʹ�ù�������ʱ���ļĴ������ı䲨��
		/* Enables the TIM1 Preload on CC1 Register */
//		TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
//		/* Enables the TIM1 Preload on CC2 Register */
//		TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
//		/* Enables the TIM1 Preload on CC3 Register */
//		TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
		/* Enables the TIM1 Preload on CC4 Register */
//		TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
		
 		/*           
		 * CHx ----     ----  
		 *         |   |           
		 *         |   |             
		 *          ---        
		 * CHNx
		 *          ---
		 *         |   |           
		 *         |   |             
		 *     ----     ----     
		 */        
		/* Automatic Output enable, Break, dead time and lock configuration*/
		TIM1_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
		TIM1_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
		TIM1_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF; 
		TIM1_BDTRInitStructure.TIM_DeadTime = DEADTIME;
		TIM1_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
		TIM1_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
		TIM1_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
		TIM_BDTRConfig(TIM1, &TIM1_BDTRInitStructure);
		
		TIM_ARRPreloadConfig(TIM1, ENABLE);
		TIM_CtrlPWMOutputs(TIM1,ENABLE);
		/* TIM1 counter enable */
		TIM_Cmd(TIM1, ENABLE);
		TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);//ʹ������ж�
        //���ô����ж�Ϊ����¼�
//		TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
	}
	
	{
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		/* Enable the Update Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

	}
}
/*
 * @brief PWM��� ��Χ0-7200
 */
void set_PWM_x(int chx , uint16_t pwm)
{
	switch (chx)
	{
		case 1:
			TIM_SetCompare3(TIM1,pwm);
			break;
		case 2:
			TIM_SetCompare2(TIM1,pwm);
			break;
		case 3:
			TIM_SetCompare1(TIM1,pwm);
			break;
	}
}
/**
  * @brief  TIM�жϷ�����
  * @param  
  * @retval 
  */
extern Volt_Components Stat_Volt_Input;
void TIM1_UP_IRQHandler(void)
{
  // Clear Update Flag
	SVPWM_3ShuntCalcDutyCycles(Stat_Volt_Input);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	TIM_ClearFlag(TIM1, TIM_FLAG_Update); 
}

