
#ifndef ARCHIHW1_FLIPFLOP_H
#define ARCHIHW1_FLIPFLOP_H

#include "../Types.h"

template <typename value_t>
class FlipFlop {
private:
    value_t current, next;

public:
    void tick(){
        current = next;
    }

    const value_t GetCurrent() const{ return current; }
    void WriteNext(value_t v){
        next = v;
    }

    bool operator==(const FlipFlop<value_t>& rhs){ return GetCurrent() == rhs.GetCurrent(); }
    bool operator!=(const FlipFlop<value_t>& rhs){ return !(*this == rhs); }

    FlipFlop<value_t>& operator=(const FlipFlop<value_t>& rhs){
        this->WriteNext(rhs.GetCurrent());
        return *this;
    }
};

#endif //ARCHIHW1_FLIPFLOP_H
