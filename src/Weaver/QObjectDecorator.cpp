#include <QObjectDecorator.h>
#include <JobCollection.h>
#include <Sequence.h>
#include <ManagedJobPointer.h>

namespace ThreadWeaver
{

QObjectDecorator::QObjectDecorator(JobInterface *decoratee, QObject *parent)
    : QObject(parent)
    , IdDecorator(decoratee)
{
}

QObjectDecorator::QObjectDecorator(JobInterface *decoratee, bool autoDelete, QObject *parent)
    : QObject(parent)
    , IdDecorator(decoratee, autoDelete)
{
}

void QObjectDecorator::defaultBegin(JobPointer self, Thread *thread)
{
    Q_ASSERT(job());
    Q_EMIT started(self);
    job()->defaultBegin(self, thread);
}

void QObjectDecorator::defaultEnd(JobPointer self, Thread *thread)
{
    Q_ASSERT(job());
    job()->defaultEnd(self, thread);
    if (!self->success()) {
        Q_EMIT failed(self);
    }
    Q_EMIT done(self);
}

}

