#include <time.h>
#include "zigbee_timer.h"
#include <string>
#include <map>

using namespace std;

Zigbee_TimerMgr* Zigbee_TimerMgr::instance = nullptr;// = new TimerMgr();
Zigbee_TimerMgr* Zigbee_TimerMgr::get_instance() {
	if (instance == nullptr) {
		instance = new Zigbee_TimerMgr();
	}
	return instance;
}


void zigbee_thread_timer() {
	std::chrono::milliseconds duration(1000);
	while(1){
		std::this_thread::sleep_for(duration);
		//Zigbee_TIMER_MGR->heartbeat();
		Zigbee_TimerMgr::get_instance()->heartbeat();
	}
}