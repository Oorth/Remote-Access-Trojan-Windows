#include <iostream>
#include <string.h>
using namespace std;

#define MAX_IP_LENGTH 16   // Maximum length for an IPv4 address
#define MAX_TEXT_LENGTH 256 // Maximum length for the random text

char ipAddress[MAX_IP_LENGTH];
char randomText[MAX_TEXT_LENGTH];
char randomText2[MAX_TEXT_LENGTH];


void banner()
{
    cout << R"(

                      :::::::::      :::           :::::::::      ::: ::::::::::: 
                     :+:    :+:   :+: :+:         :+:    :+:   :+: :+:   :+:      
                    +:+    +:+  +:+   +:+        +:+    +:+  +:+   +:+  +:+       
                   +#+    +:+ +#++:++#++:       +#++:++#:  +#++:++#++: +#+        
                  +#+    +#+ +#+     +#+       +#+    +#+ +#+     +#+ +#+         
                 #+#    #+# #+#     #+#       #+#    #+# #+#     #+# #+#          
                #########  ###     ###       ###    ### ###     ### ###           

                                [::] Made By Oorth :) [::]
                        
                        [::] Use this Ethically, if you dont [::]
                            [::] I'll just enjoy watching  [::]

                                [::] UNDER DEVELOPMENT!! [::]
)";

}
void getdata()
{

    FILE *sourceFile;
    long fileSize;
    char *buffer;

    sourceFile = fopen("test.txt", "r");

    fseek(sourceFile, 0, SEEK_END);                                     // Move the file pointer to the end to get the file size
    fileSize = ftell(sourceFile);                                       // Get the size of the file
    fseek(sourceFile, 0, SEEK_SET);                                     // Move back to the start of the file

    buffer = (char *)malloc(fileSize + 1);                              // +1 for null-terminator
    
    while (fscanf(sourceFile, "%15s", ipAddress) == 1)                  // Read the IP address from the line
    {        
        fgetc(sourceFile);                                              // Consume the newline character left by fscanf
        if (fgets(randomText, MAX_TEXT_LENGTH, sourceFile) != NULL)     // Read the random text from the next line
        {
            randomText[strcspn(randomText, "\n")] = '\0';               // Remove the newline character if it exists in randomText
        }
    }

    fclose(sourceFile);
    free(buffer);
}

int main() 
{

    banner();
    getdata();

    printf("\n\n\tIP Address: %s\n", ipAddress);
    printf("\tRandom Text: %s\n\n\n", randomText);

    return 0;
}