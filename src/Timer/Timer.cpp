
#include "Timer.h"

void Timer::Wait_usec(useconds_t basePart, useconds_t randPart)
{
    usleep(rand() % randPart + basePart);
}
void Timer::Wait_msec(unsigned int basePart, unsigned int randPart)
{
    usleep((rand() % randPart + basePart) * 1e3);
}
void Timer::Wait_sec(unsigned int basePart, unsigned int randPart)
{
    sleep(rand() % randPart + basePart);
}
boost::posix_time::ptime Timer::GetOffsetedTime_usec(useconds_t basePart, useconds_t randPart)
{
    return boost::posix_time::microsec_clock::universal_time() +
        boost::posix_time::microsec(rand() % randPart + basePart);
}
boost::posix_time::ptime Timer::GetOffsetedTime_msec(unsigned int basePart, unsigned int randPart)
{
    return boost::posix_time::microsec_clock::universal_time() +
        boost::posix_time::millisec(rand() % randPart + basePart);
}
boost::posix_time::ptime Timer::GetOffsetedTime_sec(unsigned int basePart, unsigned int randPart)
{
    return boost::posix_time::microsec_clock::universal_time() +
        boost::posix_time::seconds(rand() % randPart + basePart);
}
