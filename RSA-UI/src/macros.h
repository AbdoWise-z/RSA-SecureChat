#pragma once

#define to_str(x) #x
#define pause system("pause>nul");
#define ll long long
#define ull unsigned long long
#define ld long double
#define INIT_TIMER std::chrono::steady_clock::time_point timer_variable_start_time1;std::chrono::steady_clock::time_point timer_variable_start_time2; std::chrono::duration<ll , std::nano> timer_variable_duration
#define TIMER_GET_DELTA timer_variable_duration
#define TIMER_GET_DELTA_MS std::chrono::duration_cast<std::chrono::milliseconds>(timer_variable_duration).count()
#define TIMER(x) timer_variable_start_time1 = std::chrono::steady_clock::now(); x;timer_variable_start_time2 = std::chrono::steady_clock::now(); timer_variable_duration = timer_variable_start_time2 - timer_variable_start_time1
#define TIMER_LOG(x) timer_variable_start_time1 = std::chrono::steady_clock::now(); x;timer_variable_start_time2 = std::chrono::steady_clock::now(); timer_variable_duration = timer_variable_start_time2 - timer_variable_start_time1;std::cout << to_str(x) << " took " << TIMER_GET_DELTA_MS << " ms" << std::endl;
#define TIMER_START timer_variable_start_time1 = std::chrono::steady_clock::now()
#define TIMER_STOP timer_variable_start_time2 = std::chrono::steady_clock::now(); timer_variable_duration = timer_variable_start_time2 - timer_variable_start_time1
