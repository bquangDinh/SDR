#ifndef SINGLETON_H
#define SINGLETON_H

template<typename T>
class Singleton {
public:
    static T& instance() {
        static T inst;

        return inst;
    }

    Singleton(const Singleton&) = delete; // make sure there is only one instance
    Singleton& operator=(const Singleton&) = delete; // same as above

protected:
    Singleton() = default;

    virtual ~Singleton() = default;
};
#endif // SINGLETON_H
