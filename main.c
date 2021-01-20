#include "memory.h"

uint8_t read_val[60];
uint8_t write_val[60];

int main(void)
{
    myArray = (uint8_t *) 0x1800;               // Start of FRAM block of 512 bytes

    uint8_t i = 0;
    uint16_t addr = 0;
    uint8_t data[6];

    WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog timer

    M24LC512_initPort();                        // Initialize I2C module

    M24LC512_byteWrite(0xF9FF,0x88);
    M24LC512_ackPolling();                      // Wait for EEPROM write cycle
                                                // completion
    M24LC512_byteWrite(0xFA00,0x96);
    M24LC512_ackPolling();                      // Wait for EEPROM write cycle
                                                // completion
    M24LC512_byteWrite(0xFA01,0x57);
    M24LC512_ackPolling();                      // Wait for EEPROM write cycle
                                                // completion
    M24LC512_byteWrite(0xFA02,0x75);
    M24LC512_ackPolling();                      // Wait for EEPROM write cycle
                                                // completion
    M24LC512_byteWrite(0xFA03,0x9B);
    M24LC512_ackPolling();                      // Wait for EEPROM write cycle
                                                // completion
    M24LC512_byteWrite(0xFA04,0xBA);
    M24LC512_ackPolling();                      // Wait for EEPROM write cycle
                                                // completion

    data[0] = M24LC512_randomRead(0xF9FF);      // Read from address 0xF9FF
    data[1] = M24LC512_currentRead();           // Read from address 0xFA00
    data[2] = M24LC512_currentRead();           // Read from address 0xFA01
    data[3] = M24LC512_currentRead();           // Read from address 0xFA02
    data[4] = M24LC512_currentRead();           // Read from address 0xFA03
    data[5] = M24LC512_currentRead();           // Read from address 0xFA04

    // Fill write_val array with counter values
    for(i = 0 ; i < 60 ; i++)
    {
        write_val[i] = i;
    }

    addr = 0x0064;                              // Set starting address
      // Write a sequence of data array
    M24LC512_pageWrite(&addr , write_val , 60);
    // Read out a sequence of data from EEPROM
    M24LC512_sequentialRead(addr, read_val , 60);

    __no_operation();
    while(1);
}
