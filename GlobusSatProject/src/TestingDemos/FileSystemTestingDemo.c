#include "FileSystemTestingDemo.h"
#include "GlobalStandards.h"
#include <hcc/api_fat.h>

Boolean TestlistFiels(){


	F_FIND find;
	if (!f_findfirst("*.*",&find))
	{
		do
		{
			printf("filename:%s",find.filename);

			 if (find.attr&F_ATTR_DIR)
			 {
				 printf ("directory\n");
			 }
			 else
			 {
				 printf (" size %d\n",find.filesize);
			 }
		} while (!f_findnext(&find));
	}
	printf("all file names printed\n");
	return TRUE;
}


Boolean selectAndExecuteFSTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) List files \n\r");

	unsigned int number_of_tests = 1;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = TestlistFiels();
		break;
	default:
		break;
	}
	return offerMoreTests;
}

Boolean MainFileSystemTestBench()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteFSTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return FALSE;
}
