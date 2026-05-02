#ifndef QTMARKDOWN_CORE_TIMER_H
#define QTMARKDOWN_CORE_TIMER_H

#include <functional>

namespace md::editor::core {

class ITimer {
public:
    virtual ~ITimer() = default;
    virtual void start(int intervalMs) = 0;
    virtual void stop() = 0;
    virtual void setCallback(std::function<void()> callback) = 0;
    virtual bool isRunning() const = 0;
};

} // namespace md::editor::core
#endif // QTMARKDOWN_CORE_TIMER_H
