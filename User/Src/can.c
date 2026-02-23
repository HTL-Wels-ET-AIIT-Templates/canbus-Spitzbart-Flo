/**
 ******************************************************************************
 * @file           : can.c
 * @brief          : CAN handling functions
 ******************************************************************************
 * Functions for initializing CAN peripheral, sending and receiving CAN
 * messages.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "stm32f429i_discovery_lcd.h"
#include "tempsensor.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

// ToDo: korrekte Prescaler-Einstellung check
#define   CAN1_CLOCK_PRESCALER    12

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef     canHandle;
uint8_t sendState  = 0;
static unsigned int recvCnt = 0;
static unsigned int sendCnt = 0;
uint8_t byte0;
uint8_t byte1;
uint16_t bigbyte;

/* Private function prototypes -----------------------------------------------*/
static void initGpio(void);
static void initCanPeripheral(void);


/**
 * Initialize hardware GPIO and CAN peripheral
 */
void canInitHardware(void) {
	initGpio();
	initCanPeripheral();



}

/**
 * canInit function, set up hardware and display
 * @param none
 * @return none
 */
void canInit(void) {
	canInitHardware();

	LCD_SetFont(&Font12);
	LCD_SetColors(LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	LCD_SetPrintPosition(3,1);
	printf("CAN1: Send-Recv");

	LCD_SetColors(LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	LCD_SetPrintPosition(5,1);
	printf("Send-Cnt:");
	LCD_SetPrintPosition(5,15);
	printf("%5d", 0);
	LCD_SetPrintPosition(7,1);
	printf("Recv-Cnt:");
	LCD_SetPrintPosition(7,15);
	printf("%5d", 0);
	LCD_SetPrintPosition(9,1);
	printf("Send-Data:");
	LCD_SetPrintPosition(15,1);
	printf("Recv-Data:");

	LCD_SetPrintPosition(30,1);
	printf("Bit-Timing-Register: 0x%lx", CAN1->BTR);

	// ToDo (2): set up DS18B20 (temperature sensor)

}

/**
 * sends a CAN frame, if mailbox is free
 * @param none
 * @return none
 */
void canSendTask(void) {
	// ToDo declare the required variables

	sendCnt ++;
	LCD_SetPrintPosition(5,15);  //Je nach
	printf("%5d", sendCnt);  //Anwendung
	CAN_TxHeaderTypeDef txHeader;
	uint8_t txData[8];
	txHeader.StdId = 0x1AE;
	txHeader.ExtId = 0x00;
	txHeader.RTR = CAN_RTR_DATA;
	txHeader.IDE = CAN_ID_STD;
	txHeader.DLC = 2;
	txData[0] = 0xC3;
	txData[1] = 10;
	uint32_t txMailbox;



	// ToDo (2): get temperature value



	// ToDo prepare send data

	// Prüfen ob alle 3 Mailboxen frei sind
	if (HAL_CAN_GetTxMailboxesFreeLevel(&canHandle) == 0)
	{
		// keine freie Mailbox → senden später erneut
	}
	else
	{
		if (HAL_CAN_AddTxMessage(&canHandle, &txHeader, txData, &txMailbox) != HAL_OK)
		{
			// Fehler beim Senden
		}

		// Optional: warten bis gesendet
		while (HAL_CAN_IsTxMessagePending(&canHandle, txMailbox));
	}


	// ToDo send CAN frame


	// ToDo display send counter and send data



}

/**
 * checks if a can frame has been received and shows content on display
 * @param none
 * @return none
 */
void canReceiveTask(void) {

	CAN_RxHeaderTypeDef rxHeader;
	uint8_t rxData[8];

	LCD_SetPrintPosition(7,15); //Je nach
	printf("%5d", recvCnt);  //Anwendung
	// Prüfen ob Nachricht im FIFO0 liegt
	if (HAL_CAN_GetRxFifoFillLevel(&canHandle, CAN_RX_FIFO0) > 0)
	{
		if (HAL_CAN_GetRxMessage(&canHandle, CAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK)
		{
			// Empfangsfehler
		}
		else
		{
			// ==== Auswertung ====
			if (rxHeader.IDE == CAN_ID_STD)
			{
				uint16_t id = rxHeader.StdId;

				byte0 = rxData[0];
				byte1 = rxData[1];
				bigbyte = (byte0 << 8)| byte1;
				LCD_SetPrintPosition(15, 15);
				printf("STD %03x: %02x %02x", id, byte0, byte1);
				recvCnt ++;
			}
			else if (rxHeader.IDE == CAN_ID_EXT)
			{
				uint32_t id = rxHeader.ExtId;

				byte0 = rxData[0];
				byte1 = rxData[1];

				LCD_SetPrintPosition(15, 15);
				printf("EXT %08lx: %02x %02x", id, byte0, byte1);
				recvCnt ++;
			}

		}
	}



	// ToDo: check if CAN frame has been received




	// ToDo: Get CAN frame from RX fifo



	// ToDo: Process received CAN Frame (extract data)



	// ToDo display recv counter and recv data



}

/**
 * Initialize GPIOs for CAN
 */
static void initGpio(void)
{
	// TX an PB9 -> Probleme beim Senden, LCD rauscht
	// RX an PB8 -> Probleme beim Senden, LCD rauscht

	GPIO_InitTypeDef  canPins;

	__HAL_RCC_GPIOB_CLK_ENABLE();

	canPins.Alternate = GPIO_AF9_CAN1;
	canPins.Mode = GPIO_MODE_AF_OD;
	canPins.Pin = GPIO_PIN_8 | GPIO_PIN_9;
	canPins.Pull = GPIO_PULLUP;
	canPins.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &canPins);
}

/**
 * Initialize CAN peripheral.
 * Note: CAN1_CLOCK_PRESCALER has to be set!
 * No Filters are applied.
 * IRQs are enabled
 */
static void initCanPeripheral(void) {

	CAN_FilterTypeDef canFilter;

	// CAN Clock enable
	__HAL_RCC_CAN1_CLK_ENABLE();

	// init CAN
	canHandle.Instance = CAN1;
	canHandle.Init.TimeTriggeredMode = DISABLE;
	canHandle.Init.AutoBusOff = DISABLE;
	canHandle.Init.AutoWakeUp = DISABLE;
	canHandle.Init.AutoRetransmission = ENABLE;
	canHandle.Init.ReceiveFifoLocked = DISABLE;
	canHandle.Init.TransmitFifoPriority = DISABLE;
	canHandle.Init.Mode = CAN_MODE_LOOPBACK;
	canHandle.Init.SyncJumpWidth = CAN_SJW_1TQ;

	// CAN Baudrate
	canHandle.Init.TimeSeg1 = CAN_BS1_11TQ;
	canHandle.Init.TimeSeg2 = CAN_BS2_4TQ;
	canHandle.Init.Prescaler = CAN1_CLOCK_PRESCALER;

	if (HAL_CAN_Init(&canHandle) != HAL_OK)
	{
		/* Initialization Error */
		Error_Handler();
	}

	// CAN Filter (keine CAN-Frames filtern)
	// CAN_FilterMode_IdList:
	// Vorgabe der ID in CAN_FilterIdHigh/ CAN_FilterIdLow
	// z.B. CAN_FilterIdHigh = 0x0100 << 5
	//
	// CAN_FilterMode_IdMask:
	// Vorgabe der Maske in CAN_FilterMaskIdHigh/ CAN_FilterMaskIdLow
	// und zusaetzlich die in CAN_FilterIdHigh vorgegebene ID
	// Filtergleichung:
	// (ID & CAN_FilterMaskIdHigh/Low) == CAN_FilterIdListHigh/Low
	// -> CAN-Pakete duerfen passieren
	//
	// Bedeutung der Maske:
	// 0x000 ... alle Bit durchlassen
	// 0x7ff ... alle Bit sperren
	// 0x6ff ... alle Bit durchlassen, fuer die das 9.Bit gesetzt ist
	canFilter.FilterBank = 0;
	canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
	canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
	canFilter.FilterIdHigh = 0x0000;
	canFilter.FilterIdLow = 0x0000;
	canFilter.FilterMaskIdHigh = 0x0000;
	canFilter.FilterMaskIdLow = 0x0000;
	canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
	canFilter.FilterActivation = ENABLE;
	canFilter.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&canHandle, &canFilter) != HAL_OK)
	{
		/* Filter configuration Error */
		Error_Handler();
	}
	/*##-3- Start the CAN peripheral ###########################################*/
	if (HAL_CAN_Start(&canHandle) != HAL_OK)
	{
		/* Start Error */
		Error_Handler();
	}

	/*##-4- Activate CAN RX notification #######################################*/
	//	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	//	if (HAL_CAN_ActivateNotification(&canHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	//	{
	//		/* Notification Error */
	//		Error_Handler();
	//	}

}


/**
 * CAN1-RX ISR
 */
void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&canHandle);
}

/**
 * @brief  Rx Fifo 0 message pending callback
 * @param  hcan: pointer to a CAN_HandleTypeDef structure that contains
 *         the configuration information for the specified CAN.
 * @retval None
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	// Receive is done in main loop

}


