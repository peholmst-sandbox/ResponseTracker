#ifndef EVENT_H
#define EVENT_H

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

using namespace std;

#include "common.h"

// Based on code example found here:
// https://stackoverflow.com/questions/35847756/robust-c-event-pattern

namespace Base::Event {

/**
 * @brief Base class for Subscriber. Clients should not need to use this class
 * directly.
 *
 * @tparam EventArgs the types of the event arguments.
 */
template <class... EventArgs> class SubscriberBase : private Base::NonCopyable {
  public:
    /**
     * @brief Invokes the event handler with the given arguments.
     *
     * @param args the event arguments.
     */
    virtual void invoke(EventArgs... args) const = 0;

    /**
     * @brief Checks if this Subscriber represents the given event handler.
     *
     * @param eventHandler the event handler to check.
     * @returns true if this subscriber represents the given event handler,
     * false otherwise.
     */
    virtual bool representsEventHandler(void* eventHandler) const = 0;
};

/**
 * @brief Class used internally by Event to represent a subscriber. Clients
 * should not need to use this class directly.
 *
 * @tparam TEventHandler the type of the event handler that this subscriber
 * represents.
 * @tparam EventArgs the types of the event arguments.
 */
template <class TEventHandler, class... EventArgs>
class Subscriber : public SubscriberBase<EventArgs...> {
  public:
    // Creates a new Subscriber for the given event handler and event handler
    // method.
    explicit Subscriber(TEventHandler* eventHandler,
                        void (TEventHandler::*eventHandlerMethod)(EventArgs...))
        : _eventHandler(eventHandler), _handlerMethod(eventHandlerMethod) {}

    void invoke(EventArgs... args) const override final {
        (_eventHandler->*_handlerMethod)(args...);
    }

    bool representsEventHandler(void* eventHandler) const override final {
        return _eventHandler == eventHandler;
    }

  private:
    TEventHandler* _eventHandler;
    void (TEventHandler::*_handlerMethod)(EventArgs...);
};

/**
 * @brief Base class for Event. Clients should not need to use this class
 * directly but interact with Event instead.
 */
class EventBase : private Base::NonCopyable {
  public:
    /**
     * @brief Unsubscribes the given event handler from this event. Clients do
     * not need to call this method if they use the EventHandler class.
     *
     * @param eventHandler the event handler to unsubscribe.
     */
    virtual void unsubscribe(void* eventHandler) = 0;
};

/**
 * @brief Event class that can be subscribed to and fired. When the event is
 * fired, the event arguments will be passed to all its subscribers.
 *
 * @tparam EventArgs the types of the event arguments.
 */
template <class... EventArgs> class Event : public EventBase {
  public:
    void unsubscribe(void* eventHandler) override final {
        auto toRemove = remove_if(_subscribers.begin(), _subscribers.end(),
                                  [eventHandler](auto& subscriber) {
                                      return subscriber->representsEventHandler(
                                          eventHandler);
                                  });
        _subscribers.erase(toRemove, _subscribers.end());
    }

    /**
     * @brief Subscribes the given event handler to this event.
     *
     * @param eventHandler the event handler.
     * @param handlerMethod the method of the event handler that will be invoked
     * when the event is fired.
     */
    template <class TEventHandler>
    void subscribe(TEventHandler* eventHandler,
                   void (TEventHandler::*handlerMethod)(EventArgs... args)) {
        auto subscriber = new Subscriber<TEventHandler, EventArgs...>(
            eventHandler, handlerMethod);
        _subscribers.push_back(SmartBasePointer(subscriber));
    }

    /**
     * @brief Fires the event to all its subscribers.
     *
     * @param args the event arguments to pass to all subscribers.
     */
    void fire(EventArgs... args) const {
        for (auto& subscriber : _subscribers) {
            subscriber->invoke(args...);
        }
    }

  private:
    using SmartBasePointer = unique_ptr<SubscriberBase<EventArgs...>>;
    vector<SmartBasePointer> _subscribers;
};

/**
 * @brief Base class for event handlers that receive and handle event
 * notifications. This event handler will automatically unsubscribe from the
 * event upon destruction.
 *
 * @tparam Derived the subclass that extends this event handler and defines the
 * event handler method.
 */
template <class Derived> class EventHandler : private Base::NonCopyable {
  public:
    /**
     * @brief Connects this event handler to the given event, invoking the given
     * handler method when an event is fired.
     *
     * @param event the event to subscribe to.
     * @param handlerMethod the method of the event handler to invoke when an
     * event is fired.
     */
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

/**
 * @brief Event handler that uses a function to handle events of a single type.
 * This makes it possible to create event handlers without subclassing
 * EventHandler.
 *
 * @tparam EventArgs the types of the event arguments.
 */
template <class... EventArgs>
class SingleEventHandler
    : public EventHandler<SingleEventHandler<EventArgs...>> {
  public:
    /**
     * @brief Creates a new SingleEventHandler with the given handler function.
     *
     * @param handler the function that will be invoked whenever the event is
     * fired.
     */
    explicit SingleEventHandler(const function<void(EventArgs...)>& handler)
        : _handler(handler) {}

    /**
     * @brief Connects this event handler to the given event.
     *
     * @param event the event to connect to.
     */
    void connect(Event<EventArgs...>& event) {
        EventHandler<SingleEventHandler<EventArgs...>>::connect(event,
                                                                &handleEvent);
    }

  private:
    void handleEvent(EventArgs... args) { _handler(args...); }

    function<void(EventArgs...)> _handler;
};

}; // namespace Base::Event

#endif // EVENT_H
