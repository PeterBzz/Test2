
#ifndef Timer_H
#define Timer_H

#include <boost/date_time.hpp>

using namespace std;

class Timer
{
	public:
		static void Wait_usec(useconds_t basePart, useconds_t randPart);
		static void Wait_msec(unsigned int basePart, unsigned int randPart);
		static void Wait_sec(unsigned int basePart, unsigned int randPart);
		static boost::posix_time::ptime GetOffsetedTime_usec(useconds_t basePart, useconds_t randPart);
		static boost::posix_time::ptime GetOffsetedTime_msec(unsigned int basePart, unsigned int randPart);
		static boost::posix_time::ptime GetOffsetedTime_sec(unsigned int basePart, unsigned int randPart);

	private:
		
};

#endif
