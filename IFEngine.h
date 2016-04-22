
#ifndef ARCHIHW1_IFENGINE_H
#define ARCHIHW1_IFENGINE_H

#include "ExecutionEngine.h"

class IFEngine : public ExecutionEngine{

public:
    IFEngine(Context &ctx, Instructions &instructions) :
            ExecutionEngine(ctx, instructions) {}

    void Start() override;
};


#endif //ARCHIHW1_IFENGINE_H
