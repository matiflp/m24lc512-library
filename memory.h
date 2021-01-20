/**
  * @file     mamory.h
  * @brief    Driver para realizar una comunicacion I2C.
  * @date     Created on: 30 oct. 2019
  * @authors  Mat�as L�pez - Jes�s L�pez
  * @version  1.0
  */
//*****************************************************************************
//
// memory.h - Driver para realizar una comunicacion I2C.
//
//*****************************************************************************

#ifndef MEMORY_H_
#define MEMORY_H_

//*****************************************************************************
//                              Include
//*****************************************************************************
#include "driverlib.h"

//*****************************************************************************
//                              Defines
//*****************************************************************************
//*****************************************************************************
//! @name Cabecera en Memoria FRAM:
//! \details Direcci�n de la FRAM donde se van a guardan los datos de
//!          configuraci�n.
//! @{
//*****************************************************************************
#define FRAM_START 0x1800

//*****************************************************************************
//! \details Puntero que indica la configuracion utilizada, es decir, los
//!          sensores que se estan utilizando.
//*****************************************************************************
extern uint8_t *myArray;

//*****************************************************************************
//! \details Contador que indica que hubo una sobreescritura en la memoria,
//!          que se transmitira, mediante el comando EM, para permitir realizar
//!          un control de los datos.
//*****************************************************************************
extern uint16_t countS;

//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//! @name Memoria 24LC512:
//! \brief Comandos a utilizar con la memoria I2C 24LC512.
//! @{
//*****************************************************************************
//*****************************************************************************
//! \details Comando que indica la direcci�n de la memoria I2C que el \b MCU
//!          deber� enviar por el pin \b SDA para poder comunicarse.
//*****************************************************************************
#define M24LC512_I2C_ADDRESS    0x50

//*****************************************************************************
//! \details M�xima cantidad de bytes que se pueden escribir por p�gina.
//*****************************************************************************
#define M24LC512_MAXPAGEWRITE   128

//*****************************************************************************
//! \details Direcci�n de inicio de escritura en la memoria externa.
//*****************************************************************************
#define M24LC512_STARTADDRESS   0x0000

//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//                              Configuraci�n de Pines
//*****************************************************************************
//*****************************************************************************
//! @name Configuraci�n de pines:
//! \brief Configuraci�n de los pines que se utilizan en la comunicaci�n I2C.
//! @{
//*****************************************************************************
//*****************************************************************************
//! \details Registro utilizado para seleccionar la funci�n de la comunicaci�n
//!          I2C.
//*****************************************************************************
#define M24LC512_PORT_SEL  P5SEL0

//*****************************************************************************
//! \details Registro utilizado para poner un alto o un bajo o, si el registro
//!          PxDIR se coloca como entrada, seleccionar entre resistencias de
//!          PullUp/PullDown dependiendo de la necesidad.
//*****************************************************************************
#define M24LC512_PORT_OUT  P5OUT

//*****************************************************************************
//! \details Registro para habilitar la resistencias de PullUp/PullDown si el
//!          registro PxDIR se coloca como entrada.
//*****************************************************************************
#define M24LC512_PORT_REN  P5REN

//*****************************************************************************
//! \details Registro en el que los pines se puede colocar como entrada o
//!          salida.
//*****************************************************************************
#define M24LC512_PORT_DIR  P5DIR

//*****************************************************************************
//! \details Pin de datos de la comunicaci�n I2C.
//*****************************************************************************
#define M24LC512_PIN_SDA   BIT2                  // UCB0SDA pin

//*****************************************************************************
//! \details Pin de sincronizaci�n de la comunicaci�n I2C.
//*****************************************************************************
#define M24LC512_PIN_SCL   BIT3                  // UCB0SCL pin

//*****************************************************************************
//! \details Valor con el que se divide la fuente de reloj seleccionada y que se
//!          utiliza para obtener una frecuancia de 100KHz necesaria para la
//!          comunicaci�n I2C.
//*****************************************************************************
#define M24LC512_PIN_SCL_CLOCK_DIV 20                // SCL clock divider

//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//                              Funciones prototipos
//*****************************************************************************
//*****************************************************************************
//! \brief Configura e inicializa la comunicaci�n I2C.
//!
//! \details \b Descripci�n \n
//!          Funci�n que se encarga de configurar la comunicaci�n I2C. Primero
//!          en el registro \b P5SEL0 se selecciona la funci�n primaria del
//!          m�dulo seteando un 1 en los pines \b P5.2 y \b P5.3
//!          correspondientes a los pines SDA y SCL respectivamente, como se
//!          indica la tabla <em> "Table 6-20. Port P5.0, P5.1, P5.2, and P5.3
//!          Pin Functions" </em> en la secci�n <em>"6.9.13 Input/Output
//!          Schematics"</em> del datasheet <em>"MSP430FR413x mixed-signal
//!          microcontrollers"</em>. Luego para configurar la comunicaci�n se
//!          setea el bit \b UCSWRST y as� evitar comportamientos impredecibles.
//!          Ahora se configura la comunicaci�n utilizando el registro
//!          \b UCB0CTLW0 en donde se configura en el modo I2C, en el modo
//!          maestro, se setea el bit \b UCSYNC ya que el fabricante indica que
//!          en el eUSI_B se debe poner en 1. Tambi�n se configura como
//!          transmisor y se selecciona como fuente de reloj el \b SMCLK y
//!          luego se coloca un divisor igual a 0xA en el registro \b UCB0BRW
//!          que es el divisor de la fuente de reloj que har� que la frecuencia
//!          sea de 100KHz. Posteriormente en el registro \b UCB0I2CSA se le
//!          asigna la direcci�n del esclavo con el que se quiere comunicar.
//!          Mediante un \c if() se verifica si el bus est� ocupado, si resulta
//!          verdadero desactiva la se�al de reloj y la genera manualmente.
//!
//! \return \c void.
//!
//! \attention Modifica los bits de los registros \b P5SEL, \b UCB0CTLW0,
//!            \b UCB0BRW, \b UCB0I2CSA, \b P5DIR, \b P5OUT, \b UCB0IE y
//!            \b UCB0IFG.
//*****************************************************************************
void M24LC512_initPort(void);

//*****************************************************************************
//! \brief Configura el MCU para realizar una escritura.
//!
//! \details \b Descripci�n \n
//!          Funci�n que se encarga de configurar el \b MCU en el modo
//!          transmisi�n para realizar una escritura en el dispositivo
//!          direccionado, luego se limpia la bandera \b UCTXIFG0 por si
//!          existiese alguna interrupci�n pendiente, se deshabilita la
//!          interrupci�n \b UCRXIE0 de recepci�n pendiente y por �ltimo se
//!          habilitan las interrupciones \b UCTXIE0, \b UCSTTIE y \b UCSTPIE
//!          utilizadas en la trasmisi�n de datos.
//!
//! \return \c void.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG y
//!            \b UCB0IE.
//*****************************************************************************
static void M24LC512_initWrite(void);

//*****************************************************************************
//! \brief Configura el MCU para realizar una lectura.
//!
//! \details \b Descripci�n \n
//!          Funci�n que se encarga de configurar el \b MCU en el modo
//!          recepci�n para realizar una lectura del dispositivo
//!          direccionado, luego se limpia la bandera \b UCRXIFG0 por si
//!          existiese alguna interrupci�n pendiente, se deshabilita la
//!          interrupci�n \b UCTXIE0 de una transmisi�n pendiente, el bit
//!          \b UCSTTIE que genera una condici�n de start y el bit \b UCSTPIE
//!          que genera una condici�n de stop, y por �ltimo se habilitan las
//!          interrupci�n \b UCRXIE0, utilizada en la recepci�n de datos.
//!
//! \return \c void.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG y
//!            \b UCB0IE.
//*****************************************************************************
static void M24LC512_initRead(void);

//*****************************************************************************
//! \brief Realiza la escritura de un solo byte en la memoria EEPROM.
//!
//! \details \b Descripci�n \n
//!          Esta funci�n se encarga de escribir en una direcci�n de memoria un
//!          byte de datos. Para llevar a cabo esto primero se separa
//!          la direcci�n en bytes de parte alta y parte baja ya que es una
//!          direcci�n compuesta por 2 bytes. Despu�s se coloca como transmisor
//!          para luego empezar la comunicaci�n enviando un start, que trasmite,
//!          en conjunto, la direcci�n del dispositivo con el que se esta
//!          estableciendo la comunicaci�n y, posteriormente, se transmite la
//!          direcci�n de memoria que se esta intentando acceder para escribir
//!          en la misma, empezando por la parte alta y luego la parte baja.
//!          Una vez establecida la comunicaci�n con el esclavo se env�a el
//!          dato que se desea escribir en la memoria. Por �ltimo, se
//!          transmite una condici�n de Stop para finalizar la comunicaci�n.
//!          Se puede notar que una vez se realiza una transmisi�n de alguna
//!          parte de la trama se entra a un modo bajo consumo y se habilitan
//!          las interrupciones generales, de modo que cuando se active alguna
//!          interrupci�n de pedido de atenci�n vaya a parar a esa interrupcion,
//!          lo que indica que ya se transmiti� el byte en cuesti�n. Entonces
//!          en la interrupci�n se sale del bajo consumo, se deshabilitan las
//!          interrupciones globales para luego continuar con la normal
//!          ejecuci�n del programa.
//!
//! \param address Direcci�n del dispositivo con el que el micro quiere
//!        establecer una comunicaci�n.
//! \param data Informaci�n que el usuario desea escribir en la memoria.
//!
//! \return \c void.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b UCB0TXBUF, \b SR y \b UCB0IE.
//*****************************************************************************
void M24LC512_byteWrite(const uint16_t Address , const uint8_t Data);

//*****************************************************************************
//! \brief Funci�n que realiza una escritura en la memoria por p�gina,
//!        compuestas por 128 bytes.
//!
//! \details \b Descripci�n \n
//!          Funci�n que se encarga de escribir una cantidad x de datos
//!          indicados por el usuario y con una cantidad m�xima de 128 bytes
//!          por p�gina.  Si el maestro debe transmitir m�s de 128 bytes antes
//!          de generar una condici�n de stop, el puntero de direcciones interno
//!          de la memoria, que apunta a una direcci�n de memoria a la que se
//!          encuentra actualmente accediendo, comenzar� de nuevo al superarse
//!          la cantidad de bytes por p�gina y los datos recibidos anteriormente
//!          se sobrescribir�n, por tal motivo se utiliza un \c while() en
//!          conjunto con \c if-else para diferenciar si se quiere escribir una
//!          p�gina completa de la memoria o una cantidad menor y as� evitar
//!          perdida de datos.
//!          En conjunto con la condici�n de start del maestro, se transmite
//!          el c�digo de control (cuatro bits), la Selecci�n de Chip (tres
//!          bits) y el bit R/W en "0" en el bus (que indica una escritura).
//!          Esto indica al receptor esclavo direccionado que el byte alto de
//!          la direcci�n seguir� despu�s de que haya generado un \b ACK
//!          durante el noveno ciclo de reloj. Por lo tanto, el siguiente byte
//!          transmitido por el maestro es el byte de orden superior de la
//!          palabra direcci�n y se escribir� en la direcci�n puntero de la
//!          \b 24LC512. El siguiente byte es el byte de direcci�n menos
//!          significativo. Despu�s de recibir otra se�al de reconocimiento
//!          \b ACK del \b 24LC512, el dispositivo maestro transmitir� la
//!          palabra de datos que se escribir� en la ubicaci�n de memoria
//!          direccionada. El \b 24LC512 reconoce nuevamente y el maestro
//!          transmite hasta 127 bytes adicionales, que se almacenan
//!          temporalmente en el b�fer de p�gina en el chip y se escribir�n en
//!          la memoria despu�s de que el maestro haya transmitido una condici�n
//!          de stop. Despu�s de recibir cada palabra, los siete bits
//!          inferiores del puntero de direcci�n de la memoria se incrementan
//!          internamente en uno.
//!          Luego de ese ciclo de escritura se realiza un \a M24LC512_ackPolling()
//!          para esperar a que la memoria est� disponible, ya que mientras esta
//!          escribiendo los datos indicados por el usuario, esta no responder� a los
//!          pedidos del \b MCU.
//!          Al igual que con la operaci�n de escritura de bytes, una vez que
//!          se recibe la condici�n Stop, comenzar� un ciclo de escritura
//!          interno. Si se intenta escribir en la matriz con el pin WP en
//!          alto, el dispositivo reconocer� el comando, pero no escribir�,
//!          ocurrir� el ciclo, no se escribir�n datos y el dispositivo
//!          aceptar� inmediatamente un nuevo comando.
//!
//! \param Address Direcci�n de la memoria desde donde el micro escribir� los
//!        bytes de datos.
//! \param *Data Puntero donde se almacenar�n los datos, que se quieren escribir
//!        en la memoria.
//! \param Size Cantidad de datos a escribir en la memoria.
//!
//! \return \c void.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b UCB0TXBUF ,\b SR y \b UCB0IE.
//*****************************************************************************
void M24LC512_pageWrite(uint16_t* StartAddress , uint8_t *Data,
                        const uint16_t Size);

//*****************************************************************************
//! \brief Realiza una lectura de datos en la direcci�n actual en que se
//!        encuentre la memoria.
//!
//! \details \b Descripci�n \n
//!          El \ b 24LC512 contiene un contador de direcciones que mantiene
//!          la direcci�n de la �ltima palabra accedida, incrementada
//!          internamente en "1". Por lo tanto, si el acceso de lectura
//!          anterior era a la direcci�n 'n' (n es cualquier direcci�n v�lida),
//!          la siguiente operaci�n de lectura de direcci�n actual acceder�a a
//!          los datos de la direcci�n n + 1.
//!          Al recibir el byte de control con \b R/W bit establecido en
//!          '1', el \b 24LC512 emite un acknowledge(\b ACK) y transmite la
//!          palabra de datos de 8 bits. El maestro emite un \b NACK que indica
//!          el recibimiento de la transferencia, genera una condici�n de stop
//!          y el \b 24LC512 interrumpe la transmisi�n.
//!
//! \note Se env�a la condici�n de stop antes de almacenar el dato
//!       sino la memoria continuar�a enviando datos como si se tr�tese de
//!       una lectura secuencial.
//!
//! \return \c UCB0RXBUF Valor que se encuentra almacenado en la memoria.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b UCB0RXBUF, \b SR y \b UCB0IE.
//*****************************************************************************
unsigned char M24LC512_currentRead(void);

//*****************************************************************************
//! \brief Operaci�n de lectura aleatoria. Los datos se leen desde la EEPROM.
//!        La direcci�n EEPROM se define con el par�metro \b Address.
//!
//! \details \b Descripci�n \n
//!          Las operaciones de lectura aleatoria permiten al maestro acceder
//!          a cualquier ubicaci�n de memoria de manera aleatoria. Para
//!          realizar este tipo de operaci�n de lectura, primero se debe
//!          establecer la direcci�n a la cual se quiere acceder. Esto se hace
//!          enviando la direcci�n de la palabra al \b 24LC512 como parte de
//!          una operaci�n de escritura (bit R/W establecido en "0"). Despu�s
//!          de que se env�a la direcci�n y recibir el \b ACK, el maestro
//!          genera una condici�n de start. Esto finaliza la operaci�n de
//!          escritura, pero no antes de establecer el puntero de direcci�n
//!          interno de la memoria en la direcci�n indicada. Luego, el maestro
//!          emite el byte de control nuevamente, pero con el bit R/W
//!          establecido en uno. El \b 24LC512 emitir� un \b ACK y transmitir�
//!          el byte de datos. El maestro emite un \b NACK, y genera una
//!          condici�n de detenci�n que hace que el \b 24LC512 interrumpa
//!          transmisi�n. Despu�s de un comando de lectura aleatorio, el
//!          contador de direcci�n interno apuntar� a la ubicaci�n de la
//!          direcci�n despu�s de la que se acaba de leer.
//!
//! \note Se env�a la condici�n de stop antes de almacenar el dato
//!       sino la memoria continuar�a enviando datos como si se tr�tese de
//!       una lectura secuencial.
//!
//! \param address Direcci�n de la memoria de donde el micro quiere obtener un
//!        byte de datos.
//!
//! \return \c UCB0RXBUF Valor que se encuentra almacenado en la memoria.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b UCB0TXBUF ,\b UCB0RXBUF, \b SR y \b UCB0IE.
//*****************************************************************************
uint8_t M24LC512_randomRead(const uint16_t Address);

//*****************************************************************************
//! \brief Operaci�n de lectura secuencial. Los datos se leen desde la EEPROM
//!        en forma secuencial desde el par�metro \b address  como punto de
//!        partida. Especifique el tama�o que se leer� y completar� en un b�fer
//!        de datos.
//!
//! \details \b Descripci�n \n
//!          Las operaciones de lectura secuencial permiten al maestro acceder
//!          a cualquier ubicaci�n de memoria y a partir de ah� leer la cantidad
//!          de datos que se requieran. Para realizar este tipo de operaci�n de
//!          lectura, primero se debe establecer la direcci�n a la cual se
//!          quiere acceder. Esto se hace enviando la direcci�n de la palabra
//!          al \b 24LC512 como parte de una operaci�n de escritura (bit R/W
//!          establecido en "0"). Despu�s de que se env�a la direcci�n y
//!          recibir el \b ACK, el maestro genera una condici�n de stop.
//!          Esto finaliza la operaci�n de escritura, pero no antes de
//!          establecer el puntero de direcci�n interno de la memoria en la
//!          direcci�n indicada. Luego, el maestro emite el byte de control
//!          nuevamente, pero con el bit R/W establecido en uno.
//!          El \b 24LC512 emitir� un \b ACK y transmitir�el byte de datos.
//!          Despu�s de que el \b 24LC512 transmite el primer
//!          byte de datos, el maestro emite un \b ACK. Este reconocimiento
//!          dirige al \b 24LC512 a transmitir la siguiente palabra de 8 bits
//!          dirigida secuencialmente. Despu�s del byte final transmitido al
//!          maestro, el maestro emite un \b NACK, y generar� una condici�n de
//!          stop. Para proveer lecturas secuenciales, el \b 24LC512
//!          contiene un puntero de direcci�n interno que se incrementa en uno
//!          al finalizar cada operaci�n. Este puntero de direcci�n permite
//!          que todo el contenido de la memoria se lea en serie durante una
//!          operaci�n. El puntero de direcci�n interno pasar� autom�ticamente
//!          de la direcci�n FFFF a la direcci�n 0000 si el maestro reconoce
//!          el byte recibido de la direcci�n de matriz FFFF.
//!
//! \note Se env�a la condici�n de stop antes de almacenar el dato
//!       sino la memoria enviara un datos de mas que genera problemas en
//!       la lectura.
//!
//! \param Address Direcci�n inicial de la memoria a partir de la cual el
//!        microcontrolador quiere obtener una cierta cantidad de datos.
//! \param *Data Puntero donde se almacenan los datos leidos en forma
//!        secuencial en la memoria desde una direcci�n incial indicada.
//! \param Size Cantidad de datos a leer en la memoria.
//!
//! \return \c void
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b UCB0TXBUF ,\b UCB0RXBUF, \b SR y \b UCB0IE.
//*****************************************************************************
void M24LC512_sequentialRead(uint16_t Address, uint8_t *Data,
                             uint16_t Size);

//*****************************************************************************
//! \brief Acknowledge Polling. La EEPROM no reconocera al MCU si hay un ciclo
//!        de escritura en curso. Se puede usar para determinar cu�ndo se
//!        completa un ciclo de escritura.
//!
//! \details \b Descripci�n \n
//!          Como el dispositivo no reconocer� ning�n comando durante un ciclo
//!          de escritura, esta funci�n se puede usar para determinar cu�ndo se
//!          completa el ciclo (esta funci�n se puede usar para maximizar el
//!          rendimiento del bus).
//!          Una vez que el maestro ha emitido la condici�n de stop para un
//!          comando de escritura, lo cual se realiza en otras funciones de
//!          escritura, el dispositivo inicia el ciclo de escritura temporizado
//!          internamente. El \b ACK polling puede iniciarse de inmediato. Esto
//!          implica que el maestro env�e una condici�n de start, seguido del
//!          byte de control para un comando de Escritura (R / W = 0).
//!          Si el dispositivo todav�a est� ocupado con el ciclo de escritura,
//!          no se devolver� ning�n ACK. Si no se devuelve ning�n \b ACK, el
//!          bit de inicio y el byte de control deben volver a enviarse. Si el
//!          ciclo se completa, el dispositivo devolver� el \b ACK y luego se
//!          termina la comunicaci�n enviando una condici�n de stop y el
//!          maestro podr� continuar con el siguiente comando de  lectura o
//!          escritura.
//!
//! \return \c void.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b SR, \b TA1CTL, \b TA1CCTL0 y\b TA1CCR0.
//*****************************************************************************
void M24LC512_ackPolling(void);

//*****************************************************************************
//! \brief Establece los valores iniciales de la cabecera en la memoria.
//!
//! \details \b Descripci�n \n
//!          Se establecen las direcciones y valores iniciales de la cabecera
//!          en la memoria externa.
//!
//! \return \c void.
//*****************************************************************************
void M24LC512_setinitValueHeader(void);

//*****************************************************************************
//! \brief Actualiza la cabecera en memoria.
//!
//! \details \b Descripci�n \n
//!          Se encarga de ir actualizando la cabecera en memoria cada vez
//!          que ocurre una escritura en la misma o cuando se reciba una
//!          comando que asi lo requiera.
//!
//! \param currentAddress Direccion actual que apunta a la proxima direcci�n
//!                       de la memoria a escribir.
//! \param size Indica la cantidad de bytes escritos en la memoria.
//!
//! \return \c void.
//*****************************************************************************
void M24LC512_updateHeader(const uint16_t currentAddress, const uint16_t size);

//*****************************************************************************
//! \brief Realiza un chequeo del estado de la memoria.
//!
//! \details \b Descripci�n \n
//!          Funci�n que comprueba si la memoria funciona correctamente
//!          enviando un start una cierta cantidad de veces y verifica si este
//!          se transimite correctamente, es decir, verifica si recibe un ACK
//!          o si falla la comunicacion. Si el start es reconocido se realiza
//!          una transmision cualquiera para no generar problemas en
//!          posteriores escrituras ya que al transmitir solo el stop genera
//!          problemas en posteriores accesos a la memoria.
//!
//! \return Indica si hubo exito, o fallo, la comunicacion con la memoria.
//!
//! \attention Modifica los bits de los registros \b UCB0CTLW0, \b UCB0IFG,
//!            \b SR, \b TA1CTL, \b TA1CCTL0 y\b TA1CCR0.
//*****************************************************************************
bool M24LC512_memoryCheck(void);

#endif /* MEMORY_H_ */
