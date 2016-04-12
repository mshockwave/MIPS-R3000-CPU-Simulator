
#include "RawBufferHandle.h"

template <class T, size_t N>
//Hint: Need 'typename' keyword here
typename RawBufferHandle<T, N>::release_callback_t
        RawBufferHandle<T,N>::EmptyCallback = [](RawBufferHandle<T,N>& self)->void {};

/*
static RawBufferHandle<int,5>* dummy(){
    int buffer[5] = {1, 2, 3, 4, 5};
    auto bufferHandle = RawBufferHandle<int,5>::Wrap(buffer);

    //Hint: Can neither use anonymous lambda nor auto declaration
    RawBufferHandle<int,5>::release_callback_t cb = [](RawBufferHandle<int,5>&)->void {
        puts("Destroy!!");
    };
    bufferHandle->SetReleaseCallback(cb);

    RawBufferHandle<int,5>::iterator b = (bufferHandle->begin());
    //RawBufferHandle<int,5>::iterator c;

    try{
        for(; b != bufferHandle->end(); b++){
            printf("%d ", *b);
        }
    }catch (RawBufferHandle<int,5>::boundary_exception_t e){
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


