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

#ifndef THREADTASK_H
#define THREADTASK_H

#include <QRunnable>

class QObject;
class ThreadTaskCourier;

/* Base class for threaded tasks with support for results, progress, and cancellation.
 * This is built on top of QThreadPool/QRunnable, and makes use of the ThreadTaskCourier
 * to safely relay results.
 *
 * Subclasses must implement the runTask() method for the body of their task execution.
 * Always check isCancelled() at the beginning of the task and any other appropriate
 * points, and return immediately if true.
 *
 * The ThreadTask instance is passed to the caller as the result (via a meta-method
 * invocation of the callback function), who is expected to know how to cast the object
 * and retrieve the result from the subclass. The caller may be destroyed at any time,
 * and this object will be freed by the courier. */

class ThreadTask : public QRunnable
{
	friend class ThreadTaskCourier;

public:
	ThreadTask(QObject *caller, const char *callback);

	void cancel() { cancelFlag = true; }
	bool isCancelled() const { return cancelFlag; }

protected:
	virtual void runTask() = 0;
	virtual void run();

private:
	QObject *taskCaller;
	const char *taskCallback;
	volatile bool cancelFlag;
};

#endif
