#include "timerProvider.h"

namespace gasyboy
{
    namespace provider
    {
        std::shared_ptr<Timer> TimerProvider::_timerInstance = nullptr;

        std::shared_ptr<Timer> TimerProvider::getInstance()
        {
            if (!_timerInstance)
            {
                _timerInstance = std::make_shared<Timer>();
            }
            return _timerInstance;
        }

        void TimerProvider::deleteInstance()
        {
            _timerInstance.reset();
        }
    }
}