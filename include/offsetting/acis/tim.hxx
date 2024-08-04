/* $Id: tim.hxx,v 1.16 2002/01/30 17:29:08 skenny Exp $ */
/*******************************************************************/
/*    Copyright (c) 1989-2020 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/

#ifndef TIMH
#define TIMH

#ifdef D3_TIMES

// THE D-CUBED PERFORMANCE TIMING MECHANISM
EVEN MORE ADVANCED - ADDING EXTRA BUCKIn the most simple circumstance - and this is what TIM_init domake a bucket along with the timer, and hang the bucket on theBelow is more-or-less what the TIM_init macro dostatic TIMER_BUCKET bucket( "bucket" );	// the name the bucketstatic TIMER_HANDLE TIMER_ADD_BUCKET add_bucket( &handle, &bucket );	// hang bucket oTIMING_DATA timer( "func", 1, &ha...now off we go and do somethingBuckets are attached to timers by the TIMER_ADD_BUCKET class. actually creating an object with local scope which performs tThe reason for doing it this way is that the bucket gets unawhen the TIMER_ADD_BUCKET goes out of scoRULE 4: TIMER_ADD_BUCKET should always have local scope. Alshould make it BEFORE you make the TIMING_DATA itself, or the will be incorreMore usefully, here's how we attach an additional bucket to else's timer. Suppose we have either declared, or included a file with this declaratiextern TIMER_HANDLE someone_else_TIMER_HANDthen, within an "#ifdef D3_TIMES" we could wristatic TIMER_BUCKET my_bucket( "my_calls_to_someone_eTIMER_ADD_BUCKET my_add_bucket( &someone_else_TIMER_HANDLE, &my_bu...stuff which calls "someone_else"and we will get, in addition to anything else, a cumulativtime for all the calls to "someone_else" when called from withblock of coAny number of buckets can be attached to pretty much any nutimers. Buckets are sufficiently smart that time will ncounted twice (or more). For example, if you rig up a buaccumulate time spent in CSI and SSI, time spent in CSI on beSSI will not be counted twice. It is only the first timer thatrunning with a particular bucket attached that will be allowedits time in at the eFor an example, look in blend/kernbool/capping/capping.cxx .

// VPL 18June2001: moved the implementation classes out of D3_TIMES macro definition.

// The basic macros for "simple use". We can add more if the need
// arises.

// Automatically define a timer and bucket to go togther. The timer
// issues timing statements if output is non-zero. The bucket will
// output a cumulative time at image exit, or when
// TIM_output_timer_buckets is invoked (e.g. by the test harness "time"
// command.

#    define TIM_init_output(fn, output)                   \
        LOCAL_VAR TIMER_BUCKET _BUCKET(fn);               \
        LOCAL_VAR TIMER_HANDLE _HANDLE;                   \
        TIMER_ADD_BUCKET _ADD_BUCKET(&_HANDLE, &_BUCKET); \
        TIMING_DATA _DATA(fn, output, &_HANDLE)

// Automatically define a timer and bucket, both of which output in the
// normal manner.

#    define TIM_init(fn) TIM_init_output(fn, 1)

// Automatically define a timer but which does no accumulation.

#    define TIM_init_no_acc(fn)         \
        LOCAL_VAR TIMER_HANDLE _HANDLE; \
        TIMING_DATA _DATA(fn, 1, &_HANDLE)

// Automatically define a bucket and timer to accumulate times for the
// given handle. You would use this if you have defined the handle
// externally so that other algorithms can reference it.

#    define TIM_init_handle_output(fn, handle_ptr, output)  \
        LOCAL_VAR TIMER_BUCKET _BUCKET(fn);                 \
        TIMER_ADD_BUCKET _ADD_BUCKET(handle_ptr, &_BUCKET); \
        TIMING_DATA _DATA(fn, output, handle_ptr)

// Automatically define a timer which uses the given handle which
// outputs in the normal manner.

#    define TIM_init_handle(fn, handle_ptr) TIM_init_handle_output(fn, handle_ptr, 1)

// Automatically define a timer to put its results in this bucket. You
// would use this if you want several different functions all to
// accumulate a combined total. The timer will issue timing statements
// per-call if output is non-zero.

#    define TIM_init_bucket_output(fn, bucket_ptr, output)  \
        LOCAL_VAR TIMER_HANDLE _HANDLE;                     \
        TIMER_ADD_BUCKET _ADD_BUCKET(&_HANDLE, bucket_ptr); \
        TIMING_DATA _DATA(fn, output, &_HANDLE)

// Automatically create a timer to dump its results in the given bucket,
// and *do* produce timing statements for each call.

#    define TIM_init_bucket(fn, ptr) TIM_init_bucket_output(fn, ptr, 1)

#    define TIM_out() \
        { _DATA.stop(); }

#else  // D3_TIMES

#    define TIM_init(fn)

#    define TIM_init_output(fn, output)

#    define TIM_init_no_acc(fn)

#    define TIM_init_handle(fn, handle_ptr)

#    define TIM_init_handle_output(fn, handle_ptr, output)

#    define TIM_init_bucket(fn, bucket_ptr)

#    define TIM_init_bucket_output(fn, bucket_ptr, output)

#    define TIM_out()

#endif  // D3_TIMES

// VPL 18June2001: moved the implementation classes out of D3_TIMES macro definition.

#include <stdlib.h>
#include <string.h>

#include "base.hxx"  // to make sure we have ACIS_OBJECT base class
#include "dcl_kern.h"
#include "debugmsc.hxx"  // for D3 macros
#ifndef _WINDOWS

// STIPORT TAC macintosh doesn't have these headers
#    if !defined(mac) || defined(MacX)
#        include <sys/times.h>
#        include <sys/types.h>
#        ifndef MacX
#            include <malloc.h>
#        endif
#    else
#        include <extras.h>  // for strdup
#    endif

#else
#    include <time.h>
#endif

// Now the implementation of the classes themselves.

// First we need to be able to create arbitrary lists of buckets, as
// pretty much any bucket can be attached to any timer.

class TIMER_BUCKET;

class DECL_KERN TIMER_BUCKET_LIST : public ACIS_OBJECT {
  public:
    TIMER_BUCKET_LIST(TIMER_BUCKET* b, TIMER_BUCKET_LIST* n = NULL): bucket(b), next(n) {}
    TIMER_BUCKET* bucket;
    TIMER_BUCKET_LIST* next;
};

// This class maintains accumulated times for all calls to any timers
// (TIMING_DATAs) to which it has been attached. We also maintain a
// global list of all the existing buckets so that
// TIM_output_timer_buckets() can zip through them all and print the
// statistics.

class DECL_KERN TIMER_BUCKET : public ACIS_OBJECT {
    // The name which will be printed ahead of "accumulated times".

    char* _name;

    // User time and system time accumulate here.

    double _user_time, _sys_time;

    // Number of calls to the timer which have been made. If this is 0
    // we won't bother outputting anything.

    int _ncalls;

    // The number of running timers which want to put their time into
    // this bucket will be kept here. That's because only the first
    // of those timers started will actually be allowed to do so, to
    // stop us counting double (or worse). This gets incremented each
    // time a timer with an interest in this bucket starts, and
    // decremented when it stops.

    int _active;

    // To main the global list. Doubly-linked so that we can easily
    // extract ourselves when we are destroyed.

    TIMER_BUCKET *_prev, *_next;

  public:
    // Constructor, given the name.

    TIMER_BUCKET(char* n);

    ~TIMER_BUCKET();

    // Next in the global list of all existing buckets.

    TIMER_BUCKET* next() const { return _next; }

    char* name() { return _name; }

    // VPL 19June2001: added functions to access user time & system time.
    double user_time() const { return _user_time; }
    double system_time() const { return _sys_time; }

    // Print the accumulated time.

    void output();

    friend class TIMING_DATA;
};

// The "handle" class. Handles are made for TIMING_DATAs, and any
// buckets can then be hung on the handle. The handle also allows
// recursion to be handled correctly as it should not have local scope.

class DECL_KERN TIMER_HANDLE : public ACIS_OBJECT {
    // The depth of any recursive call to a timer.

    int _depth;

    // The list of buckets the timer is interested in.

    TIMER_BUCKET_LIST* _buckets;

  public:
    TIMER_HANDLE(): _depth(0), _buckets(NULL) {}

    ~TIMER_HANDLE() {}

    friend class TIMER_ADD_BUCKET;
    friend class TIMING_DATA;
};

// This is the TIMING_DATA class which represents a "timer". When it is
// constructed it starts running. It may be explicitly stopped, and even
// re-started. When it is stopped it looks for any attached buckets into
// which it will dump its times so as to accumulate running totals.

class DECL_KERN TIMING_DATA : public ACIS_OBJECT {
    // The name to be printed ahead of "timings".

    char* _name;

    // Whether the timer is running.

    int _running;

    // Whether timing statements are to be issued by this timer.

    int _output;

    // The moment at which we started.

#ifndef _WINDOWS
#    if defined(mac) && !defined(MacX)
    unsigned long initial_time;
#    else
    struct tms initial_times;
#    endif
#else
    clock_t initial_time;
#endif

    // The "handle". This is the static or global data structure where
    // buckets actually get attached. Being static/global, it also
    // allows us to handle recursion properly.

    TIMER_HANDLE* _handle;

  public:
    // Constructor, given the name, whether we want this timer to
    // generate per-call timings or not, and an associated handle.

    TIMING_DATA(char* n, int oput = 1, TIMER_HANDLE* h = NULL);

    ~TIMING_DATA();

    char* name() {
        return _name;
    }

    // Change the timer's name. Not usually needed, except for
    // marginally nefarious purposes.

    void set_name(char const* new_name);

    // Re-start the timer. If not running (i.e. already been stopped) we
    // are restarted. If already running the time is re-zeroed, so any
    // time between the original start and re-start is "lost".

    void start();

    // Stop the timer. Also outputs a timing statement if output is
    // requested.

    void stop();

    // Return the number of elapsed seconds since the timer was started.

    double elapsed_seconds();
};

// A class of which we make an instance in order to tell the given timer
// that it should be accumulating its time in the given bucket. We do
// this with a class object (instead of an obvious function call) so as
// to undo the effect automatically when we go out of scope.

class DECL_KERN TIMER_ADD_BUCKET : public ACIS_OBJECT {
    // The timer (referenced via its handle) to which we have added a bucket.

    TIMER_HANDLE* _handle;

  public:
    // Add bucket to timer (actually its handle). A timer can have any
    // number of buckets. Even if you accidentally repeat some it
    // shouldn't matter.
    // VPL 19June2001: added level SPAparameter. this controls whether or not the bucket will be added to timer,
    // based on the current setting of the d3_times option. default value of -1 will always add the timer.

    TIMER_ADD_BUCKET(TIMER_HANDLE* handle, TIMER_BUCKET* bucket, int iLevel = -1);

    // Destructor which automatically unravels what we did.

    ~TIMER_ADD_BUCKET();
};

// Function to output and reset all the timing buckets in existence.
// This is what the test harness "time" command calls, though in
// principle anyone else could too.
// this function is implemented only in the D3_TIMES version.

void DECL_KERN TIM_output_timer_buckets();

// VPL 19June2001: new function for timing data output to file, in customizable format.
// Function to ouput all the timing data from current bucket list. the data will be
// written out into a file (with specified name). the format for the data is also taken as
// argument. this format controls which buckets will be accessed (filter like behaviour).

void DECL_KERN TIM_log_data(const char* filename, const char* format);

#endif