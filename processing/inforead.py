delay_info=["scale","wrr","edcld","bc","minq","sfl","water"]
test_case=9


for k in range(len(delay_info)):
    file_out=delay_info[k]+".txt"
    fout=open(file_out, 'w')
    for j in range(test_case):
        file_in="../delay/"+"avg_"+delay_info[k]+"_"+str(j+1)+"_frameinfo_delay.txt"
        fin=open(file_in)
        line=fin.readline()
        fout.write(str(j+1)+"\t")
        fout.write(line.split()[1]+"\n")
        fin.close()
    fout.close()

for k in range(len(delay_info)):
    file_out=delay_info[k]+"_buf.txt"
    fout=open(file_out, 'w')
    for j in range(test_case):
        file_in="../buf/"+"avg_"+delay_info[k]+"_"+str(j+1)+"_buf.txt"
        fin=open(file_in)
        line=fin.readline()
        fout.write(str(j+1)+"\t")
        fout.write(line.split()[1]+"\n")
        fin.close()
    fout.close()


