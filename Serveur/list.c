#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
 
void usage(char *name)
{
   printf("Usage : %s [directory]\n", name);
}
 
int lister(char *directory)
{
   DIR *dir = NULL;
   struct dirent *file = NULL;
 
   if((dir = opendir(directory)) == NULL)
   {
      return EXIT_FAILURE;
   }
 
   printf("Repertoire %s :\n", directory);
 
   while((file = readdir(dir)) != NULL)
   {
      if(strcmp(file->d_name, ".") && strcmp(file->d_name, ".."))
      {
         if(file->d_type == DT_DIR)
         {
            lister(strncat(directory, file->d_name, 256));
         }
         else
         {
            printf("\t%s\n", file->d_name);
         }
      }
   }
 
   printf("Fin repertoire %s\n", directory);
 
   closedir(dir);
 
   return EXIT_SUCCESS;
}
 
int main(int argc, char **argv)
{

 printf("%i\n", DT_BLK);//        This is a block device.

        printf("%i\n",DT_CHR);//      This is a character device.

       printf("%i\n",DT_DIR);//      This is a directory.

       printf("%i\n",DT_FIFO);//     This is a named pipe (FIFO).

       printf("%i\n",DT_LNK);//      This is a symbolic link.

      printf("%i\n", DT_REG);//      This is a regular file.

       printf("%i\n",DT_SOCK);//     This is a UNIX domain socket.

       printf("%i\n",DT_UNKNOWN);// The file type is unknown
   char directory[256];
 
   if(argc < 2)
   {
      usage(argv[0]);
      return EXIT_FAILURE;
   }
 
   strncpy(directory, argv[1], 256);
 
   return lister(directory);
}
