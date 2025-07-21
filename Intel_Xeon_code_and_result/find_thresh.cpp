#include <bits/stdc++.h>
#include <sstream>
using namespace std;

int main(){

	// vector to store the data read from the csv file
	// if second element of the pair is 1, then it is a miss time, else it is a hit time
	vector<pair<int,int>> a;

	// internal variables
	string s, xs, ys;
	int x,y;
	char c;
	
	// read and parse the csv file line by line and fill it in the vector a
	ifstream data("data.csv");
	while(getline(data,s)){
		stringstream str(s);
		getline(str, xs, ',');
		getline(str, ys);
		x = stoi(xs);
		y = stoi(ys);
		a.push_back({x,y});
	}

	// check if the data is read correctly
	cout<<a.size()<<endl;

	// sort the data based on the first element (time value) of the pair
	sort(a.begin(), a.end());

	// vector to store the optimal threshold values
	vector<int> v;

	// initialize variables
	long minerr = 2e8;
	x = 0, y = 1e8;
	int minHL = 2e8;
	int maxML = 0;

	// loop through the data to find the minimum hit time and maximum miss time
	for(int i=0; i<(int)a.size(); i++){
		minHL = min(a[i].first, minHL);
		maxML = max(a[i].first, maxML);
	}
	cout << "minHL = " << minHL << " maxML = " << maxML << endl;

	// sweep through the data to find the optimal threshold value (which minimizes the error)
	for(int i=minHL; i<=maxML; i++){

		// assume that the threshold value is i
		long err = 0; // initialize error to 0

		// loop through the data to calculate the error on this threshold value
		for(int j=0; j<(int)a.size(); j++){
			if(a[j].first <= i && a[j].second == 1)	err++; // data point was a miss time but it is less than equal to the threshold value
			if(a[j].first > i && a[j].second == 0)	err++; // data point was a hit time but it is greater than the threshold value
		}

		// check if the error is less than the minimum error found so far
		if(minerr > err){
			// update the minimum error
			minerr = err;

			// clear the vector and add the new optimal threshold value
			v.clear();
			v.push_back(i);
		}

		else if(minerr == err){
			// if the error is same as the minimum error found so far, add this threshold value to the vector
			v.push_back(i);
		}

		// Debugging print
		// cout << "i = " << i << " minerr = " << minerr << " err = " << err << endl;

		// case when the minimum error threshold is past and now on the errors will only increase 
		if(err > 100*minerr) break;
	}

	// print the minimum error and the optimal threshold values
	cout << minerr << endl;
	for(auto &e:v) cout << e <<" ";
	cout << endl;
	return 0;
}
