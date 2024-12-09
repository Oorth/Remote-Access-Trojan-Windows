#include <iostream>
#include <string.h>
using namespace std;

#define MAX_IP_LENGTH 16
#define MAX_TEXT_LENGTH 256

int argc_global;
char **argv_global;

char ipAddress[MAX_IP_LENGTH], randomText[MAX_TEXT_LENGTH];
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

                                         !! NO CONFIGURATION FILE LOADED !!
                                    
                                                [::] Do this [::]
                                    [::] Application_name.exe -f config.txt [::]


)";

}

bool getdata()
{
    FILE *sourceFile;
    const char *fileName = NULL;


    if (argc_global < 3) return false;

    if (strcmp(argv_global[1], "-f") == 0) fileName = argv_global[2];
    else return false;


    sourceFile = fopen(fileName, "r");
    if (sourceFile == NULL)
    {
        perror("\n\n\t|||||| Failed to open file ||||||\n\n");
        return false;
    }


    while (fscanf(sourceFile, "%15s", ipAddress) == 1)                                                // Read the IP address
    {        
        fgetc(sourceFile);                                                                            // Consume the newline character left by fscanf
        if (fgets(randomText, MAX_TEXT_LENGTH, sourceFile) != NULL)                                   // Read the random text
        {
            randomText[strcspn(randomText, "\n")] = '\0';                                             // Remove the newline character if it exists in randomText
        }
    }

    fclose(sourceFile);

    return true;
}

int main(int argc, char *argv[]) 
{
    argc_global = argc;
    argv_global = argv;

    if (!getdata())
    {
        noConig();
        system("pause");
        return 0;
    }

    banner();
    os_detection();

    printf("\n\n\tIP Address: %s\n", ipAddress);
    printf("\tRandom Text: %s\n", randomText);
    printf("\tOS: %c\n\n\n", os);

    system("pause");
    return 0;
}