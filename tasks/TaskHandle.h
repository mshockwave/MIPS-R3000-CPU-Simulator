
#ifndef ARCHIHW1_TASKBASE_H
#define ARCHIHW1_TASKBASE_H

#include "../Types.h"
#include "../Instruction.h"

#include <string>
#include <functional>

#include <boost/thread/barrier.hpp>

#define STAGE_TASK() \
    [](TaskHandle* self)->Error
#define RISING_EDGE_FENCE() \
    self->clock->rising_edge.wait()

class TaskHandle {

public:

    struct ClockHandle{
        typedef boost::barrier Barrier;
        Barrier &rising_edge, &falling_edge;
        ClockHandle(Barrier& re, Barrier& fe) :
                rising_edge(re),
                falling_edge(fe){}
    };

    Context* context;
    Instruction* instruction;
    const ClockHandle* clock;
    std::string name;
    task_id_t task_id;

    //Registers index cache
    uint8_t RtIndex, RsIndex, RdIndex;
    //Registers value storage
    reg_t RtValue, RsValue, RdValue;

    //Information for dumping
    bool ForwardRt, ForwardRs;

    typedef std::function<Error(TaskHandle*)> stage_task_t;

    TaskHandle() :
            instruction(nullptr),
            context(nullptr),
            RtIndex(0), RsIndex(0), RdIndex(0),
            ForwardRs(false), ForwardRt(false){}

    TaskHandle* Get(Context* ctx, Instruction* instr, const ClockHandle* clk){
        auto* task_handle = new TaskHandle();
        task_handle->context = ctx;
        task_handle->instruction = instr;
        task_handle->clock = clk;
        task_handle->name = name;

        task_handle->stage_if = stage_if;
        task_handle->stage_id = stage_id;
        task_handle->stage_ex = stage_ex;
        task_handle->stage_dm = stage_dm;
        task_handle->stage_wb = stage_wb;

        return task_handle;
    }

    TaskHandle& Name(std::string name_, task_id_t id){
        name = name_;
        task_id = id;
        return *this;
    }

    TaskHandle& IF(stage_task_t t){
        stage_if = t;
        return *this;
    }
    Error DoIF(){
        if(stage_if){
            return stage_if(this);
        }else{
            return Error::NONE;
        }
    }

    TaskHandle& ID(stage_task_t t){
        stage_id = t;
        return *this;
    }
    Error DoID(){
        if(stage_id){
            return stage_id(this);
        }else{
            return Error::NONE;
        }
    }

    TaskHandle& EX(stage_task_t t){
        stage_ex = t;
        return *this;
    }
    Error DoEX(){
        if(stage_ex){
            return stage_ex(this);
        }else{
            return Error::NONE;
        }
    }

    TaskHandle& DM(stage_task_t t){
        stage_dm = t;
        return *this;
    }
    Error DoDM(){
        if(stage_dm){
            return stage_dm(this);
        }else{
            return Error::NONE;
        }
    }

    TaskHandle& WB(stage_task_t t){
        stage_wb = t;
        return *this;
    }
    Error DoWB(){
        if(stage_wb){
            return stage_wb(this);
        }else{
            return Error::NONE;
        }
    }

private:
    stage_task_t stage_if, stage_id, stage_ex, stage_dm, stage_wb;
};

#endif //ARCHIHW1_TASKBASE_H
