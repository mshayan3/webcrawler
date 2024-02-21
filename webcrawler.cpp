#define CURL_STATICLIB
#include<iostream>
#include<fstream>
#include<string>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <curl/curl.h>
#include<unordered_set>
#include <unistd.h>
#include<pthread.h>
#include<semaphore.h>

using namespace std;
bool firstTime = true;
sem_t Semaphore;

string ProjectName;

unordered_set<string> newset;
unordered_set<string>::iterator mainiterator;

string baseurl;
string Domain_Name = "\0";
const string temp_outfilename = "/home/f200210/Desktop/project/" + ProjectName + "/" + ProjectName + "_HTML.txt";
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}
void* Crawling(void* args)
{
	CURL* curl;
	FILE* fp;
	CURLcode res;
	if (!firstTime)
	{
		sem_wait(&Semaphore);
	}
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\t---->> Downloading HTML File!";
	cout << "\n\t---->> Thread 1 is Executing!";
	cout << "\n\t-----------------------------------------\n";
	curl = curl_easy_init();
	if (curl)
	{
		fp = fopen(temp_outfilename.c_str(), "wb");
		curl_easy_setopt(curl, CURLOPT_URL, baseurl.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
	}
	if (!firstTime)
	{
		sem_post(&Semaphore);
	}
	return NULL;

}
void* display(void* args)
{
	if (!firstTime)
	{
		sem_wait(&Semaphore);
	}
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\t---->> Discovered Links";
	cout << "\n\t---->> Thread 4 is Executing!";
	cout << "\n\t-----------------------------------------\n";
	int i = 1;
	for (string itr : newset)
	{
		cout << "\n\t" << i << "--" << itr;
		i++;
	}
	cout << endl;
	sleep(3);
	if (!firstTime)
	{
		sem_post(&Semaphore);
	}
	return NULL;
}

void* insertincrawlqueue(void* args)
{
	if (!firstTime)
	{
		sem_wait(&Semaphore);
	}
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\t---->> Saving the Crawled Links in File!";
	cout << "\n\t---->> Thread 3 is Executing!";
	cout << "\n\t-----------------------------------------\n";
	ofstream myfile;
	myfile.open("/home/f200210/Desktop/project/" + ProjectName + "/" + ProjectName + "_crawled.txt", ios::app);
	myfile << *mainiterator;
	myfile << '\n';
	cout << "\n\t The Link Saved is :\n\t" << *mainiterator << endl;
	myfile.close();
	ifstream openfile;
	openfile.open("/home/f200210/Desktop/project/" + ProjectName + "/" + ProjectName + "_crawled.txt");
	string link;
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\t---->> Links in Crawl File!";
	cout << "\n\t-----------------------------------------\n\t";
	int i = 1;
	while (openfile.good())
	{
		getline(openfile, link);
		if (!(link == ""))
		{
			cout << "\n\t" << i << "--" << link;
			i++;
		}

	}
	sleep(3);
	if (!firstTime)
	{
		sem_post(&Semaphore);
	}
	return NULL;
}
void InsertinSet(string URLCreated)
{
	int i, j;
	for (i = 0, j = 0; URLCreated[i] != '\0' && Domain_Name[j] != '\0'; i++)
	{
		if (URLCreated[i] == Domain_Name[j])
		{
			j++;
		}
		else
		{
			j = 0;
		}
	}
	ofstream myfile;
	myfile.open("/home/f200210/Desktop/project/" + ProjectName + "/" + ProjectName + "_queue.txt", ios::app);

	if (j == Domain_Name.length())
	{
		newset.insert(URLCreated);
		myfile << URLCreated;
		myfile << '\n';
	}
	myfile.close();

}
void* CreateURL(string Urlread, int start)
{
	string URLCreated;
	while (Urlread[start] != '\"')
	{
		start++;
	}
	start++;
	while (Urlread[start] != '\"')
	{
		URLCreated.append(string(1, Urlread[start]));
		start++;
	}
	if (URLCreated[0] != 'h')
	{
		if (!isalpha(URLCreated[0]))
		{
			URLCreated.erase(0, 1);
			URLCreated = baseurl + URLCreated;
		}
		else
		{
			URLCreated = baseurl + URLCreated;
		}
	}
	InsertinSet(URLCreated);
	return NULL;
}
void* HTMLParse(void* args)
{
	ifstream ifile;
	string Urlread;
	string URLCreated;
	if (!firstTime)
	{
		sem_wait(&Semaphore);
	}
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\t---->> Parsing HTML to get Links!";
	cout << "\n\t---->> Thread 2 is Executing!";
	cout << "\n\t-----------------------------------------\n";
	ifile.open(temp_outfilename);
	if (!ifile.good())
	{
		cout << "\n\tFIle is Empty!\n";
		return NULL;
	}
	while (ifile.good())
	{
		getline(ifile, Urlread);
		if (Urlread != "")
		{
			for (int i = 0; i < Urlread.length() - 1; i++)
			{
				if (Urlread[i] == '<' && Urlread[i + 1] == 'a')
				{
					CreateURL(Urlread, i);
				}
			}
		}

	}
	sleep(5);
	if (!firstTime)
	{
		sem_post(&Semaphore);
	}
	return NULL;
}
bool DirectoryExists(const char* pzPath)
{
	if (pzPath == NULL) return false;

	DIR* pDir;
	bool bExists = false;

	pDir = opendir(pzPath);

	if (pDir != NULL)
	{
		bExists = true;
		(void)closedir(pDir);
	}

	return bExists;
}

void MakeFiles(string path)
{
	ofstream fileopen;
	fileopen.open(path + "/" + ProjectName + "_queue.txt");
	fileopen.close();
	fileopen.open(path + "/" + ProjectName + "_crawled.txt");
	fileopen.close();
	fileopen.open(path + "/" + ProjectName + "_HTML.txt");
	fileopen.close();
	cout << "\n\tProject Files Created!\n";
	cout << "\n\t-----------------------------------------\n";
}

void GetDomain()
{
	Domain_Name = "";
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\tGetting the Domain Name of to Be Crawled Web!\n\n";
	for (int i = 0; i < baseurl.length() - 1; i++)
	{
		while (baseurl[i] != '/' && baseurl[i + 1] != '/')
		{
			i++;
		}
		i++;
		while (baseurl[i] != '/')
		{
			Domain_Name.append(string(1, baseurl[i]));
			i++;
		}
	}
	cout << "\n\tThe Domain of to be Crawled Web is = " << Domain_Name << endl;
	cout << "\n\t-----------------------------------------\n";
	sleep(5);
}

void MakeDir()
{
	string path = "/home/f200210/Desktop/project/";
	path += ProjectName;
	if (!DirectoryExists(path.c_str()))
	{
		int n = mkdir(path.c_str(), 0777);
		cout << "\n\t-----------------------------------------\n";
		cout << "\n\tProject Directory Created!\n";
		MakeFiles(path);
	}
	else
	{
		cout << "\n\t-----------------------------------------\n";
		cout << "\n\tProject Directory already Exist!\n";
		cout << "\n\t-----------------------------------------\n";
	}
	cout << "\n\t-------->> Getting Program Initial Data\n";
	GetDomain();
	Crawling(NULL);
	HTMLParse(NULL);
	display(NULL);
	firstTime = false;
}

int main()
{
	cout << "\n\t-----------------------------------------\n";
	cout << "\n\t---->> Semaphore Created!";
	cout << "\n\t---->> Binary Type!";
	cout << "\n\t-----------------------------------------\n";
	sem_init(&Semaphore, 0, 1);
	int count = 1;
	cout << "\tPlease Enter the Project Name = ";
	cin >> ProjectName;
	cout << "\n\tPlease Enter the Base url to Crawl = ";
	cin >> baseurl;

	MakeDir();
	mainiterator = newset.begin();
	mainiterator++;
	pthread_t thread1, thread2, thread3, thread4;
	while (mainiterator != newset.end())
	{
		pthread_create(&thread1, NULL, &Crawling, NULL);
		pthread_create(&thread2, NULL, &HTMLParse, NULL);
		pthread_create(&thread3, NULL, &display, NULL);
		pthread_create(&thread4, NULL, &insertincrawlqueue, NULL);
		pthread_join(thread1, NULL);
		pthread_join(thread2, NULL);
		pthread_join(thread3, NULL);
		pthread_join(thread4, NULL);
		baseurl = *mainiterator;
		cout << "\n\t-----------------------------------------\n";
		cout << "\n\t---->> UpDated Base Link!";
		cout << "\n\tThe Base Url is = " << baseurl << endl;
		cout << "\n\t-----------------------------------------\n";

		mainiterator++;
	}

	return 0;
}
