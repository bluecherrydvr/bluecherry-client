/*
 * Copyright 2010-2013 Bluecherry
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

#include "ThreadTask.h"
#include "ThreadTaskCourier.h"
#include <QApplication>

ThreadTask::ThreadTask(QObject *caller, const char *callback)
	: taskCaller(caller), taskCallback(callback), cancelFlag(false)
{
	/* This restriction could be removed by making the courier thread-local */
	Q_ASSERT(caller->thread() == qApp->thread());

	setAutoDelete(false);
	ThreadTaskCourier::addTask(caller);
}

void ThreadTask::run()
{
	Q_ASSERT(!autoDelete());
	runTask();
	ThreadTaskCourier::notify(this);
}
