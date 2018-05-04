#include "stdafx.h"

#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

/*
The following message parser assumes that the serial communication
has some form of handshake method confirming transmission from source.
It also builds on the assumption that the main function runs on a loop.
*/

// Assuming all relevant precurser functions/variables are initialized here.

bool message_parser() {
	/*
	This function parses the serial communication data using the given
	read() function. Here it is assumed that the little-endian order is
	handled within the process_message() function.

	Inputs:		None
	Outputs:	true: Message parsed successfully.
				false: Message parse failed.
	*/

	// Initial boolean parameters.
	bool messageParsed = false;
	bool startFlag = false;
	bool header = false;
	bool payloadUnit = false;

	// Used for identifying ending checksums presents.
	uint32_t checkSum = 0;
	uint32_t messageSum = 0;

	// Main loop for parsing message.
	while (!messageParsed) {

		// Checking to see if first read is the start flag.
		if ((read(*buffer, 5) == 5) && !startFlag) {
			for (int i = 0; i < 5; i++) {
				if (*buffer[i] == 0xFF) {
					startFlag = true;
				}
				else {
					startFlag = false;
				}
			}
		}

		// If start flag is acquired, reading header is attempted. 
		if ((read(*buffer[5], 5) == 5) && startFlag) {

			// Checking if seperator is present.
			if (*buffer[9] == 0xfe) {
				
				// Keeping track of the messageSum while skipping seperator.
				for (int i = 5; i < 9; i++) {
					messageSum += *buffer[i];
				}
				
				header = true;
			}
		}

		// Once start flag and header are acquired, repeating payload units are read. 
		if (startFlag && header) {

			// Establishing initial buffer index for payload units.
			int payloadI = 10;

			// Continously reading for payload units until checksum end is confirmed.
			while (!payloadUnit) {

				// Checking to see if the maximum message length is reached.
				if (payloadI > 95) {
					cout << "Parsing Timed Out" << endl;
					return false;
				}

				// Reading next payload unit.
				if (read(*buffer[payloadI], 5) == 5) {

					// Resetting current unit's message unitSum to 0.
					uint32_t unitSum = 0;

					// Checking to see if last unit read is the XOR checksum while skipping seperators.
					if (*buffer[payloadI + 4] == 0xFE) {
						
						// Updating sums.
						for (int i = payloadI; i < payloadI + 4; i++) {
							messageSum += *buffer[i];
							unitSum += *buffer[i];
						}
						
						// Converting the full messageSum to XOR checksum.
						checkSum ^= messageSum;

						// Checking if the full message checksum corresponds to the last unitSum.
						if (checkSum == unitSum) {

							// Returns true since message is recieved.
							payloadUnit = true;
							messageParsed = true;
							return true;
						}
					}

					// Checking to see if last unit read ended as it should.
					if (*buffer[payloadI + 4] == 0xfe) {

						// Moving the buffer index to read the next unit.
						payloadI += 5;
					}
				}
			}
		}
	}

	// If parsing failed.
	cout << "Message Parse Failed" << endl;

	return false;
}

int main() {
	// Updating buffer and waiting for a full message parse.
	bool parsed = message_parser();

	// Processing message after parsing. 
	if (parsed) {
		process_message(*buffer, *buffer[5]);
	}
}

