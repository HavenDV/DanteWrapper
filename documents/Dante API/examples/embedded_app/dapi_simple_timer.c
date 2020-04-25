/*
 * File     : dapi_simple_timer.h
 * Created  : March 2016
 * Author   : Andrew White
 * Synopsis : A simple timer API for scheduling and triggering events
 *
 * This software is Copyright (c) 2004-2017, Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

//----------
// Include

// Audinate includes
#include "dapi_simple_timer.h"

// System includes
#include <assert.h>


//----------
// Data types

typedef enum dapi_simple_timer_mode
{
	DAPI_SIMPLE_TIMER_MODE__NONE = 0,
		// timer is allocated but not scheduled
	DAPI_SIMPLE_TIMER_MODE__SINGLE,
		// timer is scheduled to fire once
	DAPI_SIMPLE_TIMER_MODE__RECURRING,
		// timer is scheduled to fire recurringly
} dapi_simple_timer_mode_t;

struct dapi_simple_timer_event
{
	dapi_simple_timer_mode_t mode;
		// Can derive this from next and period, but helpful to cache a definitive copy
	aud_utime_t next, period;
	dapi_simple_timer_event_fn * cb;
	void * context;
	dapi_simple_timer_t * parent;
		// NULL if timer not 'allocated'
};

struct dapi_simple_timer
{
	// Manage timers
	struct dapi_simple_timer__timers
	{
		size_t n, max;
		dapi_simple_timer_event_t * timers;
	} timers;

	// cache of information for processing
	struct dapi_simple_timer__process
	{
		aud_bool_t changed;
		size_t num_active;
		aud_utime_t curr_time, next_time;
	} process;
};


/*
	Implementation note:

	To keep things simple, we use an array and iterate through it looking for
	timers.  If there were likely to be a lot of timers, we could still allocate
	a single array for storage but would use three linked lists:
		- a sorted list for active timers
		- an unsorted list for inactive timers
		- an unsorted "free list" for unallocated timers

	A list implementation requires more memory, and is trickier to get right
	because changing the state of a timer during processing changes the lists,
	possibly while we are in the middle of iterating over them.
 */


//----------
// DAPI simple timer

dapi_simple_timer_t *
dapi_simple_timer_new
(
	size_t max_timers
)
{
	if (! max_timers)
		return NULL;

	dapi_simple_timer_t * timers = calloc(1, sizeof(*timers));
	if (! timers)
		return NULL;

	timers->timers.timers = calloc(max_timers, sizeof(timers->timers.timers[0]));
	if (! timers->timers.timers)
	{
		free(timers);
		return NULL;
	}

	timers->timers.n = 0;
	timers->timers.max = max_timers;
	return timers;
}


void
dapi_simple_timer_delete
(
	dapi_simple_timer_t * timers
)
{
	if (! timers)
		return;

	if (timers->timers.timers)
	{
		free(timers->timers.timers);
		timers->timers.timers = NULL;
	}

	free(timers);
}


//----------
// Timer events

dapi_simple_timer_event_t *
dapi_simple_timer_event_alloc
(
	dapi_simple_timer_t * timers
)
{
	dapi_simple_timer_event_t * the_timer;

	assert(timers);
	if (! timers)
		return NULL;
	assert(timers->timers.max);
	assert(timers->timers.timers);
	assert(timers->timers.n <= timers->timers.max);

	// if there's a free timer, use it
	if (timers->timers.n < timers->timers.max)
	{
		the_timer = timers->timers.timers + timers->timers.n;
		timers->timers.n ++;
	}
	else
	{
		// try to find an invalid timer in list
		size_t i;
		for (i = 0; i < timers->timers.max; i++)
		{
			the_timer = timers->timers.timers + i;
			if (! the_timer->parent)
			{
				goto l__init_timer;
			}
		}

		// we're full
		return NULL;
	}

l__init_timer:
	the_timer->mode = DAPI_SIMPLE_TIMER_MODE__NONE;
	the_timer->parent = timers;
	return the_timer;
}


void
dapi_simple_timer_event_release
(
	dapi_simple_timer_event_t * timer
)
{
	assert (timer->parent);

	timer->parent = NULL;
}


// Callbacks

void
dapi_simple_timer_event_set_callback
(
	dapi_simple_timer_event_t * timer,
	dapi_simple_timer_event_fn * callback,
	void * context
)
{
	timer->cb = callback;
	timer->context = context;
}


// Scheduling timers

aud_error_t
dapi_simple_timer_event_schedule_once
(
	dapi_simple_timer_event_t * timer,
	const aud_utime_t * abs,
	const aud_utime_t * delta
)
{
	return dapi_simple_timer_event_schedule_recurring(timer, abs, delta, NULL);
}


aud_error_t
dapi_simple_timer_event_schedule_recurring
(
	dapi_simple_timer_event_t * timer,
	const aud_utime_t * abs,
	const aud_utime_t * delta,
	const aud_utime_t * period
)
{
	assert(timer);
	assert(timer->parent);
	if (! (timer && timer->parent))
		return AUD_ERR_INVALIDPARAMETER;

	if (period)
	{
		if (aud_utime_compare(&AUD_UTIME_ZERO, period) >= 0)
		{
			return AUD_ERR_INVALIDPARAMETER;
				// period must be > zero
				// We could add sanity check here and make minimum period slightly
				// larger than zero
		}
	}

	if (abs)
	{
		timer->next = * abs;
	}
	else
	{
		aud_utime_get(&timer->next);
	}

	if (delta)
	{
		aud_utime_add(&timer->next, delta);
	}

	if (period)
	{
		timer->period = *period;
		timer->mode = DAPI_SIMPLE_TIMER_MODE__RECURRING;
	}
	else
	{
		timer->period = AUD_UTIME_ZERO;
		timer->mode = DAPI_SIMPLE_TIMER_MODE__SINGLE;
	}

	timer->parent->process.changed = AUD_TRUE;
	return AUD_SUCCESS;
}


/*
	Stop an existing timer.

	Deactivate an active timer and prevent it from firing.
	The timer remains allocated, and the callback retains its current value.
 */
aud_error_t
dapi_simple_timer_event_stop
(
	dapi_simple_timer_event_t * timer
)
{
	assert(timer);
	assert(timer->parent);
	if (! (timer && timer->parent))
		return AUD_ERR_INVALIDPARAMETER;

	if (timer->mode)
	{
		timer->mode = DAPI_SIMPLE_TIMER_MODE__NONE;
		timer->parent->process.changed = AUD_TRUE;
	}

	return AUD_SUCCESS;
}


const aud_utime_t *
dapi_simple_timer_event_get_next
(
	const dapi_simple_timer_event_t * timer
)
{
	assert(timer);
	assert(timer->parent);
	if (! (timer && timer->parent))
		return NULL;

	if (timer->mode)
	{
		return &timer->next;
	}
	else
	{
		return NULL;
	}
}


const aud_utime_t *
dapi_simple_timer_event_get_period
(
	const dapi_simple_timer_event_t * timer
)
{
	assert(timer);
	assert(timer->parent);
	if (! (timer && timer->parent))
		return NULL;

	if (timer->mode == DAPI_SIMPLE_TIMER_MODE__RECURRING)
	{
		return &timer->period;
	}
	else
	{
		return NULL;
	}
}


// Processing timers

aud_error_t
dapi_simple_timer_process
(
	dapi_simple_timer_t * timers,
	const aud_utime_t * curr_time
)
{
	size_t i;

	assert(timers);
	if (! timers)
		return AUD_ERR_INVALIDPARAMETER;

	if (curr_time)
	{
		timers->process.curr_time = *curr_time;
	}
	else
	{
		aud_utime_get(&timers->process.curr_time);
	}

	if (! dapi_simple_timer_get_next_firing(timers))
	{
		// nothing active
		return AUD_SUCCESS;
	}

	if (aud_utime_compare(&timers->process.curr_time, &timers->process.next_time) < 0)
	{
		// nothing ready to fire
		return AUD_SUCCESS;
	}

	for (i = 0; i < timers->timers.n; i++)
	{
		dapi_simple_timer_event_t * the_timer = timers->timers.timers + i;
		if (the_timer->parent && the_timer->mode)
		{
			if (aud_utime_compare(&timers->process.curr_time, &the_timer->next) < 0)
			{
				// timer not ready
				continue;
			}

			// Before firing the timer, reset it
			// We do this before firing the timer in case the callback itself
			// reconfigures this timer
			timers->process.changed = AUD_TRUE;
			if (the_timer->mode == DAPI_SIMPLE_TIMER_MODE__SINGLE)
			{
				the_timer->mode = DAPI_SIMPLE_TIMER_MODE__NONE;
			}
			else
			{
				the_timer->next = timers->process.curr_time;
				aud_utime_add(&the_timer->next, &the_timer->period);
			}
			if (the_timer->cb)
			{
				(*the_timer->cb)(the_timer, &timers->process.curr_time, the_timer->context);
			}
		}
	}

	return AUD_SUCCESS;
}


const aud_utime_t *
dapi_simple_timer_get_next_firing
(
	dapi_simple_timer_t * timers
)
{
	assert(timers);
	if (! timers)
		return NULL;

	if (timers->process.changed)
	{
		// update our timing cache
		aud_utime_t next_time = {0};
		size_t n_active = 0;
		size_t i;
		for (i = 0; i < timers->timers.n; i++)
		{
			dapi_simple_timer_event_t * the_timer = timers->timers.timers + i;
			if (the_timer->parent && the_timer->mode)
			{
				if (n_active)
				{
					if (aud_utime_compare(&the_timer->next, &next_time) < 0)
					{
						next_time = the_timer->next;
					}
				}
				else
				{
					next_time = the_timer->next;
				}
				n_active++;
			}
		}

		timers->process.changed = AUD_FALSE;
		timers->process.num_active = n_active;
		if (n_active)
		{
			timers->process.next_time = next_time;
		}
	}

	if (timers->process.num_active)
	{
		return &timers->process.next_time;
	}
	else
	{
		return NULL;
	}
}


//----------
