#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <Kernel/sys/syslimits.h>
#include <errno.h>

const char *pname;

int system(const char *string);

void print_usage()
{
    fprintf(stderr, "Usage: ./lazer-beatmap-import [beatmap directory]\n");
    printf("Directory path must NOT contain trailing slash\n");
}

int main(int argc, char **argv)
{
    struct dirent *de;   // Pointer for directory entry
    char *command;       // Command string for the terminal
    char cwd[PATH_MAX];  // Current working directory
    char *beatmaps_path; // Path to the beatmap directory
    char *temp_path;     // Temporary directory for zipping the folder

    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Currently working in: %s\n", cwd);
        printf("Beatmap folder path provided: %s\n", argv[1]);

        if (mkdir("failed", S_IRWXU | S_IRWXG | S_IRWXO) != -1) 
        {
            asprintf(&temp_path, "%s/failed", cwd);
        }
        else if (strcmp(strerror(errno), "File exists") == 0) 
        {
            asprintf(&temp_path, "%s/failed", cwd);
        }
        else
        {
            printf("Error creating a temporary directory: %s\n", strerror(errno));
            return 4;
        }
    }
    else
    {
        printf("Error getting current working directory: %s\n", strerror(errno));
        return 1;
    }

    if ((beatmaps_path = argv[1]) == NULL)
    {
        print_usage();
        return 1;
    }

    // opendir() returns a pointer of DIR type.
    DIR *beatmaps_dir = opendir(beatmaps_path);

    if (beatmaps_dir == NULL) // opendir returns NULL if couldn't open directory
    {
        perror("Could not open beatmap directory\n");
        return 4;
    }
    else 
    {
        printf("Opened beatmap directory %s\n", beatmaps_path);
    }

    int i = 1;
    while ((de = readdir(beatmaps_dir)) != NULL)
    {
        if (i > 2)
        {
                printf("Copying beatmap %s\n", de->d_name);
				asprintf(&command, "cp -R '%s/%s' '%s' ", beatmaps_path, de->d_name, temp_path);
				if (system(command) == 0) 
				{
                    printf("Zipping beatmap %s\n", de->d_name);
					asprintf(&command, "zip -r '%s/%s.osz' '%s/%s' ", temp_path, de->d_name, temp_path, de->d_name);
					if (system(command) == 0)
					{
						asprintf(&command, "open '%s/%s.osz' ", temp_path, de->d_name);

                        printf("Removing copied folder %s\n", de->d_name);
					    asprintf(&command, "rm -rf '%s/%s' ", temp_path, de->d_name);
                        if (system(command) != 0)
                        {
                            perror("Error removing copied folder\n");
                        }
					}
                    else perror("Error zipping beatmap\n");
				}
                else perror("Error copying beatmap\n");
        }
        i++;
    }

    free(de);
    free(command);
    free(beatmaps_path);
    free(temp_path);

    closedir(beatmaps_dir);
    free(beatmaps_dir);

    return 0;
}
