//
//  typedef.h
//  
//
//  Created by OKU Junichirou on 2015/10/28.
//
//

#ifndef typedef_h
#define typedef_h

// type definitions
typedef enum {
    eventIdle,
    eventSingleTapped,
    eventDoubleTapped,
    eventTiltLeft,
    eventTiltRight,
    eventStandLeft,
    eventStandRight,
    eventTiltBack,
    eventTiltFront,
    eventTimedOut,
    eventUpsideDown,
    eventEnd,
} event_t;

typedef enum {
    actionIdle,
    actionForward,
    actionForwardFast,
    actionBackward,
    actionBackwardFast,
    actionSelectNext,
    actionSelectPrev,
    actionToAdjusting,
    actionToClock,
    actionCancelAdjusting,
    actionToSleep,
    actionToElapsed,
    actionRecord,
    actionToRecorded,
    actionEnd,
} action_t;

typedef enum {
    stateIdle,
    stateElapsed,
    stateRecorded,
    stateClock,
    stateAdjusting,
    stateEnd,
} state_t;

#endif /* typedef_h */
