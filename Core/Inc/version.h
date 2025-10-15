/*
 * version.h
 *
 *  Created on: Jun 11, 2025
 *      Author: J. Früh
 */

#ifndef INC_VERSION_H_
#define INC_VERSION_H_

#define PROJECT_NAME "Dcc_Decoder"
#define FIRMWARE_VERSION 1

// Hardware version definition for GPIO backward compatibility
// (Only GPIO pin shift are possible)
#define HARDWARE_VERSION_1v1 11  // No more compatible
#define HARDWARE_VERSION_1v2 12  // No more compatible
#define HARDWARE_VERSION_1v4 14
#define HARDWARE_VERSION_1v5 15

#define HARDWARE_VERSION HARDWARE_VERSION_1v5


#endif /* INC_VERSION_H_ */
