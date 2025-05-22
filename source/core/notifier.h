#pragma once

class Notifier;

class Notifiable {
public:
    Notifiable();
    virtual ~Notifiable();

    void unsubscribe(Notifier* pNotifier);
    void removeAll();

protected:
    std::vector<Notifier*> notifiers;
};

class Notifier {
public:
    inline void increaseReferenceCount() { referenceCount++; }
    inline void decreaseReferenceCount() { referenceCount--; }

public:
    Notifier();
    virtual ~Notifier();

    void removeNotifiable( Notifiable* pNotifiable );

protected:
    int32_t referenceCount;
    std::vector<Notifiable*> notifiableInstances;
};
