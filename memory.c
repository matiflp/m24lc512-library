/*
 * m24lc512.c
 *
 *  Created on: 30 oct. 2019
 *      Author: Matías López - Jesús López
 */
//*****************************************************************************
//
// m24lc512.c - Driver para realizar una comunicacion I2C.
//
//*****************************************************************************

#include "memory.h"
//*****************************************************************************
uint16_t countS;
uint8_t *myArray;
//**********************************************************************************************************************************************************
void M24LC512_initPort(void)
{
    // Configuración de los pines I2C (Port 5 Pin 2 -> SDA y Port 5 Pin 3 -> SCL)
    M24LC512_PORT_SEL |= M24LC512_PIN_SDA | M24LC512_PIN_SCL;      // I2C pins

    // Configure USCI_B0 for I2 mode - Sending
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 |= UCMST | UCMODE_3 | UCSYNC | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK

    UCB0BRW = M24LC512_PIN_SCL_CLOCK_DIV;                          // SMCLK / 10 = 100 KHz; Bit clock prescaler. Modify only when UCSWRST = 1.

    UCB0I2CSA = M24LC512_I2C_ADDRESS;                              // Dirección de la memoria (Esclavo).

    UCB0CTLW0 &= ~UCSWRST;                                         // clear reset register

    if (UCB0STAT & UCBBUSY)                                        // test if bus to be free otherwise a manual Clock on is
    {                                                              // generated
        M24LC512_PORT_SEL &= ~M24LC512_PIN_SCL;                    // Select Port function for SCL
        M24LC512_PORT_OUT &= ~M24LC512_PIN_SCL;
        M24LC512_PORT_DIR |=  M24LC512_PIN_SCL;                    // drive SCL low
        M24LC512_PORT_SEL |=  M24LC512_PIN_SDA + M24LC512_PIN_SCL; // select module function for the used I2C pins
    };
}
//**********************************************************************************************************************************************************
static void M24LC512_initWrite(void)
{
  UCB0CTLW0 |= UCTR;                        // UCTR=1 => Transmit Mode (R/W bit = 0)
  UCB0IFG &= ~UCTXIFG0;
  UCB0IE &= ~UCRXIE0;                        // disable Receive ready interrupt
  UCB0IE |= UCTXIE0 | UCSTTIE | UCSTPIE;                         // enable Transmit ready interrupt
}
//**********************************************************************************************************************************************************
static void M24LC512_initRead(void)
{
  UCB0CTLW0 &= ~UCTR;                         // UCTR=0 => Receive Mode (R/W bit = 1)
  UCB0IFG &= ~(UCRXIFG0 | UCSTPIFG);
  UCB0IE &= ~(UCTXIE0 | UCSTPIE | UCSTTIE);    // disable Transmit ready interrupt
  UCB0IE |= UCRXIE0;                          // enable Receive ready interrupt
}
//**********************************************************************************************************************************************************
void M24LC512_byteWrite(const uint16_t Address, const uint8_t Data)
{
    uint8_t adr_hi;
    uint8_t adr_lo;

    adr_hi = Address >> 8;                    // calculate high byte
    adr_lo = Address & 0xFF;                  // and low byte of address

    M24LC512_initWrite();

    UCB0CTLW0 |= UCTXSTT;                      // start condition generation
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = adr_hi;                        // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = adr_lo;                        // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = Data;                          // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0CTLW0 |= UCTXSTP;                      // I2C stop condition
    __bis_SR_register(LPM3_bits + GIE);

    UCB0IE &= ~(UCTXIE0 | UCSTPIE | UCSTTIE);    // disable Transmit ready interrupt
}
//**********************************************************************************************************************************************************
void M24LC512_pageWrite(uint16_t* StartAddress, uint8_t *Data, const uint16_t Size)
{
    volatile uint16_t i = 0;
    volatile uint16_t index = 0;
    volatile uint16_t counterI2cBuffer = 0;
    uint8_t adr_hi;
    uint8_t adr_lo;
    uint16_t currentAddress = *StartAddress;
    volatile uint16_t currentSize = Size;
    volatile uint16_t status = 0;
    uint8_t moreDataToRead = 1;
    uint16_t auxSize;
    uint32_t auxAddress = 128;
    uint32_t tempAddress;

    tempAddress = currentAddress;
    while(tempAddress > 128)
    {
        tempAddress = tempAddress - 128;
        auxAddress += 128;
    }
    tempAddress = auxAddress;

    // Execute until no more data in Data buffer
    while(moreDataToRead)
    {
        adr_hi = currentAddress >> 8;                               // calculate high byte
        adr_lo = currentAddress & 0x00FF;                           // and low byte of address

        // Chop data down to 64-byte packets to be transmitted at a time
        // Maintain pointer of current startaddress
        if(currentSize > M24LC512_MAXPAGEWRITE)
        {
            index = counterI2cBuffer;
            auxAddress = currentAddress;
            auxSize = currentSize;
            counterI2cBuffer = counterI2cBuffer + M24LC512_MAXPAGEWRITE;
            currentSize = currentSize - M24LC512_MAXPAGEWRITE;
            currentAddress = currentAddress + M24LC512_MAXPAGEWRITE;
        }
        else
        {
            moreDataToRead = 0;
            index = counterI2cBuffer;
            auxAddress = currentAddress;
            auxSize = currentSize;
            counterI2cBuffer = counterI2cBuffer + currentSize;
            currentAddress = currentAddress + currentSize;
        }
        status = __get_SR_register();
        if((status & C) || (status & (C | Z)))
        {
            countS++; // Contador de sobreescritura de la memoria.
            counterI2cBuffer = index + (0xFFFF - auxAddress) + 1;
            currentSize = auxAddress - (0xFFFF - auxSize) - 1;
            currentAddress = 0x0000;
            moreDataToRead = 1;
            goto jump;
        }
        if(currentAddress > tempAddress)
        {
            counterI2cBuffer = index + (tempAddress - auxAddress);
            currentSize = auxAddress - (tempAddress - auxSize);
            currentAddress = tempAddress;
            moreDataToRead = 1;
        }

        jump:
        if(tempAddress == 65536)
            tempAddress = 0;
        tempAddress += 128;

        M24LC512_initWrite();

        UCB0CTLW0 |= UCTXSTT;                                       // start condition generation => I2C communication is started
        __bis_SR_register(LPM3_bits + GIE);                         // Enter LPM0 w/ interrupts

        UCB0TXBUF = adr_hi;                                         // Load TX buffer
        __bis_SR_register(LPM3_bits + GIE);

        UCB0TXBUF = adr_lo;                                         // Load TX buffer
        __bis_SR_register(LPM3_bits + GIE);

        for(i = counterI2cBuffer ; i > index ; i--)
        {
            UCB0TXBUF = Data[(index + counterI2cBuffer) - i];   // Load TX buffer
            __bis_SR_register(LPM3_bits + GIE);
        }

        UCB0CTLW0 |= UCTXSTP;                                       // I2C stop condition
        __bis_SR_register(LPM3_bits + GIE);                         // Ensure stop condition got sent

        M24LC512_ackPolling();                                      // Ensure data is written in EEPROM
    }

    UCB0IE &= ~(UCTXIE0 | UCSTPIE | UCSTTIE);    // disable Transmit ready interrupt
    *StartAddress = currentAddress;
}
//**********************************************************************************************************************************************************
unsigned char M24LC512_currentRead(void)
{
    volatile uint8_t aux, temp;

    // Read Data byte
    M24LC512_initRead();

    UCB0CTLW0 |= UCTXSTT;                       // I2C start condition
    UCB0CTLW0 |= UCTXSTP;
    temp = UCB0RXBUF;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE |= UCSTPIE;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE &= ~(UCRXIE0 | UCSTPIE);

    return temp;
}
//**********************************************************************************************************************************************************
uint8_t M24LC512_randomRead(const uint16_t Address)
{
    uint8_t adr_hi;
    uint8_t adr_lo;
    volatile uint8_t temp, aux;

    adr_hi = Address >> 8;                      // calculate high byte
    adr_lo = Address & 0x00FF;                    // and low byte of address

    // Write Address first
    M24LC512_initWrite();

    UCB0CTLW0 |= UCTXSTT;                       // start condition generation
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = adr_hi;                         // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = adr_lo;    // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0CTLW0 |= UCTXSTP;                       // I2C stop condition
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    // Read Data byte
    M24LC512_initRead();

    UCB0CTLW0 |= UCTXSTT;                       // I2C start condition
    UCB0CTLW0 |= UCTXSTP;
    temp = UCB0RXBUF;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE |= UCSTPIE;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE &= ~(UCRXIE0 | UCSTPIE);

    return temp;
}
//**********************************************************************************************************************************************************
void M24LC512_sequentialRead(uint16_t Address , uint8_t *Data , uint16_t Size)
{
    uint8_t adr_hi;
    uint8_t adr_lo;
    uint16_t counterSize;

    adr_hi = Address >> 8;                      // calculate high byte
    adr_lo = Address & 0x00FF;                  // and low byte of address

    // Write Address first
    M24LC512_initWrite();

    UCB0CTLW0 |= UCTXSTT;                       // start condition generation
    __bis_SR_register(LPM3_bits + GIE);         // => I2C communication is started
                                                // Enter LPM0 w/ interrupts

    UCB0TXBUF = adr_hi;                         // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0TXBUF = adr_lo;                         // Load TX buffer
    __bis_SR_register(LPM3_bits + GIE);

    UCB0CTLW0 |= UCTXSTP;                       // I2C stop condition
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    // Read Data byte
    M24LC512_initRead();

    UCB0CTLW0 |= UCTXSTT;                       // I2C start condition

    for(counterSize = (Size-1) ; counterSize > 0  ; counterSize--)
    {
        __bis_SR_register(LPM3_bits + GIE);     // Enter LPM0 w/ interrupts
        Data[(Size-1) - counterSize] = UCB0RXBUF;
    }

    UCB0CTLW0 |= UCTXSTP;                       // I2C stop condition
    Data[Size-1] = UCB0RXBUF;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE |= UCSTPIE;
    __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

    UCB0IE &= ~(UCRXIE0 | UCSTPIE);
}
//**********************************************************************************************************************************************************
void M24LC512_ackPolling(void)
{
    do
    {
        UCB0IE |= UCTXIE0 | UCSTPIE | UCSTTIE;
        UCB0IFG = 0x00;                        // clear I2C interrupt flags
        UCB0CTLW0 |= UCTR;                     // I2CTRX=1 => Transmit Mode (R/W bit = 0)
        UCB0CTLW0 &= ~UCTXSTT;
        UCB0CTLW0 |= UCTXSTT;                  // start condition is generated

        while(UCB0CTLW0 & UCTXSTT)             // wait till I2CSTT bit was cleared
        {
            if(!(UCNACKIFG & UCB0IFG))           // Break out if ACK received
                break;
        }

        UCB0CTLW0 |= UCTXSTP;                   // stop condition is generated after. Wait till stop bit is reset
        __bis_SR_register(LPM3_bits + GIE);     // Enter LPM0 w/ interrupts

        UCB0IE &= ~(UCTXIE0 | UCSTPIE | UCSTTIE);

        __delay_cycles(1000);                          // delay

    } while(UCNACKIFG & UCB0IFG);

    UCB0IFG &= ~(UCTXIFG0 | UCSTTIFG | UCSTPIFG);
}
//**********************************************************************************************************************************************************//**********************************************************************************************************************************************************
void M24LC512_setinitValueHeader(void)
{
    uint16_t i = 0x0000;

    // Habilita las escrituras en la memoria FRAM.
    SYSCFG0 &= ~DFWP;

    // Se coloca en cero el contador de sobreescritura, el flag de perdida de datos y la fecha y hora de la ultima medicion medicion.
    for(i = 21 ; i > 7 ; i--)
    {
        myArray[i] = 0x00;
    }

    // Deshabilita las escrituras en la memoria FRAM.
    SYSCFG0 |= DFWP;
}
//**********************************************************************************************************************************************************
void M24LC512_updateHeader(const uint16_t currentAddress, const uint16_t size)
{
    uint16_t address;
    uint16_t count;

    // Habilita las escrituras en la memoria FRAM.
    SYSCFG0 &= ~DFWP;

    if((myArray[15] != 2) && (myArray[15] != 3))
    {
        // Obtengo el puntero que tiene la direccion a partir de la cual se debe enviar los datos cuando se soliciten por comando - Ptro de la comunicación.
        address = ((((uint16_t)(myArray[7])) << 8) | myArray[8]);

        // Obtengo el contador de sobreescrituras.
        count = ((((uint16_t)(myArray[13])) << 8) | myArray[14]);

        // Actualizo la dirección de bytes escritos en memoria - currentAddress - Puntero de la memoria
        myArray[9] = ((uint8_t)(currentAddress >> 8));  // Parte alta
        myArray[10] = (uint8_t)currentAddress;          // Parte baja

        // Actualizo el contador de sobreescrituras cuando ocurra una sobreescritura en la memoria.
        if(countS > count)
        {
            // Actualizo la cantidad de sobreescrituras (aprox. cada 7 dias) - count
            myArray[13] = ((uint8_t)(countS >> 8));  // Parte alta
            myArray[14] = (uint8_t)countS;           // Parte baja
        }

        // Verifica que si se han perdido datos.
        if((address <= currentAddress) && (countS > count))
        {
            myArray[15] = 1;
        }
    }

    switch(myArray[15])
    {
        case 1:
            // En caso de que se pierdan datos se va actualizando la dirección desde donde se debe enviar lo datos por RF.
            myArray[7] = ((uint8_t)(currentAddress >> 8)); // Parte alta.
            myArray[8] = ((uint8_t)currentAddress);        // Parte baja.
            break;

        case 2:
            // Cuando hubo una transmision exitosa el puntero de la comunicacion se actualiza al puntero de la memoria para enviar nuevos datos.
            myArray[7] = myArray[9];
            myArray[8] = myArray[10];

            // Actualizo a cero el flag.
            myArray[15] = 0;           // Como ya se indico que los si los datos se perdieron o no se pone a cero para poder indicar nuevamente cuando ocurra una nueva perdidad de datos.
            break;

        case 3:
            // Actualizo la direccion de inicio donde se guardan los datos en la memoria - startAddress - Puntero de la comunicacion
            myArray[7] = 0x00;  // Parte alta
            myArray[8] = 0x00;  // Parte baja

            // Actualizo la dirección de bytes escritos en memoria al inicio - currentAddress - Puntero de la memoria
            myArray[9] = 0x00;  // Parte alta
            myArray[10] = 0x00; // Parte baja

            // Se pone a cero el contador de sobreescrituras - count
            myArray[13] = 0x00;  // Parte alta
            myArray[14] = 0x00;  // Parte baja

            // Se pone a cero el flag de perdidas de datos.
            myArray[15] = 0;
            break;
    }

    // Actualizo la cantidad de bytes escritos en memoria - size
    myArray[11] = ((uint8_t)(size >> 8));  // Parte alta
    myArray[12] = ((uint8_t)size);         // Parte baja

    // Deshabilita las escrituras en la memoria FRAM.
    SYSCFG0 |= DFWP;
}
//**********************************************************************************************************************************************************
bool M24LC512_memoryCheck(void)
{
    uint8_t contDo = 3;
    static uint8_t temp = 0;
    uint8_t contWhile = 3;

    do
    {
        UCB0IE |= UCTXIE0 | UCSTPIE | UCSTTIE;
        UCB0IFG = 0x00;                        // clear I2C interrupt flags
        UCB0CTLW0 |= UCTR;                     // I2CTRX=1 => Transmit Mode (R/W bit = 0)
        UCB0CTLW0 &= ~UCTXSTT;
        UCB0CTLW0 |= UCTXSTT;                  // start condition is generated

        while((UCTXSTT & UCB0CTLW0))           // wait till I2CSTT bit was cleared
        {
            if((!(UCNACKIFG & UCB0IFG)) || (contWhile > 0))           // Break out if ACK received
                break;
            contWhile--;
        }

        --contDo;
        contWhile = 3;

        __delay_cycles(1000);

        if(contDo == 0) break;

    }while(UCNACKIFG & UCB0IFG);

    if(contDo != 0 && !(UCNACKIFG & UCB0IFG) && !(UCTXSTT & UCB0CTLW0) && (UCTXIFG0 & UCB0IFG))
    {
        // Es necesario hacer una transmision completa para verificar la conexion si no no envia el stop lo que ocasiona problemas en posteriores accesos de memoria.
        __bis_SR_register(LPM3_bits + GIE);         // for start

        UCB0TXBUF = 0x00;                           // Load TX buffer
        __bis_SR_register(LPM3_bits + GIE);

        UCB0TXBUF = 0x00;                           // Load TX buffer
        __bis_SR_register(LPM3_bits + GIE);

        UCB0CTLW0 |= UCTXSTP;                       // I2C stop condition
        __bis_SR_register(LPM3_bits + GIE);         // Enter LPM0 w/ interrupts

        UCB0IE &= ~(UCTXIE0 | UCSTPIE | UCSTTIE);
        UCB0IFG &= ~(UCTXIFG0 | UCSTTIFG | UCSTPIFG);

        if(myArray[15] == 4)    // Verifica si anteriormente no habia estado repondiendo.
        {
            SYSCFG0 &= ~DFWP;   // Habilita las escrituras en la memoria FRAM.
            myArray[15] = temp; // En caso de que, anteriormente, no hubiese respondido y luego al intentarlo de nuevo (o reiniciarlo) vuelve a responder y retome el valor que tenia.
            SYSCFG0 |= DFWP;    // Deshabilita las escrituras en la memoria FRAM.
        }else
        {
            temp = myArray[15]; // Se va almacenando el ultimo valor para luego si deja de responder y vuelve a responder retoma el valor de antes.
        }

        return true;

    }else if((contDo == 0 && (UCNACKIFG & UCB0IFG) && !(UCTXSTT & UCB0CTLW0) && (UCTXIFG0 & UCB0IFG)) ||
             (contDo != 0 && !(UCNACKIFG & UCB0IFG) && (UCTXSTT & UCB0CTLW0) && !(UCTXIFG0 & UCB0IFG)))
    {
        UCB0IE &= ~(UCTXIE0 | UCSTPIE | UCSTTIE);
        UCB0IFG &= ~(UCTXIFG0 | UCSTTIFG | UCSTPIFG);
        UCB0CTLW0 &= ~UCTXSTT;

        SYSCFG0 &= ~DFWP;
        myArray[15] = 4;
        SYSCFG0 |= DFWP;

        return false;
    }
    return 0;
}
//********************************************************************************************************************************************************************
// I2C interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCIB0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:                 // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:                 // Vector 8: STPIFG

        __bic_SR_register_on_exit(LPM3_bits + GIE);
        break;

    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 14: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 16: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 18: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 20: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 22: TXIFG1
    case USCI_I2C_UCRXIFG0:                 // Vector 24: RXIFG0
    case USCI_I2C_UCTXIFG0:                 // Vector 26: TXIFG0

        __bic_SR_register_on_exit(LPM3_bits + GIE);
        break;

    case USCI_I2C_UCBCNTIFG: break;         // Vector 28: BCNTIFG
    case USCI_I2C_UCCLTOIFG: break;         // Vector 30: clock low timeout
    case USCI_I2C_UCBIT9IFG: break;         // Vector 32: 9th bit
    default: break;
  }
}
