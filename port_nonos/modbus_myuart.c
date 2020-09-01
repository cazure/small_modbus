#include "main.h"
#include "modbus_myuart.h"

#define isr_enter()
#define isr_leave()

void myuart_isr(MYUART_t * myuart);

#ifdef USE_MYUART1
extern UART_HandleTypeDef huart1;
MYUART_t myuart1;

static uint8_t u1_rbuff[256]; 
static uint8_t u1_tbuff[256]; 

void USART1_IRQHandler(void)
{
	isr_enter();
	myuart_isr(&myuart1);
	isr_leave();
}

void u1_rx_callback(void)
{
	
}
#endif

#ifdef USE_MYUART2
extern UART_HandleTypeDef huart2;
MYUART_t myuart2;

static uint8_t u2_rbuff[256]; 
static uint8_t u2_tbuff[256]; 

void USART2_IRQHandler(void)
{
	isr_enter();
	//HAL_UART_IRQHandler();
	myuart_isr(&myuart2);
	isr_leave();
}

void u2_rx_callback(void)
{
	event_set(&myevent,EVENT_UART2);
}
#endif

#ifdef USE_MYUART3
extern UART_HandleTypeDef huart3;
MYUART_t myuart3;

static uint8_t u3_rbuff[256]; 
static uint8_t u3_tbuff[256]; 

void USART3_IRQHandler(void)
{
	isr_enter();
	myuart_isr(&myuart3);
	isr_leave();
}

void u3_rx_callback(void)
{
	
}
#endif

#ifdef USE_MYUART4
extern UART_HandleTypeDef huart4;
MYUART_t myuart4;

static uint8_t u4_rbuff[256]; 
static uint8_t u4_tbuff[256]; 

void UART4_IRQHandler(void)
{
	isr_enter();
	myuart_isr(&myuart4);
	isr_leave();
}

void u4_rx_callback(void)
{
	
}
#endif

#ifdef USE_MYUART5
extern UART_HandleTypeDef huart5;
MYUART_t myuart5;

static uint8_t u5_rbuff[512]; 
static uint8_t u5_tbuff[512]; 

void UART5_IRQHandler(void)
{
	isr_enter();
	myuart_isr(&myuart5);
	isr_leave();
}

void u5_rx_callback(void)
{
	
}
#endif

int bus_myuart_init(void)
{
	printf("init");
#ifdef USE_MYUART1
	myuart1.huart = &huart1;
	myuart1.huart->Instance = USART1;
  myuart1.huart->Init.WordLength = UART_WORDLENGTH_8B;
  myuart1.huart->Init.StopBits = UART_STOPBITS_1;
  myuart1.huart->Init.Parity = UART_PARITY_NONE;
  myuart1.huart->Init.Mode = UART_MODE_TX_RX;
  myuart1.huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  myuart1.huart->Init.OverSampling = UART_OVERSAMPLING_16;
	myuart1.huart->Init.BaudRate = 115200;
	myuart1.irq_type = USART2_IRQn;
	
	myuart1.rx_event = EVENT_SERIAL1;
	myuart1.rx_callback = u1_rx_callback;
	
	modbus_ringbuffer_init(&(myuart1.rx_rbuff),u1_rbuff,sizeof(u1_rbuff));
	modbus_ringbuffer_init(&(myuart1.tx_rbuff),u1_tbuff,sizeof(u1_tbuff));
	strncpy(myuart1.name,"usart1",6);

	myuart_open(&myuart1);
#endif
#ifdef USE_MYUART2
	myuart2.huart = &huart2;
	myuart2.huart->Instance = USART2;
  myuart2.huart->Init.WordLength = UART_WORDLENGTH_8B;
  myuart2.huart->Init.StopBits = UART_STOPBITS_1;
  myuart2.huart->Init.Parity = UART_PARITY_NONE;
  myuart2.huart->Init.Mode = UART_MODE_TX_RX;
  myuart2.huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  myuart2.huart->Init.OverSampling = UART_OVERSAMPLING_16;
	myuart2.huart->Init.BaudRate = 9600;
	myuart2.irq_type = USART2_IRQn;
	
	myuart2.rx_callback = u2_rx_callback;
	
	modbus_ringbuffer_init(&(myuart2.rx_rbuff),u2_rbuff,sizeof(u2_rbuff));
	modbus_ringbuffer_init(&(myuart2.tx_rbuff),u2_tbuff,sizeof(u2_tbuff));
	
	strncpy(myuart2.name,"usart2",6);

	myuart_open(&myuart2);
#endif

#ifdef USE_MYUART3
	myuart3.huart = &huart3;
	myuart3.huart->Instance = USART3;
  myuart3.huart->Init.WordLength = UART_WORDLENGTH_8B;
  myuart3.huart->Init.StopBits = UART_STOPBITS_1;
  myuart3.huart->Init.Parity = UART_PARITY_NONE;
  myuart3.huart->Init.Mode = UART_MODE_TX_RX;
	myuart3.huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  myuart3.huart->Init.OverSampling = UART_OVERSAMPLING_16;
	myuart3.huart->Init.BaudRate = 9600;
	myuart3.irq_type = USART3_IRQn;
	
	myuart3.rx_event = EVENT_SERIAL3;
	//myuart3.rx_callback = u3_rx_callback;
	
	modbus_ringbuffer_init(&(myuart3.rx_rbuff),u3_rbuff,sizeof(u3_rbuff));
	modbus_ringbuffer_init(&(myuart3.tx_rbuff),u3_tbuff,sizeof(u3_tbuff));
	
	strncpy(myuart3.name,"usart3",6);
	
	myuart_open(&myuart3);
#endif

#ifdef USE_MYUART4	
	myuart4.huart = &huart4;
	myuart4.huart->Instance = UART4;
  myuart4.huart->Init.WordLength = UART_WORDLENGTH_8B;
  myuart4.huart->Init.StopBits = UART_STOPBITS_1;
  myuart4.huart->Init.Parity = UART_PARITY_NONE;
  myuart4.huart->Init.Mode = UART_MODE_TX_RX;
  myuart4.huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  myuart4.huart->Init.OverSampling = UART_OVERSAMPLING_16;
	myuart4.huart->Init.BaudRate = 9600;
	myuart4.irq_type = UART4_IRQn;
	
	myuart4.rx_event = EVENT_SERIAL4;
	myuart4.rx_callback = u4_rx_callback;
	
	modbus_ringbuffer_init(&(myuart4.rx_rbuff),u4_rbuff,sizeof(u4_rbuff));
	modbus_ringbuffer_init(&(myuart4.tx_rbuff),u4_tbuff,sizeof(u4_tbuff));
	
	strncpy(myuart4.name,"uart4",5);
	
	myuart_open(&myuart4);	
#endif

#ifdef USE_MYUART5
	myuart5.huart = &huart5;
	myuart5.huart->Instance = UART5;
  myuart5.huart->Init.WordLength = UART_WORDLENGTH_8B;
  myuart5.huart->Init.StopBits = UART_STOPBITS_1;
  myuart5.huart->Init.Parity = UART_PARITY_NONE;
  myuart5.huart->Init.Mode = UART_MODE_TX_RX;
  myuart5.huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  myuart5.huart->Init.OverSampling = UART_OVERSAMPLING_16;
	myuart5.huart->Init.BaudRate = 9600;
	myuart5.irq_type = UART5_IRQn;
	
	myuart5.rx_event = EVENT_SERIAL5;
	//myuart5.rx_callback = u5_rx_callback;
	
	modbus_ringbuffer_init(&(myuart5.rx_rbuff),u5_rbuff,sizeof(u5_rbuff));
	modbus_ringbuffer_init(&(myuart5.tx_rbuff),u5_tbuff,sizeof(u5_tbuff));
	
	strncpy(myuart5.name,"uart5",5);
	
	myuart_open(&myuart5);
#endif
}

//20200407 isr
void myuart_isr(MYUART_t * myuart)
{
   uint32_t isrflags   = READ_REG(myuart->huart->Instance->ISR);
   uint32_t cr1its     = READ_REG(myuart->huart->Instance->CR1);
   uint32_t cr3its     = READ_REG(myuart->huart->Instance->CR3);
   uint32_t errorflags = 0x00U;
//   uint32_t dmarequest = 0x00U;
		uint8_t temp;

  /* If no error occurs */
  errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
  if(errorflags == RESET)
  {
    /* UART in mode Receiver -------------------------------------------------*/
    if(((isrflags & USART_ISR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
    {
			//__HAL_UART_CLEAR_FLAG(&(myuart->huart), UART_FLAG_RXNE);
			temp = (uint8_t)(myuart->huart->Instance->RDR & (uint8_t)0x00FF);
			modbus_ringbuffer_putchar(&(myuart->rx_rbuff),temp);
      return;
    }
		if(((isrflags & USART_ISR_IDLE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
		{
			__HAL_UART_CLEAR_IDLEFLAG(myuart->huart);
			if(myuart->rx_callback)
				myuart->rx_callback(); 
			return;
		}
  }
	  /* If some errors occur */
  if((errorflags != RESET) && (((cr3its & USART_CR3_EIE) != RESET) || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)))
  {
    /* UART parity error interrupt occurred ----------------------------------*/
    if(((isrflags & USART_ISR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
    {
      myuart->huart->ErrorCode |= HAL_UART_ERROR_PE;
			__HAL_UART_CLEAR_PEFLAG(myuart->huart);
    }

    /* UART noise error interrupt occurred -----------------------------------*/
    if(((isrflags & USART_ISR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
    {
      myuart->huart->ErrorCode |= HAL_UART_ERROR_NE;
			__HAL_UART_CLEAR_NEFLAG(myuart->huart);
    }

    /* UART frame error interrupt occurred -----------------------------------*/
    if(((isrflags & USART_ISR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
    {
      myuart->huart->ErrorCode |= HAL_UART_ERROR_FE;
			__HAL_UART_CLEAR_FEFLAG(myuart->huart);
    }

    /* UART Over-Run interrupt occurred --------------------------------------*/
    if(((isrflags & USART_ISR_ORE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
    { 
      myuart->huart->ErrorCode |= HAL_UART_ERROR_ORE;
			__HAL_UART_CLEAR_OREFLAG(myuart->huart);
    }

    /* Call UART Error Call back function if need be --------------------------*/
    if(myuart->huart->ErrorCode != HAL_UART_ERROR_NONE)
    {
			
    }
    return;
  } /* End if some error occurs */
  
	
	 /* UART in mode Transmitter ------------------------------------------------*/
  if(((isrflags & USART_ISR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
  {
		if(modbus_ringbuffer_getchar(&(myuart->tx_rbuff),&temp)==1)
		{
			myuart->huart->Instance->RDR = (uint8_t)temp;
		}else
		{
			__HAL_UART_DISABLE_IT(myuart->huart, UART_IT_TXE);
			if(myuart->tx_callback)
				myuart->tx_callback();
		}
    return;
  }
	
	 /* UART in mode Transmitter end --------------------------------------------*/
  if(((isrflags & USART_ISR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
  {
		__HAL_UART_DISABLE_IT(myuart->huart, UART_IT_TC);
    return;
  }
}

////20200407 isr
//void myuart_isr(MYUART_t * myuart)
//{
//   uint32_t isrflags   = READ_REG(myuart->huart->Instance->SR);
//   uint32_t cr1its     = READ_REG(myuart->huart->Instance->CR1);
//   uint32_t cr3its     = READ_REG(myuart->huart->Instance->CR3);
//   uint32_t errorflags = 0x00U;
////   uint32_t dmarequest = 0x00U;
//		uint8_t temp;

//  /* If no error occurs */
//  errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
//  if(errorflags == RESET)
//  {
//    /* UART in mode Receiver -------------------------------------------------*/
//    if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
//    {
//			//__HAL_UART_CLEAR_FLAG(&(myuart->huart), UART_FLAG_RXNE);
//			temp = (uint8_t)(myuart->huart->Instance->DR & (uint8_t)0x00FF);
//			modbus_ringbuffer_putchar(&(myuart->rx_rbuff),temp);
//      return;
//    }
//		if(((isrflags & USART_SR_IDLE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
//		{
//			__HAL_UART_CLEAR_IDLEFLAG(myuart->huart);
//			if(myuart->rx_callback)
//				myuart->rx_callback(); 
//			return;
//		}
//  }
//	  /* If some errors occur */
//  if((errorflags != RESET) && (((cr3its & USART_CR3_EIE) != RESET) || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != RESET)))
//  {
//    /* UART parity error interrupt occurred ----------------------------------*/
//    if(((isrflags & USART_SR_PE) != RESET) && ((cr1its & USART_CR1_PEIE) != RESET))
//    {
//      myuart->huart->ErrorCode |= HAL_UART_ERROR_PE;
//			__HAL_UART_CLEAR_PEFLAG(myuart->huart);
//    }

//    /* UART noise error interrupt occurred -----------------------------------*/
//    if(((isrflags & USART_SR_NE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
//    {
//      myuart->huart->ErrorCode |= HAL_UART_ERROR_NE;
//			__HAL_UART_CLEAR_NEFLAG(myuart->huart);
//    }

//    /* UART frame error interrupt occurred -----------------------------------*/
//    if(((isrflags & USART_SR_FE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
//    {
//      myuart->huart->ErrorCode |= HAL_UART_ERROR_FE;
//			__HAL_UART_CLEAR_FEFLAG(myuart->huart);
//    }

//    /* UART Over-Run interrupt occurred --------------------------------------*/
//    if(((isrflags & USART_SR_ORE) != RESET) && ((cr3its & USART_CR3_EIE) != RESET))
//    { 
//      myuart->huart->ErrorCode |= HAL_UART_ERROR_ORE;
//			__HAL_UART_CLEAR_OREFLAG(myuart->huart);
//    }

//    /* Call UART Error Call back function if need be --------------------------*/
//    if(myuart->huart->ErrorCode != HAL_UART_ERROR_NONE)
//    {
//			
//    }
//    return;
//  } /* End if some error occurs */
//  
//	
//	 /* UART in mode Transmitter ------------------------------------------------*/
//  if(((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
//  {
//		if(modbus_ringbuffer_getchar(&(myuart->tx_rbuff),&temp)==1)
//		{
//			myuart->huart->Instance->DR = (uint8_t)temp;
//		}else
//		{
//			__HAL_UART_DISABLE_IT(myuart->huart, UART_IT_TXE);
//			if(myuart->tx_callback)
//				myuart->tx_callback();
//		}
//    return;
//  }
//	
//	 /* UART in mode Transmitter end --------------------------------------------*/
//  if(((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
//  {
//		__HAL_UART_DISABLE_IT(myuart->huart, UART_IT_TC);
//    return;
//  }
//}

int myuart_close(MYUART_t * myuart)
{
	HAL_UART_DeInit(myuart->huart);
	HAL_NVIC_DisableIRQ(myuart->irq_type);
	printf("close %s:%p\n",myuart->name,myuart->huart->Instance);
	return 0;
}

int myuart_open(MYUART_t * myuart)
{
  HAL_UART_Init(myuart->huart);

	HAL_NVIC_SetPriority(myuart->irq_type, 0, 0);
  HAL_NVIC_EnableIRQ(myuart->irq_type);

	__HAL_UART_ENABLE_IT(myuart->huart, UART_IT_PE);
	__HAL_UART_ENABLE_IT(myuart->huart, UART_IT_ERR);
	__HAL_UART_ENABLE_IT(myuart->huart, UART_IT_RXNE);
	__HAL_UART_ENABLE_IT(myuart->huart, UART_IT_IDLE);

	__HAL_UART_CLEAR_IDLEFLAG(myuart->huart);
	
	printf("open %s:%p\n",myuart->name,myuart->huart->Instance);
	return 0;
}

int myuart_write(MYUART_t * myuart,uint8_t *data,uint16_t len,int32_t timeout)
{
	if(timeout!=0)
	{
		__HAL_UART_DISABLE_IT(myuart->huart, UART_IT_TXE);
		HAL_UART_Transmit(myuart->huart, data, len,(uint16_t)timeout);
	}else
	{
		modbus_ringbuffer_put(&(myuart->tx_rbuff),data,len);
		__HAL_UART_ENABLE_IT(myuart->huart, UART_IT_TXE);
	}
	//printf("%s write:%d:%d",myuart->name,len,timeout);
	return len;
}

int myuart_read(MYUART_t * myuart,uint8_t *data,uint16_t len,int32_t timeout)
{
	uint16_t rc=0;
	if(modbus_ringbuffer_data_len(&(myuart->rx_rbuff))>0)
	{
		rc = modbus_ringbuffer_get(&(myuart->rx_rbuff),data,len);
		//printf("%s read ok:%d:%d:%d",myuart->name,rc,len,timeout);
	}else
	{
		//printf("%s read wait:%d:%d",myuart->name,len,timeout);
		rc = 0;
	}
	return rc;
}

int myuart_flush(MYUART_t * myuart)
{
	int rc=0;
	rc = modbus_ringbuffer_data_len(&(myuart->rx_rbuff));
	//printf("%s flush:%d",myuart->name,rc);
	modbus_ringbuffer_reset(&(myuart->rx_rbuff));
	return rc;
}

int myuart_select(MYUART_t * myuart,int32_t timeout)
{
	int rc=0;
	rc = modbus_ringbuffer_data_len(&(myuart->rx_rbuff));
	//printf("%s length:%d:%d",myuart->name,rc,timeout);
	if(rc<=0)
	{
		HAL_Delay(3);
		return -1;
	}
	return rc;
}
