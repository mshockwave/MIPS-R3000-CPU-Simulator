
#ifndef ARCHIHW1_IFENGINE_H
#define ARCHIHW1_IFENGINE_H

#include "../ExecutionEngine.h"

/*
 * Response for IF stage
 * Also the 'clock thread'
 * */
class IFEngine : public ExecutionEngine{

public:
    IFEngine(Context &ctx, ClockHandle &ch) :
            ExecutionEngine(ctx, ch) { }

    void Start() override;
};

#endif //ARCHIHW1_IFENGINE_H
