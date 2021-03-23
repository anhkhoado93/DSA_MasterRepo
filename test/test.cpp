#include<iostream>
#include<fstream>
#include<regex>


using namespace std;

int main() 
{
    fstream fs;
    fs.open("prog1.txt", ios::in);
    string str;
    // regex re("\\s+");
    // while(getline(fs, str))
    // {
    //     regex_token_iterator<string::iterator> first(str.begin(), str.end(), re, -1);
    //     regex_token_iterator<string::iterator> end;
    //     cout<< "token extract: ";
    //     while (first != end) cout<< *first++ << "      ";
    //     cout<<endl;
    // }
    while(getline(fs, str))
    {
        
    }
    fs.close();

}