//
//  J1ClockKit.hpp
//
//
//  Created by OKU Junichirou on 2015/10/18.
//
//

#ifndef J1ClockKit_h
#define J1ClockKit_h

#ifdef ARDUINO
#include <Time.h>
#else
#include "Time.h"
#endif

namespace J1ClockKit {
    class Crown {
    public:
        Crown();

        void load( tmElements_t &tm );
        void save( tmElements_t &tm );

        int  getValue();

        void forward ( const int val = 1 );
        void backward( const int val = 1 );

        tmByteFields getField();
        void select( const tmByteFields field );
        void move( const int val );

    protected:
        bool isLeap( const int year );
        tmElements_t  timeElements;
        tmByteFields  field;
        int           minval;
        int           maxval;
        int           value;

    private:
        void prepare();
        void cleanup();
    };

    class ElapsedTimer {
    public:
        ElapsedTimer();

        void start( const int origin = 0 );
        unsigned long elapsed();

    protected:
        unsigned long _origin;
    };

    static ElapsedTimer* start( ElapsedTimer* timer );
    static ElapsedTimer* kill ( ElapsedTimer* timer );

} // namespace

#endif /* J1ClockKit_h */
