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

#ifndef DAPI_SIMPLE_TIMER__H
#define DAPI_SIMPLE_TIMER__H


//----------
// Overview

/*
	dapi_simple_timer is a container for holding and managing timer events.

	Events are added to the timer and scheduled to fire at either at an absolute
	time (as given by aud_utime_get) or a relative time from the current time.

	When 'process' is called, any events whose timer has expired are triggered.

	After firing, a timer event is disabled.  Timer events may be also configured
	as recurring - after firing, they automatically reschedule themselves rather
	than becoming disabled.


	The timer module must be integrated into the main select loop (or equivalent).
	The two key calls are 'process' - which checks and fires the timers - and
	'get_next_firing' - which returns the absolute time at which process should
	next be called.

	Client code should not poll process, but instead use 'get_next_firing' as
	an input to sleep or select.
 */


//----------
// Include

// Audinate includes
#include "audinate/dante_api.h"


//----------
// DAPI simple timer
// Container for holding and managing one or more timer events

typedef struct dapi_simple_timer dapi_simple_timer_t;


/*
	Allocate container
 */
dapi_simple_timer_t *
dapi_simple_timer_new
(
	size_t max_timers
);


/*
	Design note: in this implementation, the container handles all the resources
	and the client code just passes in the target callback.  To avoid variable
	memory load, this requires that we choose a maximum number of timers at
	initialisation time.
 */


/*
	Free container and release all resources

	Also frees all timers
 */
void
dapi_simple_timer_delete
(
	dapi_simple_timer_t * timers
);


//----------
// Timer events

typedef struct dapi_simple_timer_event dapi_simple_timer_event_t;

/*
	Events (single timers) must be allocated before they are used.  This
	removes a timer event from the pool and assigns it to the client.
 */

/*
	Allocate a timer event

	Newly allocated timer events have NULL callbacks and are not scheduled to fire.

	Returns
		Pointer to new event
		NULL on error (no resources or bad input pointer)
 */
dapi_simple_timer_event_t *
dapi_simple_timer_event_alloc
(
	dapi_simple_timer_t * timers
);

void
dapi_simple_timer_event_release
(
	dapi_simple_timer_event_t * timer
);


// Callbacks

/*
	The callback function for a timer event returns three parameters:

	@param timer the timer event that is firing
	@param time the "current time" of the event firing.  If NULL, an error occured
		and the timer is being cancelled (this should rarely if ever happen).
	@param context the context pointer passed in when the callback was set
 */
typedef void
dapi_simple_timer_event_fn
(
	dapi_simple_timer_event_t * timer,
	const aud_utime_t * time,
	void * context
);

void
dapi_simple_timer_event_set_callback
(
	dapi_simple_timer_event_t * timer,
	dapi_simple_timer_event_fn * callback,
	void * context
);


// Scheduling timers

/*
	Schedule the timer to fire once

	Firing time is specified as 'absolute + delta'.
	Usually, one of absolute or delta is NULL.
	A NULL absolute time means 'use the current time' (from aud_utime_get).
	A NULL delta means 'no delta'.
	See examples, below.

	@param timer an initialised timer event
	@param abs absolute time for timer firing (NULL => now)
	@param delta delta from absolute time for timer firing.

	Examples:
		To fire in 5.2 seconds from now:
			abs = NULL, delta = { 5, 200 * AUD_USEC_PER_MSEC }.
		To fire at absolute time 564 seconds, 23 microseconds:
			abs = { 564, 23 }, delta = NULL
 */
aud_error_t
dapi_simple_timer_event_schedule_once
(
	dapi_simple_timer_event_t * timer,
	const aud_utime_t * abs,
	const aud_utime_t * delta
);


/*
	Schedule a timer to fire periodically

	As dapi_simple_timer_event_schedule_once, but just before firing the callback
	reschedule the timer for +period after the "current time" passed into the process
	call.

	Note that this timer is not strictly periodic.  If a firing is delayed, the
	next firing is +period after the last.  This is usually what is wanted, otherwise
	a delayed timer can result in many firings as the timer catches up.

	@param timer an initialised timer event
	@param abs absolute time for timer firing (NULL => now)
	@param delta delta from absolute time for timer firing.
	@param period detla from last firing for next firing (NULL => treat as 'once off' timer).

 */
aud_error_t
dapi_simple_timer_event_schedule_recurring
(
	dapi_simple_timer_event_t * timer,
	const aud_utime_t * abs,
	const aud_utime_t * delta,
	const aud_utime_t * period
);


/*
	Stop an existing timer.

	Deactivate an active timer and prevent it from firing.
	The timer remains allocated, and the callback retains its current value.
 */
aud_error_t
dapi_simple_timer_event_stop
(
	dapi_simple_timer_event_t * timer
);


/*
	Get the next firing time of the timer event.

	Returns NULL if timer is not scheduled to fire.
 */
const aud_utime_t *
dapi_simple_timer_event_get_next
(
	const dapi_simple_timer_event_t * timer
);

/*
	Get the recurring period of the timer event.

	Returns NULL if timer is not scheduled to fire or is not recurring.
 */
const aud_utime_t *
dapi_simple_timer_event_get_period
(
	const dapi_simple_timer_event_t * timer
);


// Processing timers

/*
	Process all timer events, and fire them as appropriate

	Iterate through the list of timer events, firing any whose next firing time
	is <= the current time.

	@param timers the timers
	@param curr_time the current time (if NULL, obtain using aud_utime_get)
 */
aud_error_t
dapi_simple_timer_process
(
	dapi_simple_timer_t * timers,
	const aud_utime_t * curr_time
);


/*
	Return the time the next timer is scheduled to fire

	Process should be called as soon as possible after this time expires.
	Returns NULL if no timer is scheduled to fire.
 */
const aud_utime_t *
dapi_simple_timer_get_next_firing
(
	dapi_simple_timer_t * timers
);


//----------

#endif // DAPI_SIMPLE_TIMER__H
