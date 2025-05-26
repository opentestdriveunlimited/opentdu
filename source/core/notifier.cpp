#include "shared.h"
#include "notifier.h"

Notifiable::Notifiable()
{

}

Notifiable::~Notifiable()
{
    removeAll();
}

void Notifiable::unsubscribe(Notifier *pNotifier)
{
    for (Notifier* notifier : notifiers) {
        if (notifier == pNotifier) {
            pNotifier->decreaseReferenceCount();
            pNotifier->removeNotifiable( this );
        }
    }
}

void Notifiable::removeAll()
{
    for (Notifier* notifier : notifiers) {
        unsubscribe( notifier );
    }
    notifiers.clear();
}

Notifier::Notifier()
    : referenceCount( 0 )
{
    notifiableInstances.reserve( 4 );
}

Notifier::~Notifier()
{
    
}

void Notifier::removeNotifiable(Notifiable *pNotifiable)
{
    for (auto it = notifiableInstances.begin(); it != notifiableInstances.end(); it++) {
        if (*it == pNotifiable) {
            notifiableInstances.erase(it);
            return;
        }
    }
}
