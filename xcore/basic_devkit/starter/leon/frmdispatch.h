#ifndef FRM_DISPATCH_H__
#define FRM_DISPATCH_H__
#include <Flic.h>
#include <Sender.h>
#include <Receiver.h>
#include <ImgFrame.h>

class PlgFrmDispatch : public ThreadedPlugin{
  public:
   SReceiver<ImgFramePtr> inFrame;

   MSender  <ImgFramePtr> outF_even;
   MSender  <ImgFramePtr> outF_odd;

   void Create();
   void *threadFunc(void *);
};

class PlgFrmSync : public ThreadedPlugin{
  public:
   SReceiver<ImgFramePtr> inFrame_even;
   SReceiver<ImgFramePtr> inFrame_odd;

   MSender  <ImgFramePtr> out;

   int sel;

   void Create();
   void *threadFunc(void *);
};
#endif
