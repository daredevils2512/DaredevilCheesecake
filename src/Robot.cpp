#include "Robot.h"

long Robot::lastHit = 0;
std::shared_ptr<VisionServer> Robot::vs;
std::unique_ptr<frc::Command> autonomousCommand;
frc::SendableChooser<frc::Command*> chooser;
void Robot::RobotInit() {
	chooser.AddDefault("Default Auto", new ExampleCommand());
	// chooser.AddObject("My Auto", new MyAutoCommand());
	frc::SmartDashboard::PutData("Auto Modes", &chooser);
	vs.reset(new VisionServer());
	std::cout<<"Setting up VisionServer"<<std::endl;
	Robot::vs->setupServer();
	std::thread(visionLoop).detach();
}

/**
 * This function is called once each time the robot enters Disabled mode.
 * You can use it to reset any subsystem information you want to clear when
 * the robot is disabled.
 */
void Robot::visionLoop() {
	while(true){
		if(vs->hasSetup){
			visionUpdater();
		}
	}
}
void Robot::visionUpdater() {
	if(!vs->isConnected()){
		vs->findCamera();
	}
	if(vs->isConnected()){
		if(VisionServer::DEBUG_MODE) std::cout<< "connected..." <<std::endl;
		std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
						std::chrono::system_clock::now().time_since_epoch()
				);
		vs->runServerRoutine();
		long timeTaken = std::chrono::duration_cast< std::chrono::milliseconds >(
						std::chrono::system_clock::now().time_since_epoch()
				).count() - ms.count();
				frc::SmartDashboard::PutNumber("debug timeTaken (ms)",timeTaken);
	}/*else{
		if(VisionServer::DEBUG_MODE) std::cout<< "camera is not responding."<<std::endl;
		std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
								std::chrono::system_clock::now().time_since_epoch()
						);

		if(!AdbBridge::runCommand("shell exit")){
			vs->setupCamera();
		}
		long timeTaken = std::chrono::duration_cast< std::chrono::milliseconds >(
										std::chrono::system_clock::now().time_since_epoch()
								).count() - ms.count();
								frc::SmartDashboard::PutNumber("debug shell exit (ms)",timeTaken);
	}*/
}

void Robot::DisabledInit() {

	Robot::vs->isActive = false;


}

void Robot::DisabledPeriodic()  {
	frc::Scheduler::GetInstance()->Run();

}

/**
 * This autonomous (along with the chooser code above) shows how to select
 * between different autonomous modes using the dashboard. The sendable
 * chooser code works with the Java SmartDashboard. If you prefer the
 * LabVIEW Dashboard, remove all of the chooser code and uncomment the
 * GetString code to get the auto name from the text box below the Gyro.
 *
 * You can add additional auto modes by adding additional commands to the
 * chooser code above (like the commented example) or additional comparisons
 * to the if-else structure below with additional strings & commands.
 */
void Robot::AutonomousInit()  {
	/* std::string autoSelected = frc::SmartDashboard::GetString("Auto Selector", "Default");
	if (autoSelected == "My Auto") {
		autonomousCommand.reset(new MyAutoCommand());
	}
	else {
		autonomousCommand.reset(new ExampleCommand());
	} */
	Robot::vs->isActive = true;
	autonomousCommand.reset(chooser.GetSelected());

	if (autonomousCommand.get() != nullptr) {
		autonomousCommand->Start();
	}
}

void Robot::AutonomousPeriodic()  {
	frc::Scheduler::GetInstance()->Run();
}

void Robot::TeleopInit()  {
	// This makes sure that the autonomous stops running when
	// teleop starts running. If you want the autonomous to
	// continue until interrupted by another command, remove
	// this line or comment it out.
	Robot::vs->isActive = true;
	if (autonomousCommand != nullptr) {
		autonomousCommand->Cancel();
	}

}

void Robot::TeleopPeriodic()  {


	frc::Scheduler::GetInstance()->Run();
	frc::SmartDashboard::PutNumber("Targets",vs->targets.size());
	frc::SmartDashboard::PutBoolean("setupSucceeded",vs->hasSetup);
	frc::SmartDashboard::PutBoolean("isActive",vs->isActive);
	frc::SmartDashboard::PutBoolean("isConnected",vs->isConnected());

}

void Robot::TestPeriodic()  {
	frc::LiveWindow::GetInstance()->Run();
}





START_ROBOT_CLASS(Robot)
