#ifndef NAMESLOGIC_H_
#define NAMESLOGIC_H_

#define NAMES_COUNT 5
#define MAX_NAME_SIZE 16


char names[NAMES_COUNT][MAX_NAME_SIZE] = {"ishay", "uriel", "nave", "raz", "dror"};
#endif /* NAMESLOGIC_H_ */

void GetRandomName(char buffer[MAX_NAME_SIZE]);
