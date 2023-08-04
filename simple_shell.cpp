// simple shell 
#include<iostream>
#include<dirent.h>
#include<fstream>
#include<sstream>
#include<unistd.h>
#include<string>
#include<glob.h>
#include<vector>
#include<filesystem>
#include<sys/wait.h>
#include<sys/stat.h>
#include<readline/readline.h>
#include<readline/history.h>

using namespace std;
namespace fls = std::filesystem;

#define MAX 80

vector<string> tokenize(const string &input, char dl)
{
    vector<string> tokens;
    stringstream ss(input);
    string token;
    while (getline(ss, token, dl))
        tokens.push_back(token);
    return tokens;
}

vector<string> checkFiles(const string &pattern)
{
    glob_t glob_res;
    glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_res);

    vector<string> fnames;
    size_t n = glob_res.gl_pathc;
    for(size_t i= 0; i < n; i++)
    {
        fnames.push_back(glob_res.gl_pathv[i]);
    }

    globfree(&glob_res);
    return fnames;
}

void cd(const vector<string> &path)
{
    if (path.size() < 2)
        chdir(getenv("HOME"));

    else if (path.size() == 2)
    {
        if (path[1] == "..")
        {
            fls::path prevDir = fls::current_path();
            prevDir = prevDir.parent_path();
            fls::current_path(prevDir);
        }
        else if (chdir(path[1].c_str()) < 0)
        {
            fls::path currDir = fls::current_path();
            cerr << "Error given location does not exist" << endl;
        }
    }

    else
    {
        cerr << "Incorrect usage of command.\nUsage: cd [directory]"<< endl;
    }
}

void pwd()
{
    char curr[MAX];
    int s = sizeof(curr);
    if (getcwd(curr, s) != NULL)
        cout <<"Current path : "<< curr << endl;
    else
        cerr << "Error getting current directory!" << endl;
}

void ls()
{
    DIR *dir = opendir(".");
    if (dir == nullptr)
    {
        cerr << "Error opening current directory!" << endl;
        return;
    }
    dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
        cout << entry->d_name << endl;
    closedir(dir);
}

void cat(const vector<string> &fname)
{
    if (fname.size() < 2)
    {
        cerr << "Incorrect usage of command.\nUsage: cat [file]" << endl;
        return;
    }

    vector<string> fn;

    if (string(fname[1]).find_first_of("*") != string::npos)
        fn = checkFiles(fname[1]);
    else
        fn.push_back(fname[1]);

    for (const auto &filename : fn)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error opening the file: " << filename <<"\nFile does not exist."<< endl;
            continue;
        }

        string line;
        while (getline(file, line))
            cout << line << endl;

        file.close();
    }
}

void makedir(const vector<string> dirname)
{
    if (dirname.size() < 2)
    {
        cerr << "Incorrect usage of command.\nUsage: mkdir [directory]" << endl;
        return;
    }

    string filename = dirname[1];

    int result = mkdir(filename.c_str(), 0777);

    if (result < 0)
        cerr << "Error creating directory" << endl;
    else
        cout << "Directory created successfully" << endl;
}

void rmdir(const vector<string> dirname)
{
    if (dirname.size() < 2)
    {
        cerr << "Incorrect usage of command.\nUsage: rmdir [directory]" << endl;
        return;
    }

    string fname = dirname[1];

    if (!fls::exists(fname))
    {
        cerr << "Directory does not exist!" << endl;
        return;
    }
    try
    {
        fls::remove(fname);
        cout << "Directory "<<fname<<" removed successfully" << endl;
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
    }
}

void clear()
{
    system("clear");
}

void touch(const vector<string> filename)
{
    if (filename.size() < 2)
    {
        cerr << "Incorrect usage of command.\nUsage: touch [file name]" << endl;
        return;
    }

    fstream file;
    file.open(filename[1], ios::out);
    

    if(!file)
    {
        cerr << "Error! File could not be created." << endl;
        return;
    }
    else
        cout << "File created successfully." << endl;

    file.close();
}

void script(const vector<string> cmd)
{
    vector<const char *> argv;
    for (const auto &arg : cmd)
        argv.push_back(arg.c_str());
    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid < 0)
        cerr << "Process creation failed" << endl;
    else if (pid == 0)
    {
        execvp(argv[0], const_cast<char **>(&argv[0]));
        cerr << "Error! Invalid Command"<< " " << cmd[0] << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}

int main()
{
    string input;
    const char *prompt = "Shell:";
    
    // Initialize readline for command history
    using_history();

    while (true)
    {
        char curr[MAX];
        int s = sizeof(curr);
        getcwd(curr, s);
            cout << prompt<< curr ;
        char *line = readline("> ");
        if (line == nullptr)
            break;

        input = line;
        free(line);

        // Check if the input is empty
        if (input.empty())
            continue;

        // Add the input to the history
        add_history(input.c_str());

        vector<string> args = tokenize(input, ' ');

        if(args[0]=="exit")
            break;

        if(args[0]=="cd")
        {
            cd(args);
            continue;
        }

        else if(args[0]=="pwd")
        {
            pwd();
            continue;
        }

        else if(args[0]=="ls")
        {
            ls();
            continue;
        }

        else if(args[0]=="cat")
        {
            cat(args);
            continue;
        }

        else if(args[0]=="mkdir")
        {
            makedir(args);
            continue;
        }

        else if(args[0]=="rmdir")
        {
            rmdir(args);
            continue;
        }

        else if(args[0]=="clear")
        {
            clear();
            continue;
        }

        else if(args[0]=="touch")
        {
            touch(args);
            continue;
        }

        script(args);
    }

    // Clean up readline history
    clear_history();
    return 0;
}
