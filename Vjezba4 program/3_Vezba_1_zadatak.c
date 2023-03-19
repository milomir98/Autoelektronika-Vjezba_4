/* Driver includes */
#include "HWS_Driver/HWS_conf.h"
static const char character[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x06D, 0x7D, 0x07, 0x7F, 0x6F };
#define MAX_SEM_COUNT 10

/* Priorities at which the tasks are created. */
#define task_prioritet		( tskIDLE_PRIORITY + 2 )

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "I_Vezba.h"
static mxDisp7seg_Handle myDisp;
static SemaphoreHandle_t binSem1;
static TimerHandle_t myTimer1, MyTimer2;
static TaskHandle_t tA, tB, tC, tD;
/* A software timer that is started from the tick hook. */

static BaseType_t myTask;

//local function declaration

static void Display_Toggle_0();
static void first_task(void* pvParams);
static void TimerCallback(TimerHandle_t tmH);

/*static struct taskInfo {
	SemaphoreHandle_t* clk_in;
	SemaphoreHandle_t* clk_out;
	unsigned char* disp;
};*/
static unsigned char dispMem[4];
static SemaphoreHandle_t s1, s2, s3, s4;


static void TimerCallback(TimerHandle_t tmH)
{
	static unsigned char count = 0;
	static unsigned char secount = 0;
	
	mxDisp7seg_SelectDigit(myDisp, (3 - count));
	mxDisp7seg_SetDigit(myDisp, character[dispMem[count]]);
	
	if (count < 3) count++; else count = 0; // count ima vrednost od 0 do 3

	secount++;
	if (secount == 25) { // posle 500 ms
		secount = 0;
		xSemaphoreGive(s1);
	                   }
}



static void only_tsk(void* pvParams)
{
	int s1_value = 0;
	unsigned char number = 0;
	uint32_t task_ID;
	while (1) 
	{
		number++;
		task_ID = (uint32_t)pvParams;
		if (task_ID == 0)
		{
			xSemaphoreTake(s1, portMAX_DELAY);
		//	vTaskDelay(pdMS_TO_TICKS(500UL));
			if (number == 10) { // ako je broj dostigla vrednost 10, treba da se inkrementira vrednost vise cifre na displeju
				number = 0;
				xSemaphoreGive(s2);// dajemo takstni signal visoj cifri preko semafora 
			}
		}
		if (task_ID == 1)
		{
			xSemaphoreTake(s2, portMAX_DELAY);
			if (number == 10) {
				number = 0;
				xSemaphoreGive(s3);
			}
		}
		if (task_ID == 2)
		{
			xSemaphoreTake(s3, portMAX_DELAY);
			if (number == 10) {			
				number = 0;
				xSemaphoreGive(s4);
			}
		}
		if (task_ID == 3)
		{
			xSemaphoreTake(s4, portMAX_DELAY);
			if (number == 10) {
				number = 0;
				//resenja zadatka pod b
				//vTaskDelete(tA); //brisemo task sa ID 0 preko njegovre reference tA 
				//vTaskDelete(tB); //brisemo task sa ID 1 preko njegovre reference tB 
				//vTaskDelete(tC); //brisemo task sa ID 2 preko njegovre reference tC 
				//vTaskDelete(NULL); //brisemo task sa ID 3 preko NULL jer je to aktivan task
			}
		}
		dispMem[task_ID] = number; // upisujemo vrednost broja u niz cija vrednost se prikazuje na displeju
		mxDisp7seg_SelectDigit(myDisp, (4));
		if (s1_value < uxSemaphoreGetCount(s1)) s1_value = uxSemaphoreGetCount(s1);
			mxDisp7seg_SetDigit(myDisp, character[s1_value]);
			
	}
}

void III_vezba_1(void)
{
	/* Inicijalizacija drajvera za displej*/
	mxDisp7seg_Init();
	myDisp = mxDisp7seg_Open(MX7_DISPLAY_0);
	// initialize display to 0000
	dispMem[0] = 0;
	dispMem[1] = 0;
	dispMem[2] = 0;
	dispMem[3] = 0;
	
	TimerHandle_t tH;
	 //   mxDisp7seg_SelectDigit(myDisp, (4));
	//	mxDisp7seg_SetDigit(myDisp, character[uxSemaphoreGetCount(s1)]);

	

	// -------------- CREATE TASKS AND OTHER OBJECTS HERE --------

	tH = xTimerCreate(NULL, pdMS_TO_TICKS(20UL), pdTRUE, NULL, TimerCallback);
	if (tH == NULL)
		while (1);
	xTimerStart(tH, 0);

	

	

	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, (void*)0, 2, &tA) != pdPASS)
		while (1);// ID taska 0, referenca tA
	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, (void*)1, 2, &tB) != pdPASS)
		while (1);// ID taska 1, referenca tB
	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, (void*)2, 2, &tC) != pdPASS)
		while (1);// ID taska 2, referenca tC
	if (xTaskCreate(only_tsk, NULL, configMINIMAL_STACK_SIZE, (void*)3, 2, &tD) != pdPASS)
		while (1);// ID taska 3, referenca tD
	
	s1 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s1 == NULL) while (1);

	s2 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s2 == NULL) while (1);

	s3 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s3 == NULL) while (1);

	s4 = xSemaphoreCreateCounting(MAX_SEM_COUNT, 0);
	if (s4 == NULL) while (1);

	// -----------------------------------------------------------

	vTaskStartScheduler();
	while (1);
}