#ifndef EVENT_H
#define EVENT_H

#include <algorithm>
#include <memory>
#include <vector>

using namespace std;

#include "common.h"

// Based on code example found here:
// https://stackoverflow.com/questions/35847756/robust-c-event-pattern

namespace Base::Event {

// Base class for Subscriber. Clients should not use this class directly.
template <class... EventArgs> class SubscriberBase : private Base::NonCopyable {
  public:
    // Invokes the event handler method with the given arguments.
    virtual void invoke(EventArgs... args) const = 0;

    // Checks if this Subscriber represents the given event handler.
    virtual bool representsEventHandler(void* eventHandler) const = 0;
};

// Class used internally by Event to represent a subscriber.
// Clients should not use this class directly.
template <class TEventHandler, class... EventArgs>
class Subscriber : public SubscriberBase<EventArgs...> {
  public:
    // Creates a new Subscriber for the given event handler and event handler
    // method.
    explicit Subscriber(TEventHandler* eventHandler,
                        void (TEventHandler::*eventHandlerMethod)(EventArgs...))
        : _eventHandler(eventHandler), _handlerMethod(eventHandlerMethod) {}

    void invoke(EventArgs... args) const final {
        (_eventHandler->*_handlerMethod)(args...);
    }

    bool representsEventHandler(void* eventHandler) const final {
        return _eventHandler == eventHandler;
    }

  private:
    TEventHandler* _eventHandler;
    void (TEventHandler::*_handlerMethod)(EventArgs...);
};

// Base class for Event.
class EventBase : private Base::NonCopyable {
  public:
    // Unsubsribes the given event handler from this event. Clients should not
    // use this method directly.
    virtual void unsubscribe(void* eventHandler) = 0;
};

// Event class that is used to fire event notifications to subscribers. An event
// notification consists of one or more event argument, defined as template
// parameters.
template <class... EventArgs> class Event : public EventBase {
  public:
    void unsubscribe(void* eventHandler) final {
        auto toRemove = remove_if(_subscribers.begin(), _subscribers.end(),
                                  [eventHandler](auto& subscriber) {
                                      return subscriber->representsEventHandler(
                                          eventHandler);
                                  });
        _subscribers.erase(toRemove, _subscribers.end());
    }

    template <class TEventHandler>
    void subscribe(TEventHandler* eventHandler,
                   void (TEventHandler::*handlerMethod)(EventArgs... args)) {
        auto subscriber = new Subscriber<TEventHandler, EventArgs...>(
            eventHandler, handlerMethod);
        _subscribers.push_back(SmartBasePointer(subscriber));
    }

    void fire(EventArgs... args) const {
        for (auto& subscriber : _subscribers) {
            subscriber->invoke(args...);
        }
    }

  private:
    using SmartBasePointer = unique_ptr<SubscriberBase<EventArgs...>>;
    vector<SmartBasePointer> _subscribers;
};

// Base class for event handlers that receive and handle event notifications.
// This event handler will automatically unsubscribe from the event upon
// destruction.
template <class Derived> class EventHandler : private Base::NonCopyable {
  public:
    template <class... EventArgs>
    void connect(Event<EventArgs...>& event,
                 void (Derived::*handlerMethod)(EventArgs... args)) {
        event.subscribe(dynamic_cast<Derived*>(this), handlerMethod);
        _connectedEvents.push_back(&event);
    }

    ~EventHandler() {
        for (auto& event : _connectedEvents) {
            event->unsubscribe(this);
        }
    }

  private:
    std::vector<EventBase*> _connectedEvents;
};

}; // namespace Base::Event

#endif // EVENT_H
