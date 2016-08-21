#for ((x=0; x<50; x++)); do ./client -p 2000 -h eva -l $(yes blah | head -n$x); done
../client -h eva.fit.vutbr.cz -p 10000 -l rabj -N -U -S
../client -p 10000 -h eva.fit.vutbr.cz -l rysavy rabj -U -N -H
../client -p 10000 -h eva -UL -l unknown rysavy
../client -p 10000 -h eva -LG -u 1994
