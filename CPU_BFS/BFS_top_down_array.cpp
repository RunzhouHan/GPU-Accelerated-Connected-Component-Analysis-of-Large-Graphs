#include <iostream>
#include <stdio.h>
#include <set>
#include <string>
#include <queue>
#include <fstream>
#include <vector>
#include <sstream>
#include <strstream>
#include <algorithm>
#include <iterator>
#include <stdlib.h>


const int threads_per_block = 512;
const int blocks_per_grid = 65535;

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
		;
	}
	ReadFile.close();
	cout << temp << endl;
	return numoflines;
}

void RST(char *path, int len_per_line[], float rst[])
//build index array for crs
{
	ifstream ReadFile;
	string temp;
	int notzero = 0;
	int rstidx = 0;
	int lpl_idx = 0;
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
		len_per_line[lpl_idx] = notzero;
		notzero = 0;
		lpl_idx++;
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

void CRS(char *path, int len_per_line[], int csr[])
{
	ifstream ReadFile;
	string temp;
	int lpl_idx = 0; 
	ReadFile.open(path,ios::in);
    int result; 
    int i = 0; 
	while(getline(ReadFile,temp,'\n'))
	{	
		int temp2_len = len_per_line[lpl_idx];
		// cout << temp2_len << endl;
		int * temp2 = new int[temp2_len];
		lpl_idx++;
		stringstream input(temp);
		int idx = 0;
		while(input>>result)
		{	
			// cout << result << endl;
        	temp2[idx] = result;
        	// cout << temp2[idx] << endl;
        	idx++;
        }
    	for(int j = 1; j <= temp2_len; j++)
    		{	
    			// cout << temp2[j] << endl;
    			csr[i] = temp2[j];
    			i++;
    			// cout << temp2[j] << endl;
    			// cout << csr[i] << endl;
    		}
    	delete [] temp2;
    	// cout << '\n' << endl;
	}
}

int nextlen(int frontier[], float rst[], int csr[], int level[], int frontier_len)
{	
	int next_len = 0;

	for(int i = 0; i != frontier_len; i++)
	{	
		// cout << frontier[i] << endl;
		for(int j = rst[frontier[i]-1]; j != rst[frontier[i]]; j++)
		{
			// cout << level[csr[j]-1] << endl;

			if(level[csr[j]-1] == -1)
			{
				next_len++;
				// cout << next_len << endl;
			}
		}
	}
	// for(int i = 0; i != next.size(); i++)
	// {
	// 	cout << next[i] << endl;
	// }
	// cout << next_len << endl;
	return next_len;
}

void top_down(int *frontier, float rst[], int csr[], int next[], int level[] ,int gener, int next_len)
{	
	int k = 0;
	for(int i = 0; i != next_len; i++)
	{	
		for(int j = rst[frontier[i]-1]; j != rst[frontier[i]]; j++)
		{
			if(level[csr[j]-1] == -1)
			{
				level[csr[j]-1] = gener;
				next[k] = csr[j];
				k++;
//				pr[u] = v;
			}
		}
	}
	// for(int i = 0; i != next_len; i++)
	// {
	// 	cout << next[i] << endl;
	// }
}

void BFS(int * level, int len, int startpoint, float *rst, int csr[])
{
	int next_len = 1;
	int *frontier = new int[next_len];
	frontier[0] = startpoint;
    for(int i=0; i != len; i++)
    {
    	level[i] = -1;
    }
    level[startpoint-1] = 0;
    int gener = 1;
    // int t = 1;
    // int mf, nf, mu, n;
    // float CTB,CBT;
    // cout << gener << endl;
    // next_len = nextlen(frontier,rst,csr);
    // cout << next_len << endl;
    while(next_len != 0)
    {	
    	// int *next = (int *)malloc(next_len);
    	// cout << "1" << endl;
    	next_len = nextlen(frontier,rst,csr,level,next_len);
    	// cout << next_len << endl;
    	int *next = (int *)malloc(next_len);
    	top_down(frontier,rst,csr,next,level,gener,next_len);

    	// next = bottom_up(frontier,next,level,rst,csr,gener,allvertex);
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

    	delete [] frontier;
    	int *frontier = new int[next_len];
    	for(int i = 0; i != next_len; i++)
    		{
    			frontier[i] = next[i];
    		}
    	free(next);
    	gener++;
    }
    delete [] frontier;
}



int main()
{
	// cudaSetdDecice(0);
	int numoflines = CountLine("/Users/rzhan/Desktop/adjlist.txt");
	cout << "number of lines = " << numoflines << endl; 
	int rstlen = numoflines+1;
	float * rst = new float[rstlen];
	int *len_per_line = new int[numoflines];
	printf("RST");
	RST("/Users/rzhan/Desktop/adjlist.txt", len_per_line, rst);
	// for(int i = 0; i != rstlen; i++)
	// { 
	// 	cout << rst[i] << endl;
	// }
	int csrlen = rst[numoflines];
	int *csr = new int[csrlen];
	printf("CSR");
	CRS("/Users/rzhan/Desktop/adjlist.txt",len_per_line, csr);
	// for(int i = 0; i != csrlen; i++)
	// { 
	// 	cout << csr[i] << endl;
	// }
	int *allvertex = (int *)malloc(numoflines);
	for(int i = 0; i != numoflines; i++)
	{
		allvertex[i] = i+1;
		// cout << allvertex[i] << endl;
	}
    int * level = new int[numoflines];
    int startpoint=2;
    printf("asdf");
    BFS(level, numoflines, startpoint, rst, csr);
        printf("qwer");
    // delete [] rst;
    // delete [] len_per_line;
    // delete [] csr;
    // delete [] level;
 //    while(1)
 //    {
 //    cout << "give a start point: ";
 //    cin >>  startpoint;
 //    // cout << "give the parameter alpha: ";
 //    // cin >>  alpha;
 //    // cout << "give the parameter beta: ";
 //    // cin >>  beta;
 //    BFS(level, level_len, startpoint, rst, csr_, allvertex);
    for(int i=0; i != numoflines; i++)
    {
    	cout << "vertex " << allvertex[i] << " level:" << level[i] << endl;
    	// cout << "vertex " << allvertex[i] << " parent:" << level[i] << endl;
    }
	// }
}





