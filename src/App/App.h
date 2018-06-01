
#ifndef App_H
#define App_H

#include <vector>
#include <boost/date_time.hpp>
#include <sstream>
#include <mqueue.h>

#include "../Debug/Debug.h"
#include "../Message/Message.h"
#include "../Contact/Contact.h"
#include "../Timer/Timer.h"
#include "../State/State.h"

using namespace std;

class App
{
	public:
		const string lockQueueName = "/queue_test2_lock";
		const string globalQueueName = "/queue_test2_global";
		const string personalQueueNamePrefix = "/queue_test2_personal_";
		const int lockMessageMaxSize = 100;
		const int lockMessageMaxCount = 10;
		const int globalMessageMaxSize = 100;
		const int globalMessageMaxCount = 10;
		const int personalMessageMaxSize = 1000;
		const int personalMessageMaxCount = 10;
		const int retryTimeBase_ms = 100;
    	const int retryTimeRand_ms = 100;
		const int waitBeforeLogicsTimeBase_ms = 5000;
    	const int waitBeforeLogicsTimeRand_ms = 5000;

		vector<Contact> contacts;
		Contact self;
		bool hasMarker;
		bool connected;
		mqd_t lockQueue;
		mqd_t globalQueue;
		mqd_t personalQueue;
		bool hasLock;
		string personalQueueName;
		//string symbols;
		boost::posix_time::ptime logicsStartTime;
		bool finishing;

		App();
		~App();
		void GetLock();
		void FreeLock();
		void Start();
		void ParseArgs(int argc, char* argv[]);
		bool MergeContacts(Contact contact);
		bool MergeContacts(vector<Contact> contacts);
		void ConnectWithPrevious();
		void Listen();
		State GetState();
		void LogicsIteration();
		int GetSelfIndex();
		void SendUpdatedContacts();
		void SendUpdatedContacts(long excludedPid);

		static string RemoveCopiesInSelf(string str);
		static int CountSymbols(vector<Contact> contacts);
		static string FindCopies(string source, string destination);
		static vector< vector<string> > GetCopies(vector<Contact> contacts, int symbolsCount);
		static int CountCopies(vector< vector<string> > copies);
		static vector<float> GetFullness(vector<Contact> contacts, int symbolsCount);
		static int CountFullnessNotNominal(vector<float> fullness, int symbolsCount);
		static string RemoveCopies(string copies, string str);

		static App& Singletone();
};

#endif
