#include<bits/stdc++.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<termios.h>
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
int statusLine;
//int outputLine;
//int inputLine;
//FILE* out;

void moveCursor(int x,int y) {
	cout<<"\033["<<x<<";"<<y<<"H";
	fflush(stdout);
}

void printAlertLine(string s){
	moveCursor(statusLine,0);
	cout<<"ALERT : "<<s;
	moveCursor(cursor, 0);
}

/*void disableNormalMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableNormalMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
		printAlertLine("Unable to switch to Normal Mode");
	}
}*/

void enableNormalMode(){
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
}

int noOfFiles(){
    return fileNames.size();
}

void resetPointers(){
	top = 0;
	bottom = min(top+MAX, noOfFiles());
}

// set the root path on program start // WILL ONLY CALL ONCE
void init(char * cwd){
    enableNormalMode();
	rootPath = string(cwd);
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
	commandLine=w.ws_row-4;
    top = 0;
    bottom = top + MAX;
    cursor = 0;
	//outputLine=commandLine+1;
	statusLine=MAX+4;
	//inputLine=statusLine+1;
}

void printCWD(){
	moveCursor(statusLine+2,0);
	cout<<"Current Working Directory : " <<cwd<<endl;
	cout<<"Root : "<<rootPath;
	moveCursor(cursor, 0);
}

void printNormalMode(){
	moveCursor(commandLine-2,0);
	cout<<"Mode : Normal Mode";
	moveCursor(cursor,0);
}

void printCommandMode(){
	moveCursor(commandLine-2,0);
	cout<<"Mode : Command Mode";
	moveCursor(commandLine,0);
	cout<<"Enter Command ~$ ";
	moveCursor(commandLine,18);
}


/******************************************* OPEN DIRECTORY *******************************************/

/*void openDirectory(char *currentDirectoryPath){

    struct dirent* diren;
    struct stat fileInfo;
    int rowCount = 0;
    DIR *dir; 
    dir = opendir(currentDirectoryPath);
    if(!dir){
    cout<<"Directory is empty \n";
    }
    else{
        while ( (diren=readdir(dir)) ){
            rowCount++;
            stat(diren->d_name, &fileInfo);
            cout<<" >> ";
           // cout<<fileInfo.st_size<<"bytes\t";
            cout<<diren->d_name<<"\t";
            fileNames.push_back(diren);
          //  cout<<ctime(&fileInfo.st_mtime);
           
           cout<<endl;
        } 
    }
    closedir(dir);    
}*/


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
		printAlertLine("Directory is empty");
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
		printAlertLine("You hit the top");
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
		printAlertLine("You hit the bottom");
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
		//backward.push_back(cwd);
		processCurrentDIR((string(cwd)+'/'+string(fileName)).c_str());
	}
	else{
		pid_t pid=fork();
		if(pid==0){
			printAlertLine("File opened in default editor");
			execl("/usr/bin/xdg-open","xdg-open",fileName,NULL);
			exit(1);
		}
	}
	return;
}

void switchToCommandMode(){
	printCommandMode();
	disableNormalMode();
	for(int i = commandLine; i<w.ws_row; i++){
		for(int j=0; j<w.ws_col; j++){
			if(i==commandLine && j<18) continue;
			cout<<" ";
		}
	}
	moveCursor(commandLine, 18);
}


int main(){
    clr();
	//char path[1024];
	//size_t size=1024;
	getcwd(cwd,cwdSize);
	init(cwd);
	//root10 = string(cwd);	// set the root path for program
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
	atexit(disableNormalMode);

    return 0;
}
