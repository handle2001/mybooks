#include "book.h"

using namespace std;

int main( int argc, char **argv ){
	std::string bU = "http://openlibrary.org/api/books?format=json&jscmd=data&bibkeys=ISBN:";
	OLBook book(bU);
	do{
		std::string yn = "y";
		book.makeBook();
		std::cout<<"Lookup another book?(y/n)"<<endl;
		std::cin>>yn;
		std::cin.ignore();
		if(yn == "n"){break;}
	}while(1);
}
