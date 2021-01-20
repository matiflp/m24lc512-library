# Library for memory 24LC512

This library allows us to handle the 24LC512 memory, where you can take advantage of all the functions that this memory has (see device datasheet). To move through memory, it has a header, located in the FRAM of the MCU starting at address 0x1800 (using the "myArray" array), with the following elements:
. Communication pointer (myArray [7] - myArray [8]): This pointer is intended for when you want to get the amount of data stored up to that moment.
. Memory pointer (myArray [9] - myArray [10]): This pointer allows you to update the address from where you are writing.
. Amount of data (myArray [11] - myArray [12]): Indicates the amount of data written to memory. Useful for when you want to get the data with the communication pointer.
. Overwrites (myArray [13] - myArray [14]): Indicates the overwrites were in memory.
. data loss flag (myArray [15]): Indicates if there was a loss of data due to an overwrite.

Finally, it has a memory check function to verify if the memory stops responding or has broken.

Authors:
.Matías López (https://github.com/matiflp)
.Jesús López (https://github.com/jesu95)

Note: This project is based on one that I found on the internet a while ago but I don't remember where it is from, if anyone knows who it belongs to, please let me know to give them the credits.
