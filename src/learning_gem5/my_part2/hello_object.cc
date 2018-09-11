#include "learning_gem5/my_part2/hello_object.hh"

#include "debug/Hello.hh"

HelloObject::HelloObject(HelloObjectParams *params) :
    SimObject(params),
    event(*this),
    myName(params->name),
    latency(params->time_to_wait),
    timesLeft(params->number_of_fires)
{
    DPRINTF(Hello, "Created the hello object with the name %s\n", myName);
}

void
HelloObject::processEvent()
{
    timesLeft--;
    DPRINTF(Hello, "Hello world! Processing the event! %d left\n", timesLeft);

    if (timesLeft <= 0) {
        DPRINTF(Hello, "Done firing!\n");
    } else {
        schedule(event, curTick() + latency);
    }
}

void
HelloObject::startup()
{
    schedule(event, latency);
}

HelloObject*
HelloObjectParams::create()
{
    return new HelloObject(this);
}
