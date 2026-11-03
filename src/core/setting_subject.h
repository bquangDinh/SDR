#ifndef SETTING_SUBJECT_H
#define SETTING_SUBJECT_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

// Thread-safe observable value. Subscribers are called on the thread that calls set(),
// only when the value actually changes.
template <typename T>
class SettingSubject
{
public:
    using Callback = std::function<void(const T&)>;
    using SubscriptionId = uint64_t;

    explicit SettingSubject(T initial = T{}) : _value(initial) {}

    SettingSubject(const SettingSubject&) = delete;
    SettingSubject& operator=(const SettingSubject&) = delete;

    T get() const { return _value.load(); }

    // Returns true if the value changed (and subscribers were notified)
    bool set(T value) {
        if (_value.exchange(value) == value) return false;

        notify(value);
        return true;
    }

    SubscriptionId subscribe(Callback callback) {
        std::lock_guard<std::mutex> lock(_mutex);

        SubscriptionId id = ++_nextId;
        _subscribers.push_back({id, std::make_shared<Callback>(std::move(callback))});

        return id;
    }

    void unsubscribe(SubscriptionId id) {
        std::lock_guard<std::mutex> lock(_mutex);

        for (auto it = _subscribers.begin(); it != _subscribers.end(); ++it) {
            if (it->id == id) {
                _subscribers.erase(it);
                return;
            }
        }
    }

private:
    struct Subscriber {
        SubscriptionId id;
        std::shared_ptr<Callback> callback;
    };

    void notify(const T& value) {
        // Copy under lock and call outside it, so callbacks may subscribe/unsubscribe/set safely
        std::vector<Subscriber> snapshot;

        {
            std::lock_guard<std::mutex> lock(_mutex);
            snapshot = _subscribers;
        }

        for (const Subscriber& s : snapshot) {
            if (*s.callback) (*s.callback)(value);
        }
    }

    std::atomic<T> _value;
    std::mutex _mutex;
    std::vector<Subscriber> _subscribers;
    SubscriptionId _nextId = 0;
};

#endif // SETTING_SUBJECT_H
