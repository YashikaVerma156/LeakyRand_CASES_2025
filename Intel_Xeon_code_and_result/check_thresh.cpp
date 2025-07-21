#include <bits/stdc++.h>
#include <sstream>
using namespace std;

// The value of thresold to be checked 
#define THRESHOLD 200

// The value of recorded time to be ignored as outlier (possibly due to context switch)
#define OUTLIER 10000

int main(){

	// vector to store data read from the file
	// if second element of the pair is 1, then it is a miss time, else it is a hit time
	vector<pair<int,int>> a;

	// internal variables
	string s, xs, ys;
	int x,y;
	char c;

	// Read the data from the file line by line and fill it in the vector a
	ifstream data("data.csv");
	while(getline(data,s)){
		stringstream str(s);
		getline(str, xs, ',');
		getline(str, ys);
		x = stoi(xs);
		y = stoi(ys);
		a.push_back({x,y});
	}

	// Check if the data is read correctly
	cout<<a.size()<<endl;

	cout << "Threshold = " << THRESHOLD << endl;
	cout << "Outlier Boundary = " << OUTLIER << endl;
	cout << endl;

	/*---------------------------Using the Outlier check on data points---------------------------*/

	// initialize variables
	long num_hits_wrong = 0, num_hits_right = 0;
	long num_miss_wrong = 0, num_miss_right = 0;
	for(int i=0; i<(int)a.size(); i++){
		if(a[i].second == 0){ // Time measured for Hit 
			if(a[i].first <= THRESHOLD) num_hits_right++; // Classified Hit
			else num_hits_wrong++; // Classified Miss
		}
		else{ // Time measured for Miss
			if(a[i].first <= THRESHOLD) num_miss_wrong++; // Classified Hit
			else num_miss_right++; // Classified Miss
		}
	}

	// find the percentage of hits and misses correctly classified
	double hitp=0, missp=0;
	hitp = ((double)(num_hits_right))/(num_hits_right+num_hits_wrong);	
	missp = ((double)(num_miss_right))/(num_miss_right+num_miss_wrong);	
	cout << "Without ignoring Outliers...."<<endl;
	cout << "Actual Hit, Reported Hit   = "<<num_hits_right<<endl;
	cout << "Actual Hit, Reported Miss  = "<<num_hits_wrong<<endl;
	cout << "Actual Miss, Reported Miss = "<<num_miss_right<<endl;
	cout << "Actual Miss, Reported Hit  = "<<num_miss_wrong<<endl;
	cout << "Hit correctly classified =   " << 100*hitp << "%" << endl;
	cout << "Miss correctly classified =  " << 100*missp << "%" << endl;
	cout<<endl;

	/*------------------------------Ignoring the Outlier check on data points------------------------*/

	// initialize variables
	num_hits_wrong = 0, num_hits_right = 0;
	num_miss_wrong = 0, num_miss_right = 0;
	for(int i=0; i<(int)a.size(); i++){
		if(a[i].first > OUTLIER) continue;
		if(a[i].second == 0){ // Time measured for Hit 
			if(a[i].first <= THRESHOLD) num_hits_right++; // Classified Hit
			else num_hits_wrong++; // Classified Miss
		}
		else{ // Time measured for Miss
			if(a[i].first <= THRESHOLD) num_miss_wrong++; // Classified Hit
			else num_miss_right++; // Classified Miss
		}
	}

	// find the percentage of hits and misses correctly classified
	hitp=0, missp=0;
	hitp = ((double)(num_hits_right))/(num_hits_right+num_hits_wrong);	
	missp = ((double)(num_miss_right))/(num_miss_right+num_miss_wrong);	
	cout << "Ignoring Outliers Results...."<<endl;
	cout << "Actual Hit, Reported Hit   = "<<num_hits_right<<endl;
	cout << "Actual Hit, Reported Miss  = "<<num_hits_wrong<<endl;
	cout << "Actual Miss, Reported Miss = "<<num_miss_right<<endl;
	cout << "Actual Miss, Reported Hit  = "<<num_miss_wrong<<endl;
	cout << "Hit correctly classified =   " << 100*hitp << "%" << endl;
	cout << "Miss correctly classified =  " << 100*missp << "%" << endl;
	return 0;
}
