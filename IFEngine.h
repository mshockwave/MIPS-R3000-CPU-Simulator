
#ifndef ARCHIHW1_IFENGINE_H
#define ARCHIHW1_IFENGINE_H

#include "ExecutionEngine.h"
#include "tasks/TaskHandle.h"

class IFEngine : public ExecutionEngine{

private:
    Instructions& instructions;

public:
    IFEngine(Context &ctx, Instructions &instructions_, TaskHandle::ClockHandle &clk) :
            ExecutionEngine(ctx, clk),
            instructions(instructions_){}

    void Start() override;
};


#endif //ARCHIHW1_IFENGINE_H
