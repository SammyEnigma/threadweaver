/* -*- C++ -*-

   This file implements the public interfaces of the Weaver and the Job class.
   It should be the only include file necessary to use the ThreadWeaver
   library.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2005-2013 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://creative-destruction.me $

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#ifndef THREADWEAVER_WEAVER_H
#define THREADWEAVER_WEAVER_H

#include <QtCore/QObject>

#include "Queue.h"
#include "QueueStream.h"

namespace ThreadWeaver {

class Job;
class State;
class WeaverObserver;

/** The Weaver class provides the public implementation of the WeaverInterface.

        Weaver provides a static instance that can be used to perform jobs in
        threads without managing a weaver object. The static instance will
        only be created when it is first accessed. Also, Weaver objects will
        create the threads only when the first jobs are queued. Therefore, the
        creation of a Weaver object is a rather cheap operation.

        The WeaverImpl class provides two parts of API - one for the threads
        that are handled by it, and one for the ThreadWeaver users
        (application developers). To separate those two different API parts,
        Weaver only provides the interface supposed to be used by developers
        of multithreaded applications.

        Weaver creates and destroys WeaverImpl objects. It hides the
        implementation details of the WeaverImpl class. It is strongly
        discouraged to use the WeaverImpl class in programs, as its API will
        be changed without notice.
        Also, Weaver provides a factory method for this purpose that can be overloaded to create
        derived WeaverImpl objects.

    */
// Note: All member documentation is in the WeaverInterface class.
class THREADWEAVER_EXPORT Weaver : public Queue
{
    Q_OBJECT
public:
    explicit Weaver(QObject* parent = 0);

    explicit Weaver(Queue* implementation, QObject* parent = 0);

    /** Destruct a Weaver object. */
    virtual ~Weaver();

    /** Create a QueueStream to enqueue jobs into this queue. */
    QueueStream stream();

    const State* state() const;

    void setMaximumNumberOfThreads(int cap) Q_DECL_OVERRIDE;
    int maximumNumberOfThreads() const Q_DECL_OVERRIDE;
    int currentNumberOfThreads() const Q_DECL_OVERRIDE;


    void registerObserver ( WeaverObserver* );

    /** Return the global Weaver instance.
        In some cases, a global Weaver object per application is
        sufficient for the applications purpose. If this is the case,
        query instance() to get a pointer to a global instance.
        If instance is never called, a global Weaver object will not be
        created.
    */
    static ThreadWeaver::Weaver* instance();
    void enqueue(const QVector<JobPointer>& jobs) Q_DECL_OVERRIDE;
    void enqueue(const JobPointer& job);
    bool dequeue(const JobPointer&) Q_DECL_OVERRIDE;
    void dequeue() Q_DECL_OVERRIDE;
    void finish() Q_DECL_OVERRIDE;
    void suspend() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;
    bool isEmpty() const Q_DECL_OVERRIDE;
    bool isIdle() const Q_DECL_OVERRIDE;
    int queueLength () const Q_DECL_OVERRIDE;
    void requestAbort() Q_DECL_OVERRIDE;
    void reschedule() Q_DECL_OVERRIDE;
    void shutDown() Q_DECL_OVERRIDE;

    /** @brief Interface for the global queue factory. */
    struct GlobalQueueFactory {
        virtual ~GlobalQueueFactory() {}
        virtual Weaver* create(QObject* parent) = 0;

    };
    static void setGlobalQueueFactory(GlobalQueueFactory* factory);
private:
    class Private;
    Private* const d;
};

}

#endif // THREADWEAVER_WEAVER_H