// Copyright Runzhou Han 2018

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <sstream>
#include <strstream>
#include <algorithm>
#include <iterator>

using namespace std;


int CountLine(char *path)
//count lines of input adjacent list
{	
	ifstream ReadFile;
	int numoflines = 0;
	string temp;
	ReadFile.open(path,ios::in);
	while(getline(ReadFile,temp,'\n'))
	{
		numoflines++;
	}
	ReadFile.close();
	return numoflines;
}

void RST(char *path, float rst[10])
//build index array for crs
{
	ifstream ReadFile;
	string temp;
	int notzero = 0;
	int rstidx = 0;
	rst[0] = 0;
	ReadFile.open(path,ios::in);
	while(getline(ReadFile,temp,'\n'))
	{	
		rstidx++;
		for(string::size_type ix = 1; ix != temp.size(); ix++)
		{
			if(temp[ix] == ' ')
			{
				notzero++;
			}
		}
		rst[rstidx] = rst[rstidx-1] + notzero;
		// cout << temp << " size of line: " << notzero << endl;
		notzero = 0;
	}
	ReadFile.close();
}

int convertStringToInt(const string &s)
{
    int val;
    std::strstream ss;
    ss << s;
    ss >> val;
    return val;
}

std::vector <int> Entirelist(char *path)
//all the vortecis in a list
{
	ifstream ReadFile;
	string temp;
	ReadFile.open(path,ios::in);
	std::vector<string> temp2;
	std::vector<string> entirelist;
	std::vector<int> entirelist_int;
    string result;
	while(getline(ReadFile,temp,'\n'))
	{
		stringstream input(temp);
		while(input>>result)
        entirelist.push_back(result);
 	}
	for(int i = 0; i != entirelist.size(); i++)
	{ 
		entirelist_int.push_back(convertStringToInt(entirelist[i]));
	}
	sort(entirelist_int.begin(),entirelist_int.end());
	auto result2 = unique(entirelist_int.begin(),entirelist_int.end());
	entirelist_int.resize(distance(entirelist_int.begin(), result2));
	return entirelist_int;
}

std::vector <string> CRS(char *path)
{
	ifstream ReadFile;
	string temp;
	ReadFile.open(path,ios::in);
	std::vector<string> temp2;
	std::vector<string> crs;
    string result;  
	while(getline(ReadFile,temp,'\n'))
	{
		stringstream input(temp);
		while(input>>result)
        temp2.push_back(result);
    	for(int j = 1; j< temp2.size(); j++)
    		crs.push_back(temp2[j]);
    	temp2.clear();
	}
	// for(int i = 0; i != crs.size(); i++)
	// { 
	// 	cout << crs[i] << endl;
	// }
	return crs;
}

std::vector <int > top_down(std::vector<int> frontier, std::vector<int> next, int * level, float *rst, int csr[], int gener)
{	
	for(int i = 0; i != frontier.size(); i++)
	{	
		for(int j = rst[frontier[i]-1]; j != rst[frontier[i]]; j++)
		{
			if(level[csr[j]-1] == -1)
			{
				// level[csr[j]-1] = gener;
				level[csr[j]-1] = frontier[i];
				next.push_back(csr[j]);
			}
		}
	}
	// for(int i = 0; i != next.size(); i++)
	// {
	// 	cout << next[i] << endl;
	// }
	return next;
}

std::vector <int > bottom_up(std::vector<int> frontier, std::vector<int> next, int * level, float *rst, int csr[], int gener, std::vector <int> allvertex)
{
	for(int i =0; i != allvertex.size(); i++)
	{
		if(level[allvertex[i]-1] == -1)
		{
			for (int j = rst[allvertex[i]-1]; j != rst[allvertex[i]]; j++)
			{
				std::vector<int>::iterator flag;
				flag = find(frontier.begin(), frontier.end(), csr[j]);
				if (flag != frontier.end())
				{
					level[allvertex[i]-1] = csr[j];
					next.push_back(allvertex[i]);
					break;
				}
			}
		}
	}
	return next;
}


void BFS(int * level, int len, int startpoint, float *rst, int csr[], std::vector <int> allvertex, float alpha, float beta)
{
	std::vector <int > frontier;
	frontier.push_back(startpoint);
	std::vector <int > next;
    for(int i=0; i != len; i++)
    {
    	level[i] = -1;
    }
    level[startpoint-1] = 0;
    int gener = 1;
    // int t = 1;
    int mf, nf, mu, n;
    float CTB,CBT;
    while(!frontier.empty())
    {	
    	nf = frontier.size();
    	// next = top_down(frontier,next,level,rst,csr,gener);
    	next = bottom_up(frontier,next,level,rst,csr,gener,allvertex);
    	// cout << "layer " << t << endl;
    	// for(int m = 0; m != frontier.size(); m++ )
    	// {
    	// 	cout << frontier[m] << endl;
    	// }
 
    	// for(int m = 0; m != next.size(); m++ )
    	// {
    	// 	cout << next[m] << endl;
    	// }
    	// cout << "layer " << t << endl;
    	// t++ ;

    	frontier.clear();
    	for(int i = 0; i != next.size(); i++)
    		{
    			frontier.push_back(next[i]);
    		}
    	next.clear();
    	gener++;
    }
}

int main ()
{
	int numoflines = CountLine("/Users/rzhan/Desktop/adjlist.txt");
	cout << "number of lines = " << numoflines << endl; 
	int rstlen = numoflines+1;
	float * rst = new float[rstlen];
	RST("/Users/rzhan/Desktop/adjlist.txt", rst);
	rst[rstlen] = rst[numoflines] +1;
	// for( int i = 0; i <= rstlen; i++)
	// {
	// 	cout << rst[i] << endl;
	// }
	int crslen = rst[rstlen];
	std::vector <string> csr;
	csr = CRS("/Users/rzhan/Desktop/adjlist.txt");
	// for(int i = 0; i != csr.size(); i++)
	// { 
	// 	cout << csr[i] << endl;
	// }
	int csr_size = csr.size()+1;
	int csr_[csr_size];
	csr_[csr_size-1] = 0;
    for(int i=0;i<csr.size();i++)
    {
    	csr_[i] = convertStringToInt(csr[i]);
    }
    // for(int i=0;i<csr_size;i++)
    // {
    // 	cout << csr_[i] << endl;
    // }
	std::vector <int> allvertex;
	allvertex = Entirelist("/Users/rzhan/Desktop/adjlist.txt");
	// for(int i=0; i<allvertex.size(); i++)
 //    {
 //    	cout << allvertex[i] << endl;
 //    }
    int level_len = allvertex.size();
    int * level = new int[level_len];
    int startpoint;
    float alpha;
    float beta;
    while(1)
    {
    cout << "give a start point: ";
    cin >>  startpoint;
    cout << "give the parameter alpha: ";
    cin >>  alpha;
    cout << "give the parameter beta: ";
    cin >>  beta;
    BFS(level, level_len, startpoint, rst, csr_, allvertex);
    for(int i=0; i != level_len; i++)
    {
    	// cout << "vertex " << allvertex[i] << " level:" << level[i] << endl;
    	cout << "vertex " << allvertex[i] << " parent:" << level[i] << endl;
    }
	}
}




