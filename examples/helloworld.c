#include <stdio.h>
#include <unistd.h>

int main(){
	
int i=0;
	while(1){
	printf("Hello world %d\n", i);
	i = i -10;
	i = i+ 10;
	i++;
	sleep(1);
	}
}
