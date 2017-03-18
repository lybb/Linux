#include<stdio.h>
#include<assert.h>
void Display(int* arr,int len)
{
	int i=0;
	for(; i<len; i++)
	{
		printf("%d ",arr[i]);
	}
}
int main()
{
	int arr[] = {1,2,3,4,5,6,7,8,9};
	int size = sizeof(arr)/sizeof(arr[0]);
	Display(arr,size);
	printf("\n");
	return 0;
}
