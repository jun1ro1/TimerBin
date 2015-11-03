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
    actionUpsideDown,
    actionDisplayOpend,
} action_t;

typedef enum {
    stateIdle,
    stateClock,
    stateAdjusting,
    stateSleep,
    stateUpsideDown,
} state_t;

#endif /* typedef_h */
