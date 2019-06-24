#include <QtTest>

#include "event.h"

using namespace Base::Event;

class EventTest : public QObject
{
    Q_OBJECT
  private slots:
    void connect_and_fire();
    void connect_and_fire_with_lambda();
};

class MyEventHandler : public EventHandler<MyEventHandler> {
  public:
    QString receivedEventArg;
    static int eventsReceived;

    MyEventHandler(const QString& name) : _name(name) {}

    void handleEvent(const QString& arg) {
        qDebug() << _name << " received event: " << arg;
        receivedEventArg = arg;
        eventsReceived++;
    }

  private:
    QString _name;
};

// Initialize static member
int MyEventHandler::eventsReceived = 0;

void EventTest::connect_and_fire() {
    MyEventHandler::eventsReceived = 0;

    Event<const QString&> myEvent;

    MyEventHandler myHandler("myHandler");
    myHandler.connect(myEvent, &MyEventHandler::handleEvent);

    {
        MyEventHandler myHandler2("myHandler2");
        myHandler2.connect(myEvent, &MyEventHandler::handleEvent);

        myEvent.fire("hello world");
        QVERIFY(myHandler.receivedEventArg == "hello world");
        QVERIFY(myHandler2.receivedEventArg == "hello world");
        QVERIFY(MyEventHandler::eventsReceived == 2);
    }

    myEvent.fire("hello again");
    QVERIFY(MyEventHandler::eventsReceived == 3); // myHandler2 is out of scope
}

void EventTest::connect_and_fire_with_lambda() {
    int eventsReceived = 0;
    Event<const QString&> myEvent;

    SingleEventHandler<const QString&> myHandler(
        [&eventsReceived](const QString& str) { eventsReceived++; });
    myHandler.connect(myEvent);

    {
        SingleEventHandler<const QString&> myHandler2(
            [&eventsReceived](const QString& str) { eventsReceived++; });
        myHandler2.connect(myEvent);

        myEvent.fire("hello world");
        QVERIFY(eventsReceived == 2);
    }
    myEvent.fire("hello world");
    QVERIFY(eventsReceived == 3);
}

QTEST_APPLESS_MAIN(EventTest)

#include "tst_eventtest.moc"
