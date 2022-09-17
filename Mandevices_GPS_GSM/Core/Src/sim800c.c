#include "sim800c.h"

char simRxBuf[100] = {0};
int simRxIndex = 0;
char simRxData = '\0';
char simRxResponse[100] = {0};
simRxCpltStruct_t simRxCplt = SIM_NOT_RX_CPLT;

void sim800c_debug(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_UART_Transmit(&huart1, pData, Size , Timeout);
}

void sim800c_init(void)
{
    if(sim800c_sendCommand("ATE0\r\n", "OK\r\n", SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand("AT\r\n", "OK\r\n", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    // sim800c_GSM_GPRS();    
}

simState_t sim800c_sendCommand(char *simCommand, char *trueResponse, int checkOrNot)
{   
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&simRxData, 1);
    sim800c_debug((uint8_t*)simCommand, strlen(simCommand), 100);
    HAL_UART_Transmit_IT(&huart2, (uint8_t*)simCommand, strlen(simCommand));
    HAL_TIM_Base_Start_IT(&htim2);
    while(!(simRxCplt == SIM_RX_CPLT));
    sim800c_debug((uint8_t*)simRxResponse, strlen(simRxResponse), 100);
    //check respone from sim 
    if((checkOrNot == SIM_CHECK) && (strstr(simRxResponse, trueResponse) == NULL))
    {
        memset(simRxResponse, '\0', strlen(simRxResponse));
        simRxCplt = SIM_NOT_RX_CPLT;
        return SIM_NOT_OK;
    }
    memset(simRxResponse, '\0', strlen(simRxResponse));
    simRxCplt = SIM_NOT_RX_CPLT;
    return SIM_OK;
}
//check response from Sim and delete Buffer
void sim800c_callback(void)
{
    htim2.Instance->CNT = 0;
    simRxBuf[simRxIndex++] = simRxData;
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&simRxData, 1);
    if((strstr(simRxBuf, "\r\n") != NULL) && (simRxIndex == 2))
    {
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
}
//wait
void sim800c_timerCallback(void)
{
    HAL_TIM_Base_Stop_IT(&htim2);
    htim2.Instance->CNT = 0;
    if((strstr(simRxBuf, "\r\n") != NULL) | (strstr(simRxBuf, ">") != NULL))
    {
        memcpy(simRxResponse, simRxBuf, strlen(simRxBuf));
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
    simRxCplt = SIM_RX_CPLT;
}

void sim800c_sendSMS(char *latData, char *longData, char *simNumber, char *simMessage)
{
    char smsATCommand[50] = {0};
    char endSMS[2] = {0};
    endSMS[0] = 0x1A;
    endSMS[1] = '\0';
    sprintf(smsATCommand, "AT+CMGS=\"%s\"\r\n", simNumber);
    if(sim800c_sendCommand("AT+CMGF=1\r\n", "OK\r\n", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand(smsATCommand, ">", SIM_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand(simMessage, NULL, SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
    if(sim800c_sendCommand(endSMS, NULL, SIM_NOT_CHECK) != SIM_OK)
    {
        sim800c_errorHandle();
    }
}

void sim800c_sleep(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CSCLK=2\r\n", strlen("AT+CSCLK=2\r\n"),100);
}

void sim800c_wakeup(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT\r\n", strlen("AT\r\n"),100);
    HAL_Delay(100);
    HAL_UART_Transmit(&huart2, (uint8_t*)"AT+CSCLK=0\r\n", strlen("AT+CSCLK=0\r\n"),100);
}

void sim800c_errorHandle(void)
{
    sim800c_debug((uint8_t*)"Sim error\r\n", strlen("Sim error\r\n"), 100);
    __disable_irq();
    while(1)
    {

    }
}