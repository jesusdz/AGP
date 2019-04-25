#ifndef INTERACTION_H
#define INTERACTION_H

class Interaction
{
public:

    bool update();

    void postUpdate();

    bool isManipulating() const {
        return state == State::Translating ||
               state == State::Rotating ||
               state == State::Scaling;
    }


private:

    bool idle();
    bool navigate();
    bool focus();
    bool translate();
    bool rotate();
    bool scale();


    enum State { Idle, Navigating, Focusing, Translating, Rotating, Scaling };
    State state = State::Idle;
    State nextState = State::Idle;
};

#endif // INTERACTION
