
#ifndef ARCHIHW1_IDENGINE_H
#define ARCHIHW1_IDENGINE_H

#include "../ExecutionEngine.h"

class IDEngine : public ExecutionEngine{

public:
    IDEngine(Context &ctx, ClockHandle &ch) :
            ExecutionEngine(ctx, ch) { }

    void Start() override;
};

#endif //ARCHIHW1_IDENGINE_H
