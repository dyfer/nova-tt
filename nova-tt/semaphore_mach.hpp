//  semaphore class, mach wrapper
//  based on API proposed in N2043
//
//  Copyright (C) 2009 Tim Blechmann
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#ifndef NOVA_TT_SEMAPHORE_MACH_HPP
#define NOVA_TT_SEMAPHORE_MACH_HPP

#include <cassert>
#include <boost/noncopyable.hpp>

#include <mach/semaphore.h>
#include <mach/task.h>

#include <time.h>

namespace nova
{

/** semaphore class */
template <bool has_timed_wait = false>
class semaphore:
    boost::noncopyable
{
public:
    semaphore(unsigned int i=0)
    {
        kern_return_t status = semaphore_create(mach_task_self(), &sem,  SYNC_POLICY_FIXED_PRIORITY, i);
        assert(status == KERN_SUCCESS);
    }

    ~semaphore(void)
    {
        kern_return_t status = semaphore_destroy(mach_task_self(), &sem);
        assert(status == KERN_SUCCESS);
    }

    /** signal semaphore */
    void post(void)
    {
        kern_return_t status = semaphore_signal(&sem);
        assert(status == KERN_SUCCESS);
    }

    /** wait until this semaphore is signaled */
    void wait(void)
    {
        kern_return_t status = semaphore_wait(&sem);
        assert(status == KERN_SUCCESS);
    }

    /** try to wait for the semaphore
     *
     * \return true, if the value can be decremented
     *         false, otherweise
     */
    bool try_wait(void)
    {
        /* is it possible to implement this with the mach semaphore api? */
        kern_result_t status = semaphore_wait_noblock(&sem);
        return (status == KERN_SUCCESS);
    }

    /** try to wait for the semaphore until timeout
     *
     * \return true, if the value can be decremented
     *         false, otherweise
     */
    bool timed_wait(const struct timespec * absolute_timeout)
    {
        BOOST_STATIC_ASSERT(has_timed_wait);
        clock_t clk = clock();
        for (;;) {
            if (try_wait())
                return true;

            timespec now;
            int status = clock_gettime(clk, &now);
            assert(status);

            if (now.tv_sec > absolute_timeout.tv_sec ||
                (now.tv_sec == absolute_timeout.tv_sec ||
                 now.tv_nsec >= absolute_timeout.tv_nsec))
                return false; // timeout

            timespec wait_time;
            wait_time.tv_nsec = 100000;
            wait_time.tv_sec = 0;

            timespec remain;
            nanosleep(wait_time, remain);
        }
    }


    int value(void)
    {
        /* is it possible to implement this with the mach semaphore api? */
        assert(false);
    }

private:
    semaphore_t sem;
};

} // namespace nova

#endif /* NOVA_TT_SEMAPHORE_MACH_HPP */
