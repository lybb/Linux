#!/bin/awk -f

BEGIN{
	count=0;
	printf("Begin count!\n");
}

{
	while(count < 3)
	{
		count += 1;
		print $1;
	}
}

END{
	printf("End:count=%d\n",count);
}
