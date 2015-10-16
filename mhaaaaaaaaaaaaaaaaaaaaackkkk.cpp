#include <SFML/Graphics.hpp>

// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>

#include <math.h>

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

#include <windows.h>

sf::CircleShape shape(150.f);
bool color_input;

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : onArm(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose()
    {
    }

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        roll_w = 0;
        pitch_w = 0;
        yaw_w = 0;
        onArm = false;
    }

    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
    // as a unit quaternion.
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        using std::atan2;
        using std::asin;
        using std::sqrt;

        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                           1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        float pitch = asin(2.0f * (quat.w() * quat.y() - quat.z() * quat.x()));
        float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                        1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
		
        // Convert the floating point angles in radians to a scale from 0 to 20.
        roll_w = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
        pitch_w = static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 18);
        yaw_w = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);
		float drop = 1.1;
		roll = ((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
        pitch = ((pitch + (float)M_PI/2.0f)/M_PI * 18);
		if(pitch > 10)
		{	if(pitch < 20)
			{	pitch *= (drop*(20-pitch));
			}
		}
		else
		{	if(pitch != 10)
			{	pitch *= (drop*(10-pitch));
			}
		}	
        yaw = ((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);
		if(yaw > 10.0000)
		{	if(yaw < 20.0000)
			{	yaw *= (drop*(20-yaw));
			}
			else
			{	yaw = 19.9999;
			}
		}
		else
		{	if(yaw != 10.0000)
			{	yaw *= (drop*(10-yaw));
			}
		}	
		if(color_input == true)
		{	float x = (GetSystemMetrics(SM_CXSCREEN));
			float y = (GetSystemMetrics(SM_CYSCREEN));
			SetCursorPos((((((20)-yaw))/(20))*x), ((pitch/(20))*y));
		}
    }

    // onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
    // making a fist, or not making a fist anymore.
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        currentPose = pose;

        // Vibrate the Myo whenever we've detected that the user has made a fist.
        if (pose == myo::Pose::waveIn) {
            myo->vibrate(myo::Myo::vibrationMedium);
			std::cout << "RGB Colour (" << (float)shape.getFillColor().r << " , " << (float)shape.getFillColor().g << " , " << (float)shape.getFillColor().b << ")" << std::endl;
			if(color_input == true)
			{	color_input = false; 
			}
			else
			{	color_input = true;
			}
		}
		else if(pose == myo::Pose::waveOut)
		{	//myo->vibrate(myo::Myo::vibrationMedium);		// this proved to be a bit of a distraction
			INPUT ip;
			ip.type = INPUT_MOUSE;
			ip.mi.dx = 0; 
			ip.mi.dy = 0;
			ip.mi.time = 0.2;
			ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
			SendInput(1, &ip, sizeof(INPUT));
			ip.mi.dwFlags = MOUSEEVENTF_RIGHTUP; // KEYEVENTF_KEYUP for key release
			SendInput(1, &ip, sizeof(INPUT));
		}
		else if((pose == myo::Pose::fist)||(pose == myo::Pose::thumbToPinky))
		{	//myo->vibrate(myo::Myo::vibrationMedium);
			INPUT ip;
			ip.type = INPUT_MOUSE;
			ip.mi.dx = 0; 
			ip.mi.dy = 0;
			ip.mi.time = 0.2;
			ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &ip, sizeof(INPUT));
			ip.mi.dwFlags = MOUSEEVENTF_LEFTUP; // KEYEVENTF_KEYUP for key release
			SendInput(1, &ip, sizeof(INPUT));

		}	
    }

    // onArmRecognized() is called whenever Myo has recognized a setup gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmRecognized(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
    {
        onArm = true;
        whichArm = arm;
    }

    // onArmLost() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmLost(myo::Myo* myo, uint64_t timestamp)
    {
        onArm = false;
    }

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';

        // Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
        std::cout << '[' << std::string(roll_w, '*') << std::string(18 - roll_w, ' ') << ']'
                  << '[' << std::string(pitch_w, '*') << std::string(18 - pitch_w, ' ') << ']'
                  << '[' << std::string(yaw_w, '*') << std::string(18 - yaw_w, ' ') << ']'<< std::endl;
		if(color_input == true)
		{	shape.setFillColor(sf::Color((roll_w*10.75), ((270+pitch_w)*10.75), ((90 +yaw_w)*10.75)));
		}	
		
        if (onArm) {
            // Print out the currently recognized pose and which arm Myo is being worn on.

            // Pose::toString() provides the human-readable name of a pose. We can also output a Pose directly to an
            // output stream (e.g. std::cout << currentPose;). In this case we want to get the pose name's length so
            // that we can fill the rest of the field with spaces below, so we obtain it as a string using toString().
            std::string poseString = currentPose.toString();

            std::cout << '[' << (whichArm == myo::armLeft ? "L" : "R") << ']'
                      << '[' << poseString << std::string(14 - poseString.size(), ' ') << ']';
        } else {
            // Print out a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
            std::cout << "[?]" << '[' << std::string(14, ' ') << ']';
        }

        std::cout << std::flush;
    }

    // These values are set by onArmRecognized() and onArmLost() above.
    bool onArm;
    myo::Arm whichArm;

    // These values are set by onOrientationData() and onPose() above.
    int roll_w, pitch_w, yaw_w;
    myo::Pose currentPose;
};

int main(int argc, char** argv)
{   sf::RenderWindow window(sf::VideoMode(400, 400), "Myo color picker");
	shape.setPosition(sf::Vector2f(50, 50));    
	shape.setFillColor(sf::Color::Green);
	color_input = true;
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {

    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    myo::Hub hub("com.example.hello-myo");

    std::cout << "Attempting to find a Myo..." << std::endl;

    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    // waitForAnyMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
    // if that fails, the function will return a null pointer.
    myo::Myo* myo = hub.waitForMyo(10000);

    // If waitForAnyMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!myo) {
        throw std::runtime_error("Unable to find a Myo!");
    }

    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

    // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
    DataCollector collector;

    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub.addListener(&collector);



    // Finally we enter our main loop.
    while (1) {
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
        hub.run(1000/20);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
        collector.print();
		if(window.isOpen())
		{	        sf::Event event;
			 while (window.pollEvent(event))
			{
			    if (event.type == sf::Event::Closed)
			   {    window.close();
					 return 0;
				}
			
			}

        window.clear();
        window.draw(shape);
        window.display();
		}
    }

    // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}
