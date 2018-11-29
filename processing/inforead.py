delay_info=["scale","wrr","edcld","bc","sfl","water"]
test_case=6


for k in range(len(delay_info)):
    file_out=delay_info[k]+".txt"
    fout=open(file_out, 'w')
    for j in range(test_case):
        file_in="../oracle%s/data/"%str(j+1)+"avg_"+delay_info[k]+"_frameinfo_delay.txt"
        fin=open(file_in)
        line=fin.readline()
        fout.write(str(j+1)+"\t")
        fout.write(line.split()[1]+"\n")
        fin.close()
    fout.close()


