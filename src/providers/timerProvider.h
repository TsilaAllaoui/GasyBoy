#ifndef __TIMER_PROVIDER_H__
#define __TIMER_PROVIDER_H__

#include <memory>
#include "timer.h"

namespace gasyboy
{
    namespace provider
    {
        class TimerProvider
        {
        private:
            static std::unique_ptr<Timer> _timerInstance;

            TimerProvider() = default;

        public:
            ~TimerProvider() = default;

            TimerProvider(const TimerProvider &) = delete;
            TimerProvider &operator=(const TimerProvider &) = delete;

            static Timer &getInstance();

            static void deleteInstance();
        };
    }
}

#endif