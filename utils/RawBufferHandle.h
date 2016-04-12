/*
 * Container that hold the pre-allocated native buffer
 * Similar to ByteBuffer in java.nio
 * */

#ifndef ARCHIHW1_RAWBUFFERHANDLE_H
#define ARCHIHW1_RAWBUFFERHANDLE_H

#include "../Types.h"
#include <utility>

template <class value_t>
class RawBufferHandle {

public:
    typedef std::function<void(RawBufferHandle<value_t>&)> release_callback_t;
    typedef const char* boundary_exception_t;

    static RawBufferHandle* Wrap(value_t* data,
                                 size_t data_size){
        auto handle = new RawBufferHandle<value_t>();
        handle->data = data;
        handle->buffer_size = data_size;
        return handle;
    };

    class iterator {

        friend RawBufferHandle<value_t>;

    private:
        size_t index;
        RawBufferHandle<value_t>* buffer_handle;

        iterator(RawBufferHandle<value_t>* buffer) :
                index(0),
                buffer_handle(buffer){}

    public:

        iterator() :
                index(0),
                buffer_handle(nullptr){}

        //Copy constructor
        iterator(iterator& that) :
                index(that.index),
                buffer_handle(that.buffer_handle){}

        //Hint: Must add const X& constructor to provide
        //X a(another_x_expression) initialization
        //Or it would emit error that only l-value is accepted
        iterator(const iterator& that) :
                index(that.index),
                buffer_handle(that.buffer_handle){}

        bool IsAccessible(){
            return !(index < 0  || buffer_handle == nullptr || index > buffer_handle->size());
        }

        //random access
        iterator operator+(size_t rhs){
            iterator tmp(*this);
            tmp.index = index + rhs;
            return tmp;
        }
        iterator& operator+=(int rhs){
            this->index += rhs;
            return *this;
        }

        iterator& operator++(){
            //++x
            this->index += 1;
            return *this;
        }
        iterator operator++(int n){
            //x++
            iterator tmp(*this);
            operator++();
            return tmp;
        }
        iterator& operator--(){
            //--x
            this->index -= 1;
            return *this;
        }
        iterator operator--(int n){
            //x--
            iterator tmp(*this);
            operator--();
            return tmp;
        }

        bool operator==(const iterator& rhs){ return index == rhs.index; }
        bool operator!=(const iterator& rhs){ return !(*this == rhs); }
        bool operator>=(const iterator& rhs){ return index >= rhs.index; }
        bool operator<=(const iterator& rhs){ return index <= rhs.index; }
        bool operator<(const iterator& rhs){ return !(*this >= rhs); }
        bool operator>(const iterator& rhs){ return !(*this <= rhs); }

        value_t& operator*(){
            if(!IsAccessible()) throw "Index Out Of Bound";
            return buffer_handle->data[index];
        }
        value_t* operator->(){
            if(!IsAccessible()) throw "Index Out Of Bound";
            return &(buffer_handle->data[index]);
        }
    };

private:
    release_callback_t release_callback;
    value_t* data;
    size_t buffer_size;

    //Base constructor
    RawBufferHandle() : data(nullptr) {}

public:

    value_t& operator[](size_t index){
        if(index < 0 || index > buffer_size) throw "Index out of bound";
        return *(data + index);
    }
    const value_t& operator[](size_t index) const {
        if(index < 0 || index > buffer_size) throw "Index out of bound";
        return const_cast<const value_t&>(*(data + index));
    }

    //Iterator
    iterator begin(){
        iterator tmp(this);
        return tmp;
    }
    iterator end(){
        iterator tmp(this);
        return tmp + buffer_size;
    }

    size_t size(){ return buffer_size; }

    value_t* content(){ return data; }
    const value_t* content() const { return const_cast<const value_t*>(data); }

    void SetReleaseCallback(release_callback_t/*By value*/ callback){
        release_callback = std::move(callback);
    }

    ~RawBufferHandle(){
        if(release_callback != nullptr){
            release_callback(*this);
        }
    }
};


#endif //ARCHIHW1_RAWBUFFERHANDLE_H
