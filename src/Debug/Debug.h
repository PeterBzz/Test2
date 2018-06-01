
#ifndef Debug_H
#define Debug_H

#include <sstream>
#include <boost/date_time.hpp>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

class Debug
{
	public:
		static void Log(string text);

	private:
		int file;

		Debug();
		~Debug();
		
		static Debug& Singletone();
};

#endif
