#ifndef INTERACTION_H
#define INTERACTION_H

class Interaction
{
public:

    bool update();

private:

    bool idle();
    bool navigate();
    bool focus();
    bool translate();
    bool rotate();
    bool scale();

    enum State { Idle, Navigating, Focusing, Translating, Rotating, Scaling };
    State state = State::Idle;
};

#endif // INTERACTION
