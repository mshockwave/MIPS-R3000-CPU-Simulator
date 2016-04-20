
#ifndef ARCHIHW1_FLIPFLOP_H
#define ARCHIHW1_FLIPFLOP_H

#include "../Types.h"

template <typename value_t>
class FlipFlop {
private:
    value_t current, next;

public:

    FlipFlop(const value_t& v){
        this->WriteNext(v);
    }

    void tick(){
        current = next;
    }

    const value_t GetCurrent() const{ return current; }
    void WriteNext(value_t v){
        next = v;
    }

    bool operator==(const FlipFlop<value_t>& rhs){ return GetCurrent() == rhs.GetCurrent(); }
    bool operator!=(const FlipFlop<value_t>& rhs){ return !(*this == rhs); }

    void operator=(const FlipFlop<value_t>& rhs){
        this->WriteNext(rhs.GetCurrent());
    }
    void operator=(const value_t& v){
        this->WriteNext(v);
    }

    friend std::ostream& operator<<(std::ostream& os, const FlipFlop<value_t> self){
        os << self.GetCurrent();
        return os;
    }
};

#endif //ARCHIHW1_FLIPFLOP_H
