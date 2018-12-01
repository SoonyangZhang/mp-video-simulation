BEGIN{
    stop=200
    c=0
    sum_buf=0
}
{
    time=$1;
    buf=$2;
    if(time<stop){
        sum_buf=sum_buf+buf;
        c=c+1;
    }
}

END{
    average=sum_buf/c;
    printf("buf\t%s\t\n",average);
}