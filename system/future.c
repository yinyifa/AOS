/* future.c - future */

#include <xinu.h>

struct my_queue* init_my_queue(uint capacity)
{
    struct my_queue* result;
    result = (struct my_queue*)getmem(sizeof(struct my_queue));
    result->capacity = capacity;
    result->length = 0;
    result->read_top = 0;
    result->write_top = 0;
    result->the_queue = (pid32*) getmem(sizeof(pid32) * capacity);

    return result;
}

void my_enqueue(struct my_queue* queue, pid32 item)
{
    queue->the_queue[queue->write_top++] = item;
    if (queue->write_top >= queue->capacity){
        queue->write_top = 0;
    }
    queue->length++;
}

pid32 my_dequeue(struct my_queue* queue)
{
    pid32 result = queue->the_queue[queue->read_top++];
    if (queue->read_top >= queue->capacity){
        queue->read_top = 0;
    }
    queue->length--;
    return result;
}

future_t* future_alloc(future_mode_t mode, uint size, uint nelems){
        intmask	mask;			/* Saved interrupt mask		*/
        future_t* result;
        
        mask = disable();

        result = (future_t*) getmem(sizeof(future_t));
        result->data = getmem(size * nelems);
        result->size = size;
        result->mode = mode;
        result->state = FUTURE_EMPTY;
        result->max_elems = nelems;
        result->count = 0;
        result->head = 0;
        result->tail = 0;
        result->num_consumed = 0;

        // initialize and allocate memory for get_queue and set_queue
        result->get_queue = init_my_queue(32);
        result->set_queue = init_my_queue(32);
        
        restore(mask);
        return result;
}

syscall future_free(future_t* mfuture){
    intmask mask;
    syscall result;

    mask = disable();

    if (freemem((char*) (mfuture->get_queue)->the_queue, sizeof(pid32) * (mfuture->get_queue)->capacity) < 1){
        restore(mask);
        return SYSERR;
    }

    if (freemem((char*) mfuture->get_queue, sizeof(struct my_queue)) < 1){
        restore(mask);
        return SYSERR;
    }

     if (freemem((char*) (mfuture->set_queue)->the_queue, sizeof(pid32) * (mfuture->set_queue)->capacity) < 1){
        restore(mask);
        return SYSERR;
    }

    if (freemem((char*) mfuture->set_queue, sizeof(struct my_queue)) < 1){
        restore(mask);
        return SYSERR;
    }

    if (freemem(mfuture->data, (mfuture->size) * (mfuture->max_elems)) < 1){
        restore(mask);
        return SYSERR;
    }

    if (freemem((char* ) mfuture, sizeof(future_t)) < 1){
        restore(mask);
        return SYSERR;
    }
    restore(mask);
    return OK;
}

syscall future_get(future_t* mfuture, char* m_out){
    intmask mask;
    mask = disable();
   
    if (mfuture->mode == FUTURE_EXCLUSIVE)
    {
        if (mfuture->state == FUTURE_READY){
           
            memcpy(m_out, mfuture->data, mfuture->size);
            mfuture->state = FUTURE_EMPTY;
            restore(mask);
            return OK;
        }
        else if (mfuture->state == FUTURE_EMPTY){
            mfuture -> state = FUTURE_WAITING;
            mfuture -> pid = getpid();
            suspend(mfuture->pid);

            memcpy(m_out, mfuture->data, mfuture->size);
            mfuture->state = FUTURE_EMPTY;
            restore(mask);
            return OK;  
        }
        else if (mfuture->state == FUTURE_WAITING){
            restore(mask);
            return SYSERR;
        }
    }

    else if (mfuture->mode == FUTURE_SHARED)
    {
        if (mfuture->state == FUTURE_READY){
           
            memcpy(m_out, mfuture->data, mfuture->size);
            restore(mask);
            return OK;
        }
        else if (mfuture->state == FUTURE_EMPTY)
        {
            mfuture->state = FUTURE_WAITING;
            mfuture -> pid = getpid();
            
            // add process to get_queue
            my_enqueue(mfuture->get_queue, mfuture->pid);
            suspend(mfuture->pid);

            memcpy(m_out, mfuture->data, mfuture->size);
            restore(mask);
            return OK;  

        }
        else if (mfuture->state == FUTURE_WAITING)
        {
            mfuture -> pid = getpid();
            
            // add process to get_queue
            my_enqueue(mfuture->get_queue, mfuture->pid);
            suspend(mfuture->pid);

            memcpy(m_out, mfuture->data, mfuture->size);
            restore(mask);
            return OK;  
        }

    }

    else if(mfuture->mode == FUTURE_QUEUE){
        while (mfuture->count == 0){
            my_enqueue(mfuture->get_queue, getpid());
            suspend(getpid());
        }
        char* headElemPtr = mfuture->data + (mfuture->head * mfuture->size);
        memcpy(m_out, headElemPtr, mfuture->size);
        mfuture->count--;
        mfuture->head++;
        if (mfuture->head >= mfuture->max_elems){
            mfuture->head = 0;
        }
        if (mfuture->set_queue->length > 0)
            resume(my_dequeue(mfuture->set_queue));
        restore(mask);
        return OK;
    }
    
    restore(mask);
    return OK;
}

syscall future_set(future_t* mfuture, char* m_in){
    intmask mask;    
    mask = disable();
    
    if (mfuture->state == FUTURE_WAITING)
    {   
        memcpy(mfuture->data, m_in, mfuture->size);
        mfuture->state = FUTURE_READY;

        if (mfuture->mode == FUTURE_EXCLUSIVE)
        {
            resume(mfuture->pid);
            restore(mask);
            return OK;
        }
        else if (mfuture->mode == FUTURE_SHARED)
        {
            int i;
            int num_waitings = (mfuture->get_queue)->length;

            for (i=0; i < num_waitings; i++)
            {   
                resume(my_dequeue(mfuture->get_queue));
            }
            restore(mask);
            return OK;
        }

    }
    else if(mfuture->state == FUTURE_EMPTY)
    {   
        if (mfuture->mode != FUTURE_QUEUE){
            memcpy(mfuture->data, m_in, mfuture->size);
            mfuture->state = FUTURE_READY;
            restore(mask);
            return OK;
        }else{
            while (mfuture->count >= mfuture->max_elems){
                my_enqueue(mfuture->set_queue, getpid());
                suspend(getpid());
            }
            char* tailElemPtr = mfuture->data + (mfuture->tail * mfuture->size);
            memcpy(tailElemPtr, m_in, mfuture->size);
            mfuture->count++;
            mfuture->tail++;
            if (mfuture->tail >= mfuture->max_elems){
                mfuture->tail = 0;
            }
            if (mfuture->get_queue->length > 0)
                resume(my_dequeue(mfuture->get_queue));

            restore(mask);
            return(OK);
        }
    }
    else
    {
        restore(mask);
        return SYSERR;
    }
    
    restore(mask);
    return SYSERR;

}


