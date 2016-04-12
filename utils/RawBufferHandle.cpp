
#include "RawBufferHandle.h"

template <class T>
//Hint: Need 'typename' keyword here
typename RawBufferHandle<T>::release_callback_t
        RawBufferHandle<T>::EmptyCallback = [](RawBufferHandle<T>& self)->void {};

/*
//TODO: Add to unit test
static RawBufferHandle<int>* dummy(){
    int buffer[5] = {1, 2, 3, 4, 5};
    auto bufferHandle = RawBufferHandle<int>::Wrap(buffer, 5);

    //Hint: Can neither use anonymous lambda nor auto declaration
    RawBufferHandle<int>::release_callback_t cb = [](RawBufferHandle<int>&)->void {
        puts("Destroy!!");
    };
    bufferHandle->SetReleaseCallback(cb);

    RawBufferHandle<int>::iterator b = (bufferHandle->begin());
    //RawBufferHandle<int,5>::iterator c;

    try{
        for(; b != bufferHandle->end(); b++){
            printf("%d ", *b);
        }
    }catch (RawBufferHandle<int>::boundary_exception_t e){
        puts("Exception:");
        puts(e);
    }
    puts("");

    return bufferHandle;
}

int main(){

    auto ret = dummy();
    delete ret;

    return 0;
}
 */


