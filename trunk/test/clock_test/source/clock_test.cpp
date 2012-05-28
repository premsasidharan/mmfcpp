#include <stdio.h>
#include <unistd.h>

#include <child_clock.h>
#include <master_clock.h>

int main(int argc, char** argv)
{
    int64_t time = 0;
    Condition_variable cv;
	Master_clock master("main");
	Child_clock* child = master.create_child("child0");
	master.start(0);
    for (int i = 0; i < 10; i++)
    {
        master.get_deviation(1000000*(i+1), time);
        if (time > 0) 
        {
            cv.timed_uwait(time);
        }
        printf("\nlag time: %ld, %d", time, i+1);
    }
	return 0;
}

