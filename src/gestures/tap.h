#ifndef TAP_H
#define TAP_H

#include "gesture.h"
#include "gesturerecognizer.h"

class TapGesture : public Gesture
{
public:
    enum TapState {
        NoGesture,
        TapStarted,
        TapFinished
    };

    TapGesture();
    ~TapGesture();

    TapState state;
    int x;
    int y;
    int verticalRadius;
    int horizontalRadius;
};

class TapRecognizer : public GestureRecognizer
{
public:
    TapRecognizer();

    bool useTimer() const;
    GestureRecognizer::Action recognize(Gesture *gesture, const NIXTouchEvent &event);
    Gesture *create();
};

#endif // TAP_H