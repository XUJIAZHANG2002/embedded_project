#include <stdio.h>
#include <stdlib.h>

int main() {
    // Replace "your_script.py" with the actual path to your Python script
    int day = 2;
    char *city = "New York";
    char command[100];
    sprintf(command,"python3 weather.py --city \"%s\" --day %d", city,day);

    char buffer[128];

    // Open a pipe to the command and read its output
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        printf("Error opening pipe.\n");
        return -1;
    }

    // Read and print the output
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }

    // Close the pipe
    int status = pclose(pipe);

    // Check if the command execution was successful
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // Read and print the contents of the text file
        FILE *file = fopen("output.txt", "r");
        if (file != NULL) {
            printf("\%s Weather in %d days\n", city, day);
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                printf("%s\n", buffer);
            }
            fclose(file);
        } else {
            printf("\nError opening the text file.\n");
        }
    } else {
        printf("\nError executing Python script.\n");
    }

    return 0;
}
