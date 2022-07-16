#include "dpc.h"

struct __declspec(novtable) iexample
{
    virtual void hello(void*) = 0;
    virtual void goodbye() = 0;
};

void dpc_capture_t<iexample>::install_default_vtbl()
{
    install_dpc_func(&iexample::hello);
    install_dpc_func(&iexample::goodbye);
}

struct cexample
{
    void hello(unsigned char* a)
    {
        printf("%s", a);
    }

    void goodbye()
    {
        printf(";");
    }
};

int main(int argc, char* argv[])
{
    dpc_playback_t<iexample, cexample> dpc_playback;
    dpc_capture_t<iexample> dpc_capture(1024);
    cexample ci;

    dpc_playback.install_playback_func(&iexample::hello, &cexample::hello);
    dpc_playback.install_playback_func(&iexample::goodbye, &cexample::goodbye);

    dpc_capture->hello("alice");
    dpc_capture->goodbye();
    dpc_capture->hello("bob");
    dpc_capture->goodbye();

    dpc_playback.replay_capture(&ci, dpc_capture);
}
