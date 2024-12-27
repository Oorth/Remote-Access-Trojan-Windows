#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

void show_loading_with_dots() {
    string message = "Loading";  // Initial message
    const string dots[] = {".", "..", "...", "..", "."}; // Different stages of dot animation
    
    // Display the loading message first
    //cout << message;
    //cout.flush();  // Immediately print the message

    // Animate dots after the loading message
    for (int i = 0; i < 10; ++i) {  // Show 10 stages of dot animation
        cout << "\r" << dots[i % 5];  // Cycle through dot patterns
        cout.flush();  // Ensure immediate printing
        i++;
        this_thread::sleep_for(chrono::milliseconds(300));  // Change every 300 ms
    }

    // After completing the animation, print a message indicating completion
    cout << "\r" << message << " Done!" << endl;  // Overwrite "Loading" with "Done!"
}

int main() {
    show_loading_with_dots();
    return 0;
}
