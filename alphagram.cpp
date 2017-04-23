#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>

using namespace std;

// Takes one line at a time and converts to uppercase letters and print out alphagram
int main()
{
	string input;
	
	while (getline(cin, input)) {
		transform(input.begin(), input.end(), input.begin(), ::toupper);	
		sort(input.begin(), input.end());
		
		cout << input << endl;
	}
	
	return 0;
}