#include<bits/stdc++.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include <sys/stat.h>
#include <ctime>
#define clr() printf("\033[H\033[J")
using namespace std;

/******************************************* GLOBAL VARIABLES *******************************************/

static struct termios initSettings,newSettings;
struct termios orig_termios; // for initialisation of raw mode
//char const* root;
string rootPath;

size_t cwdSize=1024;
char cwd[1024];
#define MAX 5		// max items to display in normal mode
struct winsize w;   // gives terminal window size in terms of rows and columns
vector<dirent*> fileNames;
vector<string> commandTokens;
stack<string> backS;
stack<string> forwardS;

//std::vector<string> inputVector;
//std::vector<string> backStack;
//std::vector<string> forwardStack;
//std::vector<string> searchStack;

int top=0;
int bottom=top+MAX;
int cursor=1;
int commandLine;
int alertLine;
int statusLine;
//int outputLine;
//int inputLine;
//FILE* out;

void moveCursor(int x,int y) {
	cout<<"\033["<<x<<";"<<y<<"H";
	fflush(stdout);
}


void disableNormalMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void printAlertLine(string s){
	moveCursor(alertLine,0);
	cout<<"ALERT : "<<s;
	moveCursor(cursor, 0);
}


void enableNormalMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
		printAlertLine("Unable to switch to Normal Mode                     ");
	}
}

void init(char * cwd){
    enableNormalMode();
	rootPath = string(cwd);
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
	commandLine=w.ws_row-4;
    top = 0;
    bottom = top + MAX;
    cursor = 0;
	statusLine = commandLine + 2;
	//outputLine=commandLine+1;
	alertLine=MAX+4;
	//inputLine=statusLine+1;
}

void printStatusLine(string s){
	moveCursor(statusLine,0);
	cout<<"STATUS : "<<s;
	moveCursor(commandLine, 18);
}


/*void enableNormalMode(){
	tcgetattr(0,&initSettings);
	newSettings=initSettings;
	newSettings.c_lflag &= ~ICANON;
	newSettings.c_lflag &= ~ECHO;
	newSettings.c_cc[VMIN]=1;
	newSettings.c_cc[VTIME]=0;
	tcsetattr(0,TCSANOW,&newSettings);
	return;
}
void disableNormalMode(){
	tcsetattr(0,TCSANOW,&initSettings);
	return;
}*/

int noOfFiles(){
    return fileNames.size();
}

void resetPointers(){
	top = 0;
	bottom = min(top+MAX, noOfFiles());
	cursor = 0;
}

// set the root path on program start // WILL ONLY CALL ONCE


void printCWD(){
	moveCursor(alertLine+2,0);
	cout<<"Current Working Directory : " <<cwd<<endl;
	cout<<"Root : "<<rootPath;
	moveCursor(cursor, 0);
}

void printNormalMode(){
	moveCursor(commandLine-2,0);
	cout<<"Mode : Normal Mode   ";
	moveCursor(cursor,0);
}

void printCommandMode(){
	moveCursor(commandLine-2,0);
	cout<<"Mode : Command Mode";
	moveCursor(commandLine,0);
	cout<<"Enter Command ~$ ";
	moveCursor(commandLine,18);
	for(int i=commandLine; i<statusLine; i++){
		for(int j=18; j<w.ws_col; j++){
			cout<<" ";
		}
	}
	moveCursor(commandLine,18);
}

void displayFiles(){
	clr();
	struct stat fileInfo;
	// printf("at: %s\n",currentDir);
	for(auto itr = top; itr < min(bottom, noOfFiles()); itr++){
		lstat(fileNames[itr]->d_name,&fileInfo);
        cout<<"$"<<itr+1<<" : \t\t";
        (S_ISDIR(fileInfo.st_mode)) ? cout<<"d" : S_ISSOCK(fileInfo.st_mode) ? cout<<"s": cout<<"-";

        (S_IRUSR & fileInfo.st_mode) ? cout<<"r" : cout<<"-";
    
        (S_IWUSR & fileInfo.st_mode) ? cout<<"w" : cout<<"-";

        (S_IXUSR & fileInfo.st_mode) ? cout<<"x" : cout<<"-";

        (S_IRGRP & fileInfo.st_mode) ? cout<<"r" : cout<<"-";

        (S_IWGRP & fileInfo.st_mode) ? cout<<"w" : cout<<"-";

        (S_IXGRP & fileInfo.st_mode) ? cout<<"x" : cout<<"-";

        (S_IROTH & fileInfo.st_mode) ? cout<<"r" : cout<<"-";

        (S_IWOTH & fileInfo.st_mode) ? cout<<"w" : cout<<"-";

        (S_IXOTH & fileInfo.st_mode) ? cout<<"x" : cout<<"-";

        cout<<"\t\t"<<fileInfo.st_size<<" B";

		if((S_ISDIR(fileInfo.st_mode))){
			cout<<"\t\t"<<"\033[1;31m"<<fileNames[itr]->d_name<<"\033[0m";
		}
		else{
			cout<<"\t\t"<<"\033[1;36m"<<fileNames[itr]->d_name<<"\033[0m";
		}
        //cout<<endl<<endl;
       // string s = ctime(&fileInfo.st_mtime);
        //cout<<s;
		cout<<"\n";
	}
	printCWD();
	printNormalMode();
	return;
}

void processCurrentDIR(char const* dir){
	DIR* di;
	struct dirent* direntStructure;

	if(!(di=opendir(dir))){
		printAlertLine("Directory is empty                                 ");
		return;
	}
	chdir(dir);
	getcwd(cwd,cwdSize);
	fileNames.clear();

	while((direntStructure=readdir(di))){
		fileNames.push_back(direntStructure);
	}

	closedir(di);
	resetPointers();
    //bottom = min(MAX,noOfFiles());
	displayFiles();
	return;
}



void scrollUp(){
	if(cursor>1){
		cursor--;
		moveCursor(cursor,0);
		return;
	}
	if(top==0){
		printAlertLine("You hit the top                               ");
		return;
	} 
	top--;
	bottom--;
	displayFiles();
	moveCursor(cursor,0);
	return;
}

void scrollUpK(){
    top = max(top-MAX, 0);
    bottom = top+MAX;
    displayFiles();
    moveCursor(cursor,0);
    return;
}

void scrollDownL(){
    bottom = min(bottom+MAX, noOfFiles());
    top = bottom - MAX;
    displayFiles();
    moveCursor(cursor,0);
    return;
}

void scrollDown(){
	if(cursor<noOfFiles() && cursor<MAX){
		cursor++;
		moveCursor(cursor,0);
		return;
	}
	if(bottom==noOfFiles()){
		printAlertLine("You hit the bottom                                ");
		return;
	} 
	top++;
	bottom++;
	displayFiles();
	moveCursor(cursor,0);
	return;
}

void levelUp(){
	if(cwd==rootPath){
		printAlertLine("You're already present in the home directory");
		return;
	}
	backS.push(string(cwd));
	processCurrentDIR("../");  // this line moves our control to parent directory
	return;
}

void home(){
	if(cwd==rootPath){
		printAlertLine("You're already present in the home directory");
		return;
	} 
	backS.push(string(cwd));
	processCurrentDIR(rootPath.c_str());
	return;
}

void moveBack(){
	if(!backS.size()){
		return;
	}
	string prevDirectory = backS.top();
	backS.pop();
	forwardS.push(string(cwd));
	processCurrentDIR(prevDirectory.c_str());
	return;
}

void moveForward(){
	if(!forwardS.size()){
		return;
	}
	string nextDirectory = forwardS.top();
	forwardS.pop();
	backS.push(string(cwd));
	processCurrentDIR(nextDirectory.c_str());
	return;
}

void enter(){
	struct stat fileInfo;
	char *fileName = fileNames[cursor+top-1]->d_name;
	lstat(fileName,&fileInfo);

	if(S_ISDIR(fileInfo.st_mode)){
		if(strcmp(fileName,"..")==0 ){
			levelUp();
			return;
		}  
		if(strcmp(fileName,".")==0) return;
		backS.push(string(cwd));
		processCurrentDIR((string(cwd)+'/'+string(fileName)).c_str());
	}
	else{
		pid_t pid=fork();
		if(pid==0){
			printAlertLine("File opened in default editor                   ");
			execl("/usr/bin/xdg-open","xdg-open",fileName,NULL);
			exit(1);
		}
	}
	return;
}

/***************************************** COMMAND MODE ****************************************************/

void copy_helper(string fname, string path){
	char b[1024];
	int fin,fout, nread;
	fin = open(fname.c_str(),O_RDONLY);
	fout = open((path).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	while((nread = read(fin,b,sizeof(b)))>0){
		write(fout,b,nread);
	}
}

void copy(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	for(int i=1; i<len-1; i++){
		string fname = commandTokens[i];
		string path = destination+'/'+fname;
		copy_helper(fname, path);
	}

}

void copy_dir_helper(string dirName, string destination){
	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(dirName.c_str()))){
		printStatusLine("Can't open the directory         ");
		return;
	}

	chdir(dirName.c_str()); 
	while((diren = readdir(di))){
		lstat(diren->d_name,&fileInfo);
		string dname =  string(diren->d_name);
		if(S_ISDIR(fileInfo.st_mode)){
			
			if( (dname == ".") || (dname == "..") ){
				continue;
			}
			/*if(s(".",diren->d_name) || strcmp("..",diren->d_name)==0){
				continue;
			}*/
			mkdir((destination + '/' + dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
			copy_dir_helper(dname , destination + '/' + dname);
		}
		else{
			copy_helper(dname,destination + '/' + dname);
		}
	}
	chdir("..");
	closedir(di);
	return;
}

void copy_dir(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];
	for(int i=1; i<len-1; i++){
		string dname = commandTokens[i];
		mkdir((destination+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
		copy_dir_helper(cwd + '/' + dname, destination + "/" + dname);
		//copy_dir_helper(dname, destination);
	}
}

int delete_file(){
	string destination = commandTokens[1];
	int status = unlink(destination.c_str());
	return status;
}

void delete_dir_helper(string destination){
	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(destination.c_str()))){
		printStatusLine("Can't open the directory      ");
		return;
	}
	chdir(destination.c_str()); 
	while((diren = readdir(di))){
		lstat(diren->d_name,&fileInfo);
		if(S_ISDIR(fileInfo.st_mode)){
			if(strcmp(".",diren->d_name)==0 || strcmp("..",diren->d_name)==0){
				continue;
			}
			delete_dir_helper(diren->d_name);
			rmdir(diren->d_name);
		}
		else{
			unlink(diren->d_name);
		}
	}
	chdir("..");
	closedir(di);
}

int delete_dir(){
	string destination = commandTokens[1];
	//printStatusLine(destination);
	if(destination==cwd){
		printStatusLine("You are present inside the directory which you want to delete!");
		return 0;
	}
	delete_dir_helper(destination);
	rmdir(destination.c_str());
	return 1;
}

void move(){
	copy();
	int len = commandTokens.size();
	for(int i=1; i<len-1; i++){
		string fname = commandTokens[i];
		string deleting_path = string(cwd) + "/" + fname;
		int status = unlink(deleting_path.c_str());
	}
}

void move_dir(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];
	for(int i=1; i<len-1; i++){
		string dname = commandTokens[i];
		mkdir((destination+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
		copy_dir_helper(cwd + '/' + dname ,destination + '/' + dname);
		delete_dir_helper(dname);
		rmdir(dname.c_str());	
	}
}

void create_file(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	for(int i=1; i<len-1; i++){
		string fname = commandTokens[i];
		open((destination + '/' + fname).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	}
}

void create_dir(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	for(int i=1; i<len-1; i++){
		string fname = commandTokens[i];
		mkdir((destination + '/' + fname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
	}
}

void rename(){
	string oldName = commandTokens[1];
	string newName = commandTokens[2];
	rename(oldName.c_str(),newName.c_str());
}

void goTo(){
	string destination = commandTokens[1];
		if(destination == "/") home();
		else{
			processCurrentDIR(destination.c_str());
		}
}

bool search_helper(string dirName, string tobeSearch){
	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(dirName.c_str()))){
		printStatusLine("Can't open the directory         ");
		return false;
	}

	chdir(dirName.c_str()); 
	while((diren = readdir(di))){
		lstat(diren->d_name,&fileInfo);
		string dname =  string(diren->d_name);
		if(tobeSearch == dname){
			//printAlertLine("A WILD TRUE APPEARS");
			processCurrentDIR(dirName.c_str());
			return true;
		}
		if(S_ISDIR(fileInfo.st_mode)){
			if( (dname == ".") || (dname == "..") ){
				continue;
			}
			bool t =  search_helper(dirName + '/' + dname, tobeSearch);
			if(t) return true;
		}
	}
	chdir("..");
	closedir(di);
	return false;
}

bool search(){
	string tbs = commandTokens[1];
	return search_helper(cwd, tbs);
}

bool modifyPath(){
	int len = commandTokens.size();
	string path = commandTokens[len-1];

	if(path[0]=='~')
	path.replace(0,1, rootPath);
	else if(path[0]=='.')
	path = cwd;
	else if(path[0]=='/')
	path = cwd + path;
	else{
		printStatusLine("Invalid path !");
		return false;
	}

	commandTokens[len-1] = path;
	printAlertLine(path);
	return true;
}

bool performActions(string query){
	
	if(query == "exit"){
		printNormalMode();
		return true;
	}

	else if(query == "copy"){
		copy();
		printStatusLine("Files copied successfully!");
		//printCommandMode();
	}

	else if(query == "copy_dir"){
		copy_dir();
		printStatusLine("Directories copied successfully!");
	}

	else if(query == "move"){
		move();
		processCurrentDIR(cwd);
		printStatusLine("Files moved successfully!");
		//printCommandMode();
	}

	else if(query == "move_dir"){
		move_dir();
		processCurrentDIR(cwd);
		printStatusLine("Directories moved successfully!");
	}

	else if(query == "delete_file"){
		int sts = delete_file();
		if(!sts){
			processCurrentDIR(cwd);
			printStatusLine("File deleted successfully..");
		} 		
		else printStatusLine("Deletion Failed!");
		//printCommandMode();
	}

	else if(query == "delete_dir"){
		int sts = delete_dir();
		if(sts!=0){
			processCurrentDIR(cwd);
			printStatusLine("Directory deleted successfully!");
		}
	}

	else if(query == "create_file"){
		create_file();
		processCurrentDIR(cwd);
		printStatusLine("Files created successfully!");
	}

	else if(query == "create_dir"){
		create_dir();
		processCurrentDIR(cwd);
		printStatusLine("Directory created successfully!");
	}

	else if(query == "rename"){
		rename();
		processCurrentDIR(cwd);
		printStatusLine("File renamed successfully!");
	}

	else if(query == "goto"){
		goTo();
	}

	else if(query == "search"){
		bool f = search();
		if(f) printStatusLine("File/Folder found!");
		else printStatusLine("File/Folder not found!");
	}

	else{
		printStatusLine("Invalid Command!");
	}

	return false;
}

void switchToCommandMode(){

	disableNormalMode();

	while(true){
		printCommandMode();
		string command;
		commandTokens.clear();
		getline(cin,command);
		//printAlertLine(command);
		stringstream sscommand(command); 
    	string token; 
		while(getline(sscommand, token, ' ')) commandTokens.push_back(token); 

		string query = commandTokens[0];
		int isValidPath = modifyPath();	
		
		if(!isValidPath && query != "exit" && query != "rename" && query != "search") continue;
		if(performActions(query)) break;
	}

	enableNormalMode();
	return;
}


int main(){
    clr();
	getcwd(cwd,cwdSize);
	init(cwd);
	printCWD();
	processCurrentDIR(cwd);
//	initKeyboard();	// initialise terminal settings
	char ch=' ';
	while(ch!='q'){
	
			ch=cin.get();		// take a character as user input and respond instantly
			switch(ch){
				case 65:scrollUp();		// scroll up on pressing up arrow
					break;
				case 66:scrollDown();	// scroll down on down arrow
					break;
                case 107:scrollUpK();
                    break;
                case 108:scrollDownL();
                    break;
				case 10:enter();		// opens a directory and file on pressing enter
					break;
				case 104:home();
					break;
				case 127:levelUp();		// go up one level on backspace
					break;
				case 68:moveBack();		// go back on left arrow
					break;
				case 67:moveForward();	// go forward on right arrow
					break;
				case ':':switchToCommandMode();	// switch to command mode with ':' 
					break;
				default:
					break;
			
		}
	}
//	closeKeyboard();	// restore terminal settings
	clr();
	cout<<endl<<endl<<endl<<endl<<endl<< " ***************    THANKS FOR USING MY FILE EXPLORER    *************** "<<endl<<endl<<endl<<endl<<endl;
	atexit(disableNormalMode);

    return 0;
}
