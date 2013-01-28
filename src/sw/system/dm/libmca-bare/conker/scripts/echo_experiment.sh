# -*- bash -*-
test "$BASH" || eval 'exec bash $0 ${1+"$@"}'


function usage
{
    echo "usage: echo_experiment.sh -t <num_trials> -s <num_sends> -d <min_data_size> -m <max_data_size> -i <data_size_incr> -n <min_nodes> -x <max_nodes> -N <node_incr> -f <filename> -a"
    echo ""
    echo "defaults:"
    echo "num_trials = " $NUM_TRIALS
    echo "num_sends = " $NUM_SENDS
    echo "min_data_size = " $MIN_DSIZE
    echo "max_data_size = " $MAX_DSIZE
    echo "data_size_incr = " $DSIZE_INCR
    echo "min_nodes = " $MIN_NODES
    echo "max_nodes = " $MAX_NODES
    echo "node_incr = " $NODE_INCR
    echo "filename = " $FILENAME

    exit 1
}
    
NUM_TRIALS=3
NUM_SENDS=10000
MIN_DSIZE=1
MAX_DSIZE=60
DSIZE_INCR=10
MIN_NODES=2
MAX_NODES=4
NODE_INCR=1
FILENAME=echo_experiment.log
a=

while getopts aht:s:d:m:i:n:x:f:N: OPT
do
    case "$OPT" in
      h)  usage;;
      a)  a="-A";;
      t)  NUM_TRIALS="$OPTARG";;
      s)  NUM_SENDS="$OPTARG";;
      d)  MIN_DSIZE="$OPTARG";;
      m)  MAX_DSIZE="$OPTARG";;
      i)  DSIZE_INCR="$OPTARG";;
      n)  MIN_NODES="$OPTARG";;
      x)  MAX_NODES="$OPTARG";;
      N)  NODE_INCR="$OPTARG";;
      f)  FILENAME="$OPTARG";;
      \?)		# unknown flag
      	  usage;;
    esac
done
shift `expr $OPTIND - 1`

BASE_DSIZE=$MIN_DSIZE


# funky stuff to deal with special case of zero or one min dsize
if [ $MIN_DSIZE -eq 1 ]; then
   MIN_DSIZE=0
fi

if [ $MIN_DSIZE -eq 0 ]; then
   BASE_DSIZE=1
fi

rm -f $FILENAME

echo `date` > $FILENAME

echo num_trials: $NUM_TRIALS >> $FILENAME
echo num_sends: $NUM_SENDS >> $FILENAME  
echo min_dsize: $MIN_DSIZE >> $FILENAME
echo max_dsize: $MAX_DSIZE >> $FILENAME
echo dsize_incr: $DSIZE_INCR >> $FILENAME
echo min_nodes: $MIN_NODES >> $FILENAME
echo max_nodes: $MAX_NODES >> $FILENAME
echo node_incr: $NODE_INCR >> $FILENAME
echo filename: $FILENAME >> $FILENAME

cat echo_log_template.txt >> $FILENAME

# first loop over number of nodes
# then loop over number of bytes
# then loop over number of trials


# pthread MCAPI messages
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_pthread -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
      done
  done
done

# fork MCAPI messages
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_fork -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# pthread MCAPI messages non-blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_pthread -a -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# fork MCAPI messages non-blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_fork -a -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# pthread MCAPI packets
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_pthread -p -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# fork MCAPI packets
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_fork -p -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# pthread MCAPI packets non blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_pthread -p -a -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# fork MCAPI packets non blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_fork -p -a -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
    done
  done
done

# pthread pipes 
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_pthread -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME -P
    done
  done
done

# fork pipes
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_fork -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME -P
    done
  done
done


# pthread sockets
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_pthread -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME -S
    done
  done
done

# fork sockets
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      ../src/tc_echo_fork -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME -S
    done
  done
done

# pthread MCAPI scalars
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      if [ $BYTES -eq 1 ] || [ $BYTES -eq 2 ] || [ $BYTES -eq 4 ] || [ $BYTES -eq 8 ]; then
        ../src/tc_echo_pthread -s -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
      fi
    done
  done
done

# fork MCAPI scalars
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((k=0; k<NUM_TRIALS; k+=1)); do
      BYTES=$j
      if [ $BYTES -eq 0 ]; then
        BYTES=$BASE_DSIZE
      fi
      if [ $BYTES -eq 1 ] || [ $BYTES -eq 2 ] || [ $BYTES -eq 4 ] || [ $BYTES -eq 8 ]; then
        ../src/tc_echo_fork -s -i $i -b $BYTES -n $NUM_SENDS $a -f $FILENAME
      fi
    done
  done
done


echo `date` >> $FILENAME
