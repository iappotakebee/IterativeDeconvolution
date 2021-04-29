se g
file = "20190103151339"
file = sprintf("%s_history.txt", file)

#plot file u 1:2 w l
#rep file u 1:3 w l
#rep file u 1:4 w l
#rep file u 1:5 w l
#rep file u 1:6 w l
#rep file u 1:7 w l
#rep file u 1:8 w l
#lastcol=system(sprintf("awk -F'[ ]' 'NR==1{print NF}' %s",file))
#p for[i=2:lastcol] file u 1:i w l title system(sprintf("awk -F, 'NR==1{print $%d}' %s",i,file))
#set yrange [0:1000]
lastcol=system(sprintf("awk 'NR==1{print NF}' %s",file))
p for[i=2:lastcol] file u 1:i w l title system(sprintf("awk 'NR==1{print $%d}' %s",i,file))
#rep "20190103151339_all.txt" u 1:2 w l
