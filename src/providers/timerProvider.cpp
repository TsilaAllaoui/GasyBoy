#include "timerProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::unique_ptr<Timer> TimerProvider::_timerInstance = nullptr;

        Timer &TimerProvider::getInstance()
        {
            if (!_timerInstance)
            {
                _timerInstance = std::make_unique<Timer>();
            }
            return *_timerInstance;
        }

        void TimerProvider::deleteInstance()
        {
            _timerInstance.reset(nullptr);
        }
    }
}