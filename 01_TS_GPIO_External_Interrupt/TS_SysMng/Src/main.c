/***********************************************************************************
 * File                     :main.c
 *
 * Title                    :
 *
 * Author                   :Tarik SEMRADE
 *
 * Description              :External interrupt toggle led
 *                           External Interrupt Service Routine tutorial
 *
 * Version                  : 0.1
 *
 * Copyright (c) 2020 Tarik SEMRADE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *********************************************************************************/

/**********************************************************************************
 *  Included Files
 *
 *********************************************************************************/

#include "F28x_Project.h"
#include "gpio.h"
#include "xbar.h"
#include "device.h"
#include "main.h"

/**********************************************************************************
 *  Defines
 *
 *********************************************************************************/
#define BLEU_LED        DEVICE_GPIO_PIN_LED1
#define RED_LED         DEVICE_GPIO_PIN_LED2
#define GPIO_INT_PIN    2U

/**********************************************************************************
 * \function:       main
 * \brief           main `0` numbers
 * \param[in]       void
 * \return          void
 **********************************************************************************/
void main(void)
{

    /* Set up system flash and turn peripheral clocks */
    InitSysCtrl();

    /* GPIO Init */
    InitGpio();

    /* Globally disable maskable CPU interrupts */
    DINT;

    /* Clear and disable all PIE interrupts */
    InitPieCtrl();

    /* Individually disable maskable CPU interrupts */
    IER = 0x0000;

    /* Clear all CPU interrupt flags */
    IFR = 0x0000;

    /* Populate the PIE interrupt vector table with */
    InitPieVectTable();

    /***********************Interrupt linking functions*****************************/
    EALLOW;
    /* Connect ISR GPIO function to XINT5 Interruption CORE */
    PieVectTable.XINT5_INT = &Xint5_ISR;
    EDIS;

    /************************Peripheral Initialization*****************************/

    /* Init Led function */
    GpioLedInit();

    /* Init InputXbar peripheral */
    XbarPinConfig();

    /* Init External Interrupt */
    Xint5PinConfig();

    /* Peripheral Interrupt Expansion (PIE) */
    /* Enable XINT5 in the PIE: Group 12 __interrupt 3 */
    PieCtrlRegs.PIEIER12.bit.INTx3 = 1;

    /* Core Interrupt Logic */
    /* Enable group 12 interrupts */
    IER |= M_INT12;

    /* Enable Global interrupt INTM */
    EINT;

    /* Enable Global realtime interrupt DBGM */
    ERTM;

    /* Infinite led loop */
    while (1)
    {
        asm(" NOP");
    }

}
/**********************************************************************************
 * \function:       GpioLedInit
 * \brief           `0` Param
 * \param[in]       void
 * \return          void
 **********************************************************************************/
void GpioLedInit(void)
{

    /* GPIO bleuLed configuration using TI function */
    GPIO_SetupPinMux(BLEU_LED, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLEU_LED, GPIO_OUTPUT, GPIO_ASYNC);

    /* GPIO RedLed configuration using TI function */
    GPIO_SetupPinMux(RED_LED, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RED_LED, GPIO_OUTPUT, GPIO_ASYNC);

    /* GPIO Configuration for interrupt routine */
    GPIO_SetupPinMux(GPIO_INT_PIN, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(GPIO_INT_PIN, GPIO_INPUT,
    GPIO_OPENDRAIN | GPIO_PULLUP | GPIO_QUAL6 | GPIO_SYNC);

}
/**********************************************************************************
 * \function:       XbarPinConfig
 * \brief           `0` Param
 * \param[in]       void
 * \return          void
 **********************************************************************************/
void XbarPinConfig(void)
{
    /* Activate Write protection EALLOW */
    EALLOW;

    /* GPIO 2 pin routing to interrupt PIE */
    InputXbarRegs.INPUT14SELECT = GPIO_INT_PIN;

    /* Disable write protection EALLOW */
    EDIS;
}
/**********************************************************************************
 * \function:       Xint5PinConfig
 * \brief           `0` Param
 * \param[in]       void
 * \return          void
 **********************************************************************************/
void Xint5PinConfig(void)
{
    /* Enable EALLOW protected register access */
    EALLOW;

    /* Configure XINT1 to Falling edge interrupt */
    XintRegs.XINT5CR.bit.POLARITY = 0;

    /* Enable XINT5 */
    XintRegs.XINT5CR.bit.ENABLE = 1;

    /* Disable EALLOW protected register access */
    EDIS;

}
/**********************************************************************************
 * \function:       Xint5_ISR
 * \brief           `0` Param
 * \param[in]       void
 * \return          void
 **********************************************************************************/
interrupt void Xint5_ISR(void)
{
    /* Clear Flag */
    PieCtrlRegs.PIEIFR12.bit.INTx5 = 1;

    /* Toggle blue led and wait 500ms */
    GPIO_togglePin(BLEU_LED);
    DELAY_US(500*1000);

    /* Toggle red led and wait 500ms */
    GPIO_togglePin(RED_LED);
    DELAY_US(500*1000);

    /* Must acknowledge the PIE group 12 to get more interrupts */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
