#ifndef FILE_SYS
#define FILE_SYS

#include ""

/* Every static global variable used by this program will be contained by struct file_sys. 
 * When the program start to run, it init struct file_sys by reading data from disk first.
 * And if the program exit normally, it write struct file_sys to disk for next running */
struct file_sys {
	
};

#endif
