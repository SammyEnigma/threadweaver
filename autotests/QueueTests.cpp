#include "QueueTests.h"

#include <QtCore/QChar>

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QDebug>
#include <QtTest/QtTest>

#include "AppendCharacterJob.h"

#include <Queueing.h>
#include <Job.h>
#include <State.h>
#include <QueuePolicy.h>
#include <JobSequence.h>
#include <JobCollection.h>
#include <DebuggingAids.h>
#include <WeaverObserver.h>
#include <DependencyPolicy.h>
#include <ResourceRestrictionPolicy.h>

#include <ThreadWeaver.h>
#include <Thread.h>
#include <QObjectDecorator.h>

// always. ahm. no. never. never show your private parts in public.
#ifdef THREADWEAVER_PRIVATE_API
#error "PARTS OF THREADWEAVER'S PRIVATE API ARE INCLUDED IN PUBLIC HEADERS!"
#endif

QMutex s_GlobalMutex;

LowPriorityAppendCharacterJob::LowPriorityAppendCharacterJob (QChar c, QString* stringref)
    : AppendCharacterJob (c, stringref)
{}

int LowPriorityAppendCharacterJob ::priority() const
{
    return -1;
}

HighPriorityAppendCharacterJob::HighPriorityAppendCharacterJob (QChar c, QString* stringref)
    : AppendCharacterJob (c, stringref)
{}

int HighPriorityAppendCharacterJob::priority() const
{
    return 1;
}

SecondThreadThatQueues::SecondThreadThatQueues()
    : QThread()
{
}

void SecondThreadThatQueues::run()
{
    QString sequence;
    AppendCharacterJob a( 'a', &sequence );

    ThreadWeaver::enqueue_raw(&a);
    ThreadWeaver::Weaver::instance()->finish();
    QCOMPARE( sequence, QString("a" ) );
}

QueueTests::QueueTests( QObject* parent )
    : QObject( parent )
    , autoDeleteJob ( 0 )
{
}

void QueueTests::initTestCase ()
{
    ThreadWeaver::setDebugLevel ( true,  1 );
}

void QueueTests::SimpleQueuePrioritiesTest() {
    using namespace ThreadWeaver;

    Weaver weaver;
    weaver.setMaximumNumberOfThreads ( 1 ); // just one thread
    QString sequence;
    LowPriorityAppendCharacterJob jobA ( QChar( 'a' ), &sequence );
    AppendCharacterJob jobB ( QChar( 'b' ), &sequence );
    HighPriorityAppendCharacterJob jobC ( QChar( 'c' ), &sequence );

    // queue low priority, then normal priority, then high priority
    // if priorities are processed correctly, the jobs will be executed in reverse order

    weaver.suspend();

    enqueue_raw(&weaver, &jobA);
    enqueue_raw(&weaver, &jobB);
    enqueue_raw(&weaver, &jobC);

    weaver.resume();
    weaver.finish();

    QCOMPARE ( sequence, QString ("cba" ) );
}

void QueueTests::WeaverInitializationTest()
{ // this one mostly tests the sanity of the startup behaviour
    ThreadWeaver::Weaver weaver;
    QCOMPARE (weaver.currentNumberOfThreads(), 0);
    QVERIFY (weaver.isEmpty());
    QVERIFY(weaver.isIdle());
    QVERIFY(weaver.queueLength() == 0);
    weaver.finish();
}

void QueueTests::QueueFromSecondThreadTest()
{
    ThreadWeaver::Weaver::instance(); //create global instance in the main thread
    SecondThreadThatQueues thread;
    thread.start();
    thread.wait();
    QVERIFY ( ThreadWeaver::Weaver::instance()->isIdle() );
}

void QueueTests::deleteJob(ThreadWeaver::JobPointer job)
{   // test that signals are properly emitted (asynchronously, that is):
    QVERIFY( thread() == QThread::currentThread() );
    QVERIFY( job == autoDeleteJob );
    delete autoDeleteJob; autoDeleteJob = 0;
}

void QueueTests::DeleteDoneJobsFromSequenceTest()
{
    using namespace ThreadWeaver;
    QString sequence;
    autoDeleteJob = new QObjectDecorator(new AppendCharacterJob(QChar('a'), &sequence));
    AppendCharacterJob b(QChar('b'), &sequence);
    AppendCharacterJob c(QChar('c'), &sequence);
    JobCollection jobCollection;
    jobCollection << make_job_raw(autoDeleteJob) << b << c;
    QVERIFY(autoDeleteJob != 0);
    QVERIFY(connect(autoDeleteJob, SIGNAL(done(ThreadWeaver::JobPointer)),
                    SLOT(deleteJob(ThreadWeaver::JobPointer))));
    stream() << jobCollection;
    QTest::qWait(100); // return to event queue to make sure signals are delivered
    Weaver::instance()->finish();
    QTest::qWait(100); // return to event queue to make sure signals are delivered
    // no need to delete a, that should be done in deleteJob
    QVERIFY( autoDeleteJob == 0 );
}

void QueueTests::deleteCollection(ThreadWeaver::JobPointer collection)
{
    QVERIFY( thread() == QThread::currentThread() );
    QVERIFY( collection == autoDeleteCollection );
    delete autoDeleteCollection; autoDeleteCollection = 0;
}

void QueueTests::DeleteCollectionOnDoneTest()
{
    using namespace ThreadWeaver;
    QString sequence;
    autoDeleteCollection = new QObjectDecorator(new JobCollection);
    QVERIFY(connect(autoDeleteCollection, SIGNAL(done(ThreadWeaver::JobPointer)),
                    SLOT(deleteCollection(ThreadWeaver::JobPointer))));

    AppendCharacterJob a( QChar( 'a' ), &sequence );
    AppendCharacterJob b( QChar( 'b' ), &sequence );
    *autoDeleteCollection->collection() << a << b;

    enqueue_raw(autoDeleteCollection);
    // return to event queue to make sure signals are delivered
    // (otherwise, no slot calls would happen before the end of this function)
    // I assume the amount of time that we wait does not matter
    QTest::qWait(10);
    Weaver::instance()->finish();
    // return to event queue to make sure signals are delivered
    QTest::qWait(10);
    // no need to delete a, that should be done in deleteJob
    QVERIFY( sequence.length() == 2 );
    QVERIFY( autoDeleteCollection == 0 );
}

QTEST_MAIN ( QueueTests )
