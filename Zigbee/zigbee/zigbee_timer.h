#ifndef _ZIGBEE_TIMER_H_
#define _ZIGBEE_TIMER_H_

#include <map>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <list>
#include <iostream>
#include <functional>

using namespace std;


static std::mutex zigbee_g_timer_lock;


typedef std::function<void(uint32_t)> fun_action_t;

class Zigbee_TimerMgr {
public:
    ~Zigbee_TimerMgr()
    {
        //zigbee_log_debug("~TimerMgr()");
    };

    //max_id 
    uint32_t /*timerId*/ add_timer(bool auto_reset, uint32_t interval, uint32_t repeat_count, fun_action_t callback)
    {
		//printf("add_timer, auto_reset=[%d], interval=[%d], repeat_count=[%d] \n", auto_reset, interval, repeat_count);
		zigbee_g_timer_lock.lock();
        uint32_t max_id = 0;
		
        // find maximum existing timer id and assign it to max_id
		map<uint32_t /*timer_id*/, TimerItem*>::iterator it;
		for (it = _timer_items.begin(); it != _timer_items.end(); it++) {
			if (it->first > max_id) {
				//zigbee_log_debug("it->first=%d", it->first);
				max_id = it->first;
				//zigbee_log_debug("max_id=%d", max_id);
			}
		}
       
        max_id++;
		//zigbee_log_debug("here max_id=%d", max_id);
        while ((_timer_items.find(max_id) != _timer_items.end()) || max_id == 0) {
            max_id++;
        }

		printf("create timer, max_id=[%d] \n", max_id);

        // first timer id will be 1
        _timer_items[max_id] = new TimerItem(auto_reset, interval, repeat_count, callback);

		zigbee_g_timer_lock.unlock();
        return max_id;
    }

    bool remove(uint32_t timer_id)
    {
		printf("remove, timer_id=[%d] \n", timer_id);
		zigbee_g_timer_lock.lock();
        map<uint32_t, TimerItem*>::iterator it = _timer_items.find(timer_id);
        if (it != _timer_items.end()) {
            delete it->second;
            _timer_items.erase(timer_id);
        }
		zigbee_g_timer_lock.unlock();
        return true;
    }

    bool reset_timer(uint32_t timer_id)
    {
		printf("reset_timer, timer_id=[%d] \n", timer_id);
		zigbee_g_timer_lock.lock();
        bool result = false;
        map<uint32_t, TimerItem*>::iterator it = _timer_items.find(timer_id);
        if (it != _timer_items.end()) {
            it->second->reset();
            result = true;
        }
		zigbee_g_timer_lock.unlock();
        return result;
    }
    bool reset_timer(uint32_t timer_id, uint32_t repeat_count)
    {
		printf("reset_timer, timer_id=[%d]  count=[%d] \n", timer_id, repeat_count);
		zigbee_g_timer_lock.lock();
        bool result = false;
        map<uint32_t, TimerItem*>::iterator it = _timer_items.find(timer_id);
        if (it != _timer_items.end()) {
            it->second->reset(repeat_count);
            result = true;
        }
		zigbee_g_timer_lock.unlock();
        return result;
    }
    bool exist_timer(uint32_t timer_id)
    {
        //zigbee_log_debug("exist_timer, timer_id=[%d]", timer_id);
		zigbee_g_timer_lock.lock();
        bool exist = false;
        exist = _timer_items.find(timer_id) != _timer_items.end();
		zigbee_g_timer_lock.unlock();
        return exist;
    }

    bool edit(uint32_t timer_id, bool auto_reset, uint32_t interval, uint32_t repeat_count, void (*callback)(uint32_t timer_id))
    {
		//printf("edit, auto_reset=[%d], interval=[%d], repeat_count=[%d], timer_id=[%d] \n", auto_reset, interval, repeat_count, timer_id);
		zigbee_g_timer_lock.lock();
        bool result = false;
        map<uint32_t, TimerItem*>::iterator it = _timer_items.find(timer_id);
        if (it != _timer_items.end()) {
            it->second->edit(auto_reset, interval, repeat_count, callback);
            result = true;
        }
		zigbee_g_timer_lock.unlock();
        return result;
    }

private:
    class TimerItem {
        //void *_args;
    public:
        //int(*_callback)(uint32_t timer_id);
        fun_action_t _callback;

        bool _auto_reset = false;
        uint32_t _repeat_count;

        uint32_t _interval;
        time_t _start_time;
        bool _is_running;

        TimerItem()
        {
            //zigbee_log_debug("TimerItem()");
            run_count = 0;
        }
        TimerItem(bool auto_reset, uint32_t interval, unsigned int repeat_count, fun_action_t callback)
        {
			//printf("TimerItem, auto_reset=[%d], interval=[%d], repeat_count=[%d] \n", auto_reset, interval, repeat_count);
            _auto_reset = auto_reset;
            _repeat_count = repeat_count;
            _interval = interval;
            _callback = callback;
            _start_time = Zigbee_TimerMgr::get_instance()->snap();
            _is_running = true;
            run_count = 0;
        }
        ~TimerItem()
        {
			//printf("~TimerItem() \n");
        }

        void elapsed(time_t total_running_time, uint32_t timer_id)
        {
			//printf("elapsed, total_running_time=[%d], timer_id=[%d], _auto_reset=[%d], _repeat_count=[%d], run_count=[%d] \n",
            //                 total_running_time, timer_id, _auto_reset, _repeat_count, run_count);

            if (!_is_running) {
                //zigbee_log_debug("timer not running");
                return;
            }

            run_count++;
            if (_auto_reset || (_repeat_count >= run_count)) {
                if (_callback != nullptr) {
                    _callback(timer_id);
                }
                _start_time = total_running_time;
            }
            if (_auto_reset) {
                _start_time = total_running_time;
            }

            if (!_auto_reset && run_count >= _repeat_count) {
                _is_running = false;
            }
        }

        bool is_running()
        {
			//printf("is_running() \n");
            return _is_running;
        }

        void edit(bool auto_reset, uint32_t interval, unsigned int repeat_count, void (*callback)(uint32_t timer_id))
        {
			//printf("TimerItem edit, auto_reset=[%d], interval=[%d], repeat_count=[%d] \n", auto_reset, interval, repeat_count);
            _auto_reset = auto_reset;
            _repeat_count = repeat_count;
            _interval = interval;
            _callback = callback;
            _start_time = Zigbee_TimerMgr::get_instance()->snap();
            _is_running = true;
            run_count = 0;
        }

        void reset()
        {
            //zigbee_log_debug("TimerItem reset()");
            _start_time = Zigbee_TimerMgr::get_instance()->snap();
            run_count = 0;
            _is_running = true;
        }
        //=== max add this 2016.8.23
        void reset(int count)
        {
            //zigbee_log_debug("TimerItem reset()");
            _start_time = Zigbee_TimerMgr::get_instance()->snap();
            run_count = (uint16_t)count;
            _is_running = true;
        }

    private:
        uint16_t run_count;
    };

    map<uint32_t /*timer_id*/, TimerItem*> _timer_items;

    time_t _start_time_1900;    
    time_t _cloud_sync_time;    
    time_t _total_running_time; 
    bool _is_synced_once = false;
    time_t _run_time_before_sync = 0;

public:
    static Zigbee_TimerMgr* get_instance();

    int count = 0;
    void heartbeat()
    {
		zigbee_g_timer_lock.lock();

        time_t now = get_current_system_time();
        count++;
        //cout << "heartheat called:"<<count << endl;

        // todo, get_current_year returns 2016, already?
        if (!_is_synced_once) {
            struct tm* ptm = localtime(&now);
            int year = (int)ptm->tm_year + 1900;
            //zigbee_log_notice("heartbeat ingg year is %d", year);
            if (year >= 2016) {
                //zigbee_log_notice("heartbeat ingg TimerMgr 11111");
                _is_synced_once = true;
                _cloud_sync_time = now;
            } else {
                //zigbee_log_notice("heartbeat ingg TimerMgr 11112");
                _run_time_before_sync = now - _start_time_1900;
                _total_running_time = _run_time_before_sync;
            }
        } else {
            //	            zigbee_log_notice("heartbeat ingg TimerMgr 11113");
            _total_running_time = now - _cloud_sync_time + _run_time_before_sync;
        }

        list<uint32_t /*timerId*/> timers_to_remove;
        for(auto& it:_timer_items)
        {
            if (is_triggered(it.second->_start_time, it.second->_interval)) {
				printf("heartbeat ingg TimerMgr 11114 \n");
                it.second->elapsed(_total_running_time, it.first);
                if (it.second->is_running() == false) {
					printf("heartbeat ingg TimerMgr 11115 \n");
                    timers_to_remove.push_back(it.first);
                }
            }
        }

        for (auto id : timers_to_remove) {
			printf("heartbeat ingg TimerMgr 11116 \n");

            auto it = _timer_items.find(id);
            if (it != _timer_items.end()) {
                printf("heartbeat ingg TimerMgr 11117");
                delete it->second;
            }
			printf("======_timer_items.erase(%d)==\n", id);
            _timer_items.erase(id);
        }
        //-----------------------------------------

		zigbee_g_timer_lock.unlock();
    }

    time_t snap()
    {
		//printf("snap() \n");
        return _total_running_time;
    }

    bool is_triggered(time_t start_time_snap, time_t delay_time)
    {
        bool b = (_total_running_time - start_time_snap) >= delay_time;
		//printf("is_triggered(time_t start_time_snap, time_t delay_time), b=[%d] \n", b);
        return b;
    }
    const time_t get_current_system_time()
    {
		auto now = std::chrono::system_clock::now();
        time_t t = std::chrono::system_clock::to_time_t(now);
        //	         zigbee_log_debug("get_current_system_time()=[%llu]", t);
        return t;
    }
    const int get_current_year()
    {
        auto now = get_current_system_time();
        struct tm* ptm = localtime(&now);
        int year = (int)ptm->tm_year + 1900;
        //zigbee_log_debug("get_current_year() year=%d", year);
        return year;
    }

    // hours since midnight, hours since midnight
    const int get_today_total_minutes()
    {
        auto now = get_current_system_time();
        struct tm* ptm = localtime(&now);
        uint32_t m = ptm->tm_hour * 60 + ptm->tm_min;
        //zigbee_log_debug("get_today_total_minutes() m is %d", m);
        return m;
    }

    // days since Sunday	0-6
    const int get_day_of_week()
    {

        auto now = get_current_system_time();
        struct tm* ptm = localtime(&now);
        int d = ptm->tm_wday;
        //zigbee_log_debug("get_day_of_week() day of week is %d", d);
        return d;
    }

private:
    Zigbee_TimerMgr()
    {
        //zigbee_log_debug("TimerMgr()");
        //zigbee_log_debug("get_day_of_week() is %d", get_day_of_week());
        //zigbee_log_debug("get_current_year() is %d", get_current_year());
        time_t now = get_current_system_time();
        struct tm* ptm = localtime(&now);
        //zigbee_log_debug("TimerMgr() ptm->tm_year=%d", ptm->tm_year);
        //todo,116+1900=2016, get_current_year returns 2016,
        if ((ptm->tm_year + 1900) < 2016) {
            _start_time_1900 = now;
        } else {
            _cloud_sync_time = now;
        }
    };

	Zigbee_TimerMgr(const Zigbee_TimerMgr&)
    {
        //zigbee_log_debug("TimerMgr()");
    };

    static Zigbee_TimerMgr* instance;

    bool _timer_editing;
    std::thread* _timer_thread;

    //-----------
    using clock = std::chrono::high_resolution_clock;
    using point = std::chrono::time_point<clock>;
    template <typename count_t>
    static count_t get_diff(const point& start, const point& end)
    {
        //zigbee_log_debug("get_diff(const point& start, const point& end)");
        using duration_t = std::chrono::duration<count_t, std::milli>;
        return std::chrono::duration_cast<duration_t>(end - start).count();
    }
};

#define Zigbee_TIMER_MGR Zigbee_TimerMgr::get_instance()


void zigbee_thread_timer();


#endif
