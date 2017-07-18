/*
 * VisionServer.cpp
 *
 *  Created on: Jul 16, 2017
 *      Author: daredevils
 */

#include <DareCheesecake/VisionServer.h>
#include "../json.hpp"
using json = nlohmann::json;

VisionServer::VisionServer(){
	resetVars();
}
void VisionServer::resetVars() {
	socketfd=-1;
	clientfd = -1;
	pendingRestart = false;
	mIsConnected=false;
	hasSetup=false;
	lastMessageReceivedTime=0;
	isActive=false;
}
template<typename Out>
void VisionServer::splitr(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}
std::vector<std::string> VisionServer::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    splitr(s, delim, std::back_inserter(elems));
    return elems;
}

void VisionServer::setupCamera() {
	AdbBridge::start();
	AdbBridge::reversePortForward(port,port); // port forwarding time.
}
void VisionServer::setupServer(){
	//create socket

	if(clientfd != -1){
		close(clientfd);
		if(VisionServer::DEBUG_MODE) std::cout << "closing old client socket" << std::endl;
	}
	if(socketfd != -1){
		close(socketfd);
		if(VisionServer::DEBUG_MODE) std::cout << "closing old server socket" << std::endl;
	}
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	int optval = 1;
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	sockaddr_in serverAddr;
	//define how connection is going to work
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port); // our port
	serverAddr.sin_addr.s_addr = INADDR_ANY; // listen to all ips

	//bind to port
	int bindAttempt = bind(socketfd,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr));
	if(bindAttempt >= 0){
		int flags;

		/* Set socket to non-blocking */
		if ((flags = fcntl(socketfd, F_GETFL, 0)) >= 0)
		{
			if (fcntl(socketfd, F_SETFL, flags | O_NONBLOCK) >= 0)
			{
				listen(socketfd,0); // listen for connections
				setupCamera();
				hasSetup=true;
			}
		}

	}else{
		if(VisionServer::DEBUG_MODE) std::cout << "error binding. err: "<< errno <<std::endl;
	}
}
bool VisionServer::isConnected() {
	return mIsConnected;
}

void VisionServer::reqAppRestart(){
	pendingRestart = true;
}

void VisionServer::findCamera(){

	sockaddr_in clientAddr;
	fd_set readset;
	timeval time;
	time.tv_sec=4;
	time.tv_usec=0;
	if(VisionServer::DEBUG_MODE) std::cout << "Finding Camera..." << std::endl;
	socklen_t sin_size=sizeof(struct sockaddr_in);
	clientfd=accept(socketfd,(struct sockaddr*)&clientAddr, &sin_size);
	if(VisionServer::DEBUG_MODE) std::cout << "Camera result: " << clientfd << std::endl;

	mIsConnected = clientfd > -1;

}

void VisionServer::runServerRoutine() {
	bool didHeartbeat = false;
	bool didTargets = false;
	char receivedStr[2048];
	//fcntl(socketfd, F_GETFL) & ;
	recv(clientfd,receivedStr,2048,0);

	if(VisionServer::DEBUG_MODE) {
		std::cout << "-----------------------"<<std::endl;
		std::cout << "errno: " << errno<<std::endl;
		std::cout << "-----------------------"<<std::endl;
	}
	if(VisionServer::DEBUG_MODE) std::cout << "RAW: " << receivedStr<<std::endl;
	std::vector<std::string> messages = split(std::string(receivedStr),'\n');

	for(int i = 0; i < messages.size() && (!didTargets||!didHeartbeat); i++) {
		std::string pch = messages[i];
		//pch is now message.
		//PCH is a json string.
		// We need to validate it and then handle it.
		json j;
		try {
			std::string pchparsed(pch);
			j = json::parse(pchparsed);
			std::string type = j["type"];
			std::string message = j["message"];
			if(type == "targets" && !didTargets){
				didTargets = true;
				targets.clear(); // wipe targets.
				//message is a json string.
				// "capturedAgoMs" : long
				// "targets" : array full of {"y":###,"z":###}
				json msgj = json::parse(message);
				long capAgoMS = msgj["capturedAgoMs"];
				if(capAgoMS > 0){
					json jtargets = msgj["targets"];
					for (auto& element : jtargets) {

					  //target is element
						TargetInfo target;
						target.y = element["y"];
						target.z = element["z"];
						if(VisionServer::DEBUG_MODE) std::cout << "target: " << target.y << "," << target.z << std::endl;
						targets.push_back(target);
					}
				}
			}else if(type == "heartbeat" && !didHeartbeat){
				didHeartbeat = true;
				// NYI: respond to heartbeat
				if(VisionServer::DEBUG_MODE) std::cout<<"heartbeat"<<std::endl;
				char* heartbeatmsg = "{\"type\":\"heartbeat\",\"message\":\"{}\"}";
				int writeStatus = send(clientfd,heartbeatmsg,strlen(heartbeatmsg),0);
				if(writeStatus == -1){
					if(VisionServer::DEBUG_MODE) std::cout << "heartbeat errno: " << errno << std::endl;
				}
			}
		}catch(std::exception e){
			// failed, invalid json??
			if(VisionServer::DEBUG_MODE) std::cout<<"server routine json error: " << e.what()<< std::endl;
		}

	}
	mIsConnected=false;
	close(clientfd);
	clientfd = -1;
}
