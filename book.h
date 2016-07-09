#include <iostream>
#include <sstream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <jsoncpp/json/json.h>
#include <string>
#include <sqlite_modern_cpp.h>

class Book {

	//protected attributes
  protected:
  	std::string baseURL;
  	std::string URL;
  	std::string ISBN;
  	Json::Value INFO;
	std::string title;
	std::string authors;
	std::string published;
	std::string subject;
	std::string isbn10;
	std::string isbn13;
	float price;

	//public methods
  public:
  	void makeBook();
  	virtual void putInfo() /*{std::cout<<"Failed to override function!"<<std::endl;}*/ =0;  	
  	void getISBN();
  	void getJSON();
  	void printBook();
  	bool storeBook();
  
  	Book(std::string bU);
  	~Book();

	
};

Book::Book(std::string bU){
	baseURL = bU;
}
	
Book::~Book(){
	//TODO: close cURLpp, JSON, and sqlite resources cleanly
}

void Book::getISBN(){
	std::string input;
	std::cout<<"\e[1m\e[33mEnter the ISBN\e[0m: "<<std::endl;
	std::cin>>input;
	std::cout<<"\e[1m\e[33mYou entered\e[0m: "
		<<"\e[31m"<<input<<"\e[0m"
		<<std::endl;
	this->ISBN = input;
}

void Book::getJSON(){
	this->URL = this->baseURL + this->ISBN;
	//Initialize JSONpp
	Json::Value root;
	//Initialize JSONpp reader
	Json::Reader reader;
	//Create string stream
	std::ostringstream os;
	//Initialize cURLpp
	curlpp::Cleanup cleanup;
	curlpp::Easy c;
	//Give the cURLpp handle our URL to use
	c.setOpt( new curlpp::options::Url(URL));
	//Perform cURLpp request
	os << c;
	
	bool parsedSuccess = reader.parse(os.str(), root, false);
	if (not parsedSuccess){
    		std::cout<<"Failed to parse JSON"<<std::endl
		    	<<reader.getFormattedErrorMessages()
    			<<std::endl;
        }        
        this->INFO = root;
        //Pretty print to make sure we got INFO and stored it correctly
        //std::cout<<this->INFO.toStyledString()<<std::endl;
        this->putInfo();
}
	
void Book::printBook(){
	std::cout<<"\e[1m\e[32mTitle\e[0m: "
		<<"\e[4m\e[96m"<<this->title<<"\e[0m"<<std::endl
	    <<"\e[1m\e[32mAuthor(s)\e[0m: "
		<<"\e[96m"<<this->authors<<"\e[0m"<<std::endl
	    <<"\e[1m\e[32mDate Published\e[0m: "
	    	<<"\e[96m"<<this->published<<"\e[0m"<<std::endl	    
	    <<"\e[1m\e[32mISBN-10\e[0m: "
	    	<<"\e[96m"<<this->isbn10<<"\e[0m"<<std::endl
	    <<"\e[1m\e[32mISBN-13\e[0m: "
	    	<<"\e[96m"<<this->isbn13<<"\e[0m"<<std::endl
	    <<"\e[1m\e[32mDescription\e[0m: \e[96m\""
	    	<<this->subject<<"\""<<"\e[0m"<<std::endl;
}

void Book::makeBook(){

	//Get the ISBN number from user
	getISBN();
	//Get the JSON data from Google/OL, parse it, and store it in local protected variables
	getJSON();
	//Display the retrieved data
	printBook();
	//Ask to store retrieved data in database
	std::string yn = "n";
	std::cout<<"Store book in database?(y/n)";
	std::cin>>yn;
	std::cin.ignore();
	if(yn == "y"){
		if(storeBook()){
			std::cout<<"\""<<this->title<<"\" stored successfully!"<<std::endl;
		}
		else{
			std::cout<<"Failed to store book!"<<std::endl;
		}
	}
}

bool Book::storeBook(){

  try {
	//Open database connection and create file if it doesn't exist
	sqlite::database db("library.db");
	
	//Create the books table if it doesn't exist
	db <<
		"create table if not exists books ("
		" _id integer primary key autoincrement not null,"
		"title text,"
		"authors text,"
		"published text,"
		"isbn10 text,"
		"isbn13 text,"
		"subjects text"
		");";
		
	//Insert this new book record
	db << "insert into books (title, authors, published, isbn10, isbn13, subjects) values (?,?,?,?,?,?);"
	   << this->title
	   << this->authors
	   << this->published
	   << this->isbn10
	   << this->isbn13
	   << this->subject;
	   
	//Return record _id (to verify insert worked) and show total # of records
	int count = 0;
	db <<"select count(*) from books" >> count;
	std::cout<<"Last record ID is: "<<db.last_insert_rowid()<<std::endl
		 <<"There are now "<<count<<" books in the database."<<std::endl;
	return true;
   }
   catch (std::exception& e){
   	std::cout << e.what() << std::endl;
   }
}

class OLBook: public Book {
   public:
   	using Book::Book;   	
   	void putInfo();
};

void OLBook::putInfo(){
   		std::string authors;
		std::string subjects;
		//Json::Value root = info[0]["items"][0]["volumeInfo"];	
		std::string id = "ISBN:" + this->ISBN;
		Json::Value root = this->INFO[id];
		Json::Value a_authors = root["authors"];
		Json::Value a_subjects = root["subjects"];
	
	
		this->title = root["title"].asString();	
		std::cout<<"title"<<std::endl;
		if(a_authors.size()>1){
			for(int i=0; i<a_authors.size(); ++i){
				authors.append(a_authors[i]["name"].asString());
				if(i < (a_authors.size() - 1)){
					authors.append(", ");
				}	
			}
		}
		else{
			authors = a_authors[0]["name"].asString();
		}
		this->authors = authors;
		std::cout<<"authors"<<std::endl;
	
		this->published = root["publish_date"].asString();
		std::cout<<"publish_date"<<std::endl;
		this->isbn10 = root["identifiers"]["isbn_10"][0].asString();
		std::cout<<"isbn10"<<std::endl;
		this->isbn13 = root["identifiers"]["isbn_13"][0].asString();
		std::cout<<"isbn13"<<std::endl;
		if(a_subjects.size()>1){
			for(int i=0; i<a_subjects.size(); ++i){
				subjects.append(a_subjects[i]["name"].asString());
				if(i < (a_subjects.size() - 1)){
					subjects.append(", ");
				}
			}
		}
		else{
			subjects = a_subjects[0]["name"].asString();
		}
		this->subject = subjects;
		std::cout<<"subjects"<<std::endl;
	}
