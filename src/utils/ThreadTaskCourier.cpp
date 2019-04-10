/*
 * Copyright 2010-2019 Bluecherry, LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ThreadTaskCourier.h"
#include "ThreadTask.h"
#include <QApplication>
#include <QThread>
#include <QMetaObject>
#include <QMetaType>

ThreadTaskCourier *ThreadTaskCourier::instance = NULL;

ThreadTaskCourier::ThreadTaskCourier()
{
	qRegisterMetaType<ThreadTask*>("ThreadTask*");
}

void ThreadTaskCourier::addTask(QObject *caller)
{
	Q_ASSERT(QThread::currentThread() == qApp->thread());
	Q_ASSERT(caller->thread() == qApp->thread());
	if (!instance)
		instance = new ThreadTaskCourier;

	QHash<QObject*,int>::iterator it = instance->pending.find(caller);
	if (it != instance->pending.end())
	{
		(*it)++;
		return;
	}

	instance->pending.insert(caller, 1);
	connect(caller, SIGNAL(destroyed()), instance, SLOT(objectDestroyed()), Qt::DirectConnection);
}

void ThreadTaskCourier::notify(ThreadTask *task)
{
	Q_ASSERT(instance);
	
	bool ok = QMetaObject::invokeMethod(instance, "deliverNotify", Qt::QueuedConnection,
										Q_ARG(ThreadTask*,task));

	Q_ASSERT(ok);
	Q_UNUSED(ok);
}

void ThreadTaskCourier::deliverNotify(ThreadTask *task)
{
	QObject *caller = task->taskCaller;

	QHash<QObject*,int>::iterator it = pending.find(caller);
	if (it == pending.end())
	{
		/* Caller has probably been deleted already */
		delete task;
		return;
	}

	bool ok = QMetaObject::invokeMethod(caller, task->taskCallback, Qt::DirectConnection, Q_ARG(ThreadTask*,task));
	Q_ASSERT_X(ok, "ThreadTaskCourier", "Invocation of thread task callback failed");
	Q_UNUSED(ok);

	if (*it < 2)
	{
		disconnect(caller, SIGNAL(destroyed()), this, SLOT(objectDestroyed()));
		pending.erase(it);
	}
	else
		(*it)--;

	delete task;
}

void ThreadTaskCourier::objectDestroyed()
{
	pending.remove(sender());
}
