for ap1 in {1,5,9}
do
    for ap2 in {1,5,9}
    do
        for ap3 in {1,5,9}
        do
            for ap4 in {1,5,9}
            do
                ./ns3 run "rrm.cc --scen=2 --ch=${ap1}${ap2}${ap3}${ap4}" > output.log 2>&1
                result=$(grep "Average_throughput:" output.log | sed 's/Average_throughput: //')
                echo "[${ap1}${ap2}${ap3}${ap4}] : ${result}"
            done
        done
    done
done

