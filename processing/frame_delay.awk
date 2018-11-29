BEGIN{
    stop=4000;
    c=0;
    sum_delay=0;
}

{
    frame_id=$1;
    frame_delay=$5;
    if(frame_id<=stop){
        sum_delay=sum_delay+frame_delay;
        c=c+1;
    }
}

END{
    average=sum_delay/c;
    printf("delay %s\t\n",average);
}