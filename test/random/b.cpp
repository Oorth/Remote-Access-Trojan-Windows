#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <algorithm>  // For std::replace

#ifdef _WIN32
#include <windows.h>
#include <direct.h>  // For _getcwd()
#else
#include <unistd.h>
#endif

class Shell {
public:
    // Constructor
    Shell() {}

    // Destructor
    ~Shell() {}

    // Start the shell
    void start() {
        std::string command;

        // Main shell loop
        while (true) {
            show_prompt();  // Display the prompt
            
            std::getline(std::cin, command);  // Read user input
            
            if (command == "exit") {
                break;  // Exit the shell if the user types "exit"
            }
            
            // Execute the command or handle it (for now, just display it)
            std::cout << "You typed: " << command << std::endl;
        }
    }

private:
    // Show the shell prompt in the format user@hostname:~$ 
    void show_prompt() {
        // Get the current username
        const char* user = getenv("USER");
        if (user == nullptr) {
            user = getenv("USERNAME");  // On Windows, the environment variable is USERNAME
        }
        if (user == nullptr) {
            std::cerr << "Unable to get user name" << std::endl;
            return;
        }

        // Use "abc" as the hostname, instead of fetching the real one
        const char* hostname = "abc";

        // Get the current working directory
        char cwd[1024];
#ifdef _WIN32
        if (_getcwd(cwd, sizeof(cwd)) == nullptr) {  // Use _getcwd() on Windows
#else
        if (getcwd(cwd, sizeof(cwd)) == nullptr) {  // Use getcwd() on Linux/macOS
#endif
            std::cerr << "Unable to get current working directory" << std::endl;
            return;
        }

        // Get the home directory from the environment variable (for Linux/macOS and Windows)
        const char* homeDir;
#ifdef _WIN32
        homeDir = getenv("USERPROFILE");  // Windows home directory is USERPROFILE
#else
        homeDir = getenv("HOME");  // Linux/macOS home directory is HOME
#endif
        
        // Convert backslashes to forward slashes on Windows
#ifdef _WIN32
        std::replace(cwd, cwd + strlen(cwd), '\\', '/');
#endif

        // Check if the current directory is the home directory and replace with "~"
        if (homeDir != nullptr && strcmp(cwd, homeDir) == 0) {
            std::cout << user << "@" << hostname << ":~$ ";  // Display ~ for home directory
        } else {
            std::cout << user << "@" << hostname << ":" << cwd << "$ ";  // Display full path
        }
    }
};

// Entry point
int main() {
    Shell shell;
    shell.start();  // Start the shell

    return 0;
}
