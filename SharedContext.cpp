#include "SharedContext.hpp"

#include <signal.h>
#include <errno.h>

bool isProcessAlive(pid_t pid) {
    if (pid < 1) {
        return false;
    }

    if (kill(pid, 0) == 0) {
        return true;  // Process exists
    } else {
        if (errno == ESRCH) {
            return false;  // Process does not exist
        }
        // Handle other errors like EPERM
        return false;
    }
}

key_t SharedContext::get_shared_memory_key() {
    return ftok("/tmp", PROJECT_ID);
}

void SharedContext::bind_shared_memory() {
    int segment_id;
    key_t key = get_shared_memory_key();

    bool shB = true, smB = true;
    segment_id = shmget(key, sizeof(SharedMemoryBuffer), S_IRUSR | S_IWUSR);
    if (segment_id == -1) {
        shB = false;
    }

    shared_memory = (SharedMemoryBuffer*)shmat(segment_id, NULL, 0);
    if (shared_memory == (void*) -1) {
        smB = false;
    }

    state_ |= (uint8_t)(shB && smB);
}

void SharedContext::threadTask(){
    console->info("thread started!");
    const SharedMemoryBuffer* context = get_context();
    while(!abortThread_){
        if(context == nullptr){
            // console->critical("shared_memory dropped!");
            state_ |= STATE_NULL_REF;
        } else {
            state_ &= ~STATE_NULL_REF;
        }

        if(!(isProcessAlive(context->procid))){
            state_ &= ~STATE_VALID;
        }else{
            state_ |= STATE_VALID;
        }

        if((getTs() - context->ts) > 1000 && !timedOut_){
            // console->info("connection to cinepi raw timed out!");
            state_ |= STATE_TIMEOUT;
            timedOut_ = true;
        } else {
            state_ &= ~STATE_TIMEOUT;
        }

        if(timedOut_ && (lastProcId_ != context->procid)){
            timedOut_ = false;
        }

        lastProcId_ = context->procid;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
