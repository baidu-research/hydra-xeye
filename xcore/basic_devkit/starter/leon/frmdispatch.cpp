
#include "frmdispatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void PlgFrmDispatch::Create(){
    inFrame.Create(16);

    Add(&inFrame , ".inFrame");
    Add(&outF_even, ".outF_even");
    Add(&outF_odd, ".outF_odd");
}

void* PlgFrmDispatch::threadFunc(void *ctx)
{
    rtems_object_set_name(rtems_task_self(), "frame_dispatch_thread");
    UNUSED(ctx);
    int err = 0;

    while(1)
    {
        ImgFramePtr inF;
        err = inFrame.Receive(&inF);
        if(!err)
        {
            if(inF.ptr->seqNo % 2 == 0)
            {
                outF_even.Send(&inF);
            } else {
                outF_odd.Send(&inF);
            }
        }
    }
    return NULL;
}

void PlgFrmSync::Create(){
    sel = 0;
    inFrame_even.Create(4);
    inFrame_odd.Create(4);

    Add(&inFrame_even , ".inFrame_even");
    Add(&inFrame_odd, ".inFrame_odd");
    Add(&out, ".out");
}

void* PlgFrmSync::threadFunc(void *ctx)
{
    rtems_object_set_name(rtems_task_self(), "frame_syncout_thread");
    UNUSED(ctx);

    while(1)
    {
        ImgFramePtr inF;
        if (sel == 0)
        {
            inFrame_even.Receive(&inF);
            sel = 1;
        } else {
            inFrame_odd.Receive(&inF);
            sel = 0;
        }
        out.Send(&inF);
    }
    return NULL;
}
