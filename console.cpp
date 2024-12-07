#include <iostream>
#include <string.h>
using namespace std;

#define MAX_IP_LENGTH 16   // Maximum length for an IPv4 address
#define MAX_TEXT_LENGTH 256 // Maximum length for the random text

char ipAddress[MAX_IP_LENGTH];
char randomText[MAX_TEXT_LENGTH];
char randomText2[MAX_TEXT_LENGTH];
char os;

void os_detection()
 {
    #if defined(_WIN32) || defined(_WIN64)
        os = 'W';
    #elif defined(__linux__)
        os = 'L';
    #elif defined(__APPLE__) || defined(__MACH__)
        os = 'M';
    #elif defined(__unix__) || defined(__unix)
        os = 'U';
    #endif
}

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

void noConig()
{
    cout << R"(

                      :::::::::      :::           :::::::::      ::: ::::::::::: 
                     :+:    :+:   :+: :+:         :+:    :+:   :+: :+:   :+:      
                    +:+    +:+  +:+   +:+        +:+    +:+  +:+   +:+  +:+       
                   +#+    +:+ +#++:++#++:       +#++:++#:  +#++:++#++: +#+        
                  +#+    +#+ +#+     +#+       +#+    +#+ +#+     +#+ +#+         
                 #+#    #+# #+#     #+#       #+#    #+# #+#     #+# #+#          
                #########  ###     ###       ###    ### ###     ### ###               

                            !!NO CONFIGURATION FILE LOADED!!
                                    
                                    [::] Do this [::]
                        [::] Application_name.exe -config.txt [::]


)";

}

void getdata(const char *fileName)
{

    FILE *sourceFile;
    long fileSize;
    char *buffer;

    sourceFile = fopen(fileName, "r");

    if (sourceFile == NULL)  // Check if the file opened successfully
    {
        perror("Failed to open file");
        return;
    }

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

int main(int argc, char *argv[]) 
{
   bool configration=0;
    os_detection();

    if (argc < 1)  // Check if the user has provided both -f and the file argument
    {
        noConig();
        return 1;
    }

    const char *fileName = NULL;

    // Loop through the command-line arguments to find the -f flag and its associated file
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)  // If -f is found
        {
            fileName = argv[i + 1];  // Get the file name from the next argument
            break;
        }
    }

    if (fileName == NULL)  // Check if a valid file name was found
    {
        printf("File parameter missing or incorrect. Usage: application.exe -f <filename>\n");
        return 1;
    }

 //   if (configration == 0) noConig();
    else
    {
        banner();
        getdata(fileName);

        printf("\n\n\tIP Address: %s\n", ipAddress);
        printf("\tRandom Text: %s\n", randomText);
        printf("\tOS: %c\n\n\n", os);
    }
    return 0;
}