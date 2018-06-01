
#include <vector>
#include <boost/date_time.hpp>
#include <sstream>
#include <mqueue.h>

#include "Debug/Debug.h"
#include "Contact/Contact.h"
#include "App/App.h"

using namespace std;

int main(int argc, char* argv[])
{
	
	srand(time(NULL));

	cout << "PID " << to_string(getpid()) << endl;

	App app = App::Singletone();
	app.ParseArgs(argc, argv);
	app.Start();

	/*
	vector<Contact> contacts;
	contacts.push_back(Contact(1, app.personalQueueNamePrefix,
		app.personalMessageMaxCount, app.personalMessageMaxSize,
		"qwerty"));
	contacts.push_back(Contact(3, app.personalQueueNamePrefix,
		app.personalMessageMaxCount, app.personalMessageMaxSize,
		"rwuiop"));
	contacts.push_back(Contact(2, app.personalQueueNamePrefix,
		app.personalMessageMaxCount, app.personalMessageMaxSize,
		"otywie"));
	int symbolsCount = App::CountSymbols(contacts);
	vector< vector<string> > copies = App::GetCopies(contacts, symbolsCount);
	vector<float> fullness = App::GetFullness(contacts, symbolsCount);
	int copiesCount = App::CountCopies(copies);
	int fullnessNotNominalCount = App::CountFullnessNotNominal(fullness, symbolsCount);
	*/

	//cin.get();
	return EXIT_SUCCESS;
}
