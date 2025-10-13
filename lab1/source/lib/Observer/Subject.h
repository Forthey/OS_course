#pragma once
#include <algorithm>
#include <memory>
#include <vector>

#include "Observer.h"

template<typename Container = std::vector<Observer *> >
class Subject {
public:
    virtual ~Subject() = default;

    virtual void attach(Observer *observer);

    virtual void detach(Observer *observer);

    virtual void notify(std::shared_ptr<Message> message);

protected:
    Container m_observers;
};

template<typename Container>
void Subject<Container>::attach(Observer *observer) {
    if constexpr (std::is_same_v<Container, std::vector<Observer*> >) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end()) {
            m_observers.push_back(observer);
        }
    }
}

template<typename Container>
void Subject<Container>::detach(Observer *observer) {
    if constexpr (std::is_same_v<Container, std::vector<Observer* > >) {
        std::erase(m_observers, observer);
    }
}

template<typename Container>
void Subject<Container>::notify(std::shared_ptr<Message> message) {
    for (auto *observer: m_observers) {
        observer->put(message);
    }
}
