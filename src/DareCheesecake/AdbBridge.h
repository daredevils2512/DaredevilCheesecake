/*
 * RunADBCommand.h
 *
 *  Created on: Jul 16, 2017
 *      Author: daredevils
 */

#ifndef SRC_DARECHEESECAKE_ADBBRIDGE_H_
#define SRC_DARECHEESECAKE_ADBBRIDGE_H_
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
class AdbBridge {
public:
	static bool runCommand(const char* command);
	static void start();
	static void stop();
	static void restartAdb();
	static void portForward(int local_post, int remote_port);
	static void reversePortForward(int remote_port, int local_port);
	static void restartApp();
};

#endif /* SRC_DARECHEESECAKE_ADBBRIDGE_H_ */
