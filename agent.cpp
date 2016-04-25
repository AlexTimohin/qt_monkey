#include "agent.hpp"

#include <cassert>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <atomic>
#include <functional>

#include "agent_qtmonkey_communication.hpp"
#include "common.hpp"
#include "user_events_analyzer.hpp"

using namespace qt_monkey;
using namespace qt_monkey::Private;

namespace
{
class FuncEvent final : public QEvent
{
public:
    FuncEvent(QEvent::Type type, std::function<void()> func)
        : QEvent(type), func_(std::move(func))
    {
    }
    void exec() { func_(); }
private:
    std::function<void()> func_;
};

class EventsReciever final : public QObject
{
public:
    EventsReciever()
    {
        eventType_ = static_cast<QEvent::Type>(QEvent::registerEventType());
    }
    bool event(QEvent *event) override
    {
        if (event->type() != eventType_)
            return false;

        static_cast<FuncEvent *>(event)->exec();

        return true;
    }
    QEvent::Type eventType() const { return eventType_; }
private:
    std::atomic<QEvent::Type> eventType_;
};

class AgentThread final : public QThread
{
public:
    AgentThread(QObject *parent) : QThread(parent) {}
    void waitReady()
    {
        while (!ready_.exchange(false))
            ;
    }
    void run() override
    {
        CommunicationAgentPart client;
        if (!client.connectToMonkey()) {
            qFatal(
                "%s",
                qPrintable(
                    T_("%1: can not connect to qt monkey").arg(Q_FUNC_INFO)));
            std::abort();
        }
        connect(&client, SIGNAL(error(const QString &)), parent(),
                SLOT(onCommunicationError(const QString &)),
                Qt::QueuedConnection);
        EventsReciever eventReciever;
        objInThread_ = &eventReciever;
        channelWithMonkey_ = &client;
        ready_ = true;
        exec();
    }

    CommunicationAgentPart *channelWithMonkey() { return channelWithMonkey_; }

    void runInThread(std::function<void()> func) {
        assert(objInThread_ != nullptr);
        QCoreApplication::postEvent(objInThread_, new FuncEvent(objInThread_->eventType(), std::move(func)));
    }
private:
    std::atomic<bool> ready_{false};
    EventsReciever *objInThread_{nullptr};
    CommunicationAgentPart *channelWithMonkey_{nullptr};
};
}

Agent::Agent(std::list<CustomEventAnalyzer> customEventAnalyzers)
    : eventAnalyzer_(
          new UserEventsAnalyzer(std::move(customEventAnalyzers), this))
{
    connect(eventAnalyzer_, SIGNAL(userEventInScriptForm(const QString &)),
            this, SLOT(onUserEventInScriptForm(const QString &)));
    QCoreApplication::instance()->installEventFilter(eventAnalyzer_);
    thread_ = new AgentThread(this);
    thread_->start();
    static_cast<AgentThread *>(thread_)->waitReady();
}

void Agent::onCommunicationError(const QString &err)
{
    qFatal("%s: communication error %s", Q_FUNC_INFO, qPrintable(err));
    std::abort();
}

Agent::~Agent() {}

void Agent::onUserEventInScriptForm(const QString &script)
{
    qDebug("%s: script '%s'\n", Q_FUNC_INFO, qPrintable(script));
    auto thread = static_cast<AgentThread *>(thread_);
    thread->runInThread([thread, script] { thread->channelWithMonkey()->sendCommand(PacketTypeForMonkey::NewUserAppEvent, script);});
}