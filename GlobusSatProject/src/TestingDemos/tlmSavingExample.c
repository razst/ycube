#include <stdio.h>

#define SIZE 10

typedef struct data 
{
	int time;
	char arr[14];

} data;


typedef struct data1
{
	int num;

} data1;

typedef struct data2 
{
	float number;
	char str[10];

} data2;



data arr[SIZE];
int current_index = 0;

void reset() //reset the array
{
	for (int i = 0; i < SIZE; i++)
	{
		arr[i].time = -1;
	}
}
int addNum(int num) //add item to the array in the correct posiotion
{

	data2 d2;
	d2.number = 3.14;
	memcpy(&d2.str, "hello", sizeof("hello"));

	memcpy(&arr[current_index], &num, sizeof(int));
	memcpy(arr[current_index].arr, &d2, sizeof(d2));

	current_index = move(current_index);
	return 0;
}

int move(int num) //advance the index acordding to the arr size
{
	num++;
	if (num == SIZE)
	{
		num = 0;
	}
	return num;
}

void printArr() //printing the data by order
{
	int index = current_index;
	printf("Arr by order:\n");
	for (int i = 0; i < SIZE; i++)
	{
		if (arr[index].time != -1)
		{
			printf("time: %d\n", arr[index].time);
			float num;
			char str[10];
			memcpy(&num, arr[index].arr, sizeof(int));
			memcpy(str, arr[index].arr + sizeof(int), sizeof(str));

			printf("num: %f, str: %s\n\n", num, str);

		}
		index = move(index);
	}
}

int main1()
{
	reset();
	int input = 0, num = 0, start = 0, end = 0;
	while (input != 4)
	{
		printf("Choose option\n1 - add number\n2 - print array\n3 - add number by range\n4 - Exit\n");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			printf("enter number to add: ");
			scanf("%d", &num);
			addNum(num);
			break;
		case 2:
			printArr();
			break;
		case 3:
			printf("enter begining: ");
			scanf("%d", &start);
			printf("enter ending: ");
			scanf("%d", &end);
			if (end > start)
			{
				for (int i = start; i <= end; i++)
				{
					addNum(i);
				}
			}
			break;
		}
	}
	return 0;
}
