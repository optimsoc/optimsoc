# -*- bash -*-
test "$BASH" || eval 'exec bash $0 ${1+"$@"}'


function usage
{
    echo "usage: overlap_experiment.sh -t <num_trials> -s <num_sends> -d <min_data_size> -m <max_data_size> -i <data_size_incr> -n <min_nodes> -N <node_incr> -x <max_nodes> -v <_min_vector_size> -V <max_vector_size> -I <vector_size_incr> -f <filename>"
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
    echo "min_vector_size = " $MIN_VECTOR_SIZE
    echo "max_vector_size = " $MAX_VECTOR_SIZE 
    echo "vector_size_incr = " $VECTOR_SIZE_INCR
    echo "filename = " $FILENAME

    exit 1
}
    
NUM_TRIALS=3
NUM_SENDS=10000
MIN_DSIZE=20
MAX_DSIZE=40
DSIZE_INCR=20
MIN_NODES=2
MAX_NODES=4
NODE_INCR=1
MIN_VECTOR_SIZE=100
MAX_VECTOR_SIZE=1000
VECTOR_SIZE_INCR=100
FILENAME=overlap_experiment.log


while getopts ht:s:d:m:i:n:x:v:V:I:f:N: OPT
do
    case "$OPT" in
      h)  usage;;
      t)  NUM_TRIALS="$OPTARG";;
      s)  NUM_SENDS="$OPTARG";;
      d)  MIN_DSIZE="$OPTARG";;
      m)  MAX_DSIZE="$OPTARG";;
      i)  DSIZE_INCR="$OPTARG";;
      n)  MIN_NODES="$OPTARG";;
      x)  MAX_NODES="$OPTARG";;
      N)  NODE_INCR="$OPTARG";;
      f)  FILENAME="$OPTARG";;
      v)  MIN_VECTOR_SIZE="$OPTARG";;
      V)  MAX_VECTOR_SIZE="$OPTARG";;
      I)  VECTOR_SIZE_INCR="$OPTARG";;
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
echo min_vector_size: $MIN_VECTOR_SIZE >> $FILENAME
echo max_vector_size: $MAX_VECTOR_SIZE >> $FILENAME
echo vector_size_incr: $VECTOR_SIZE_INCR >> $FILENAME
echo filename: $FILENAME >> $FILENAME

cat overlap_log_template.txt >> $FILENAME

# first loop over number of nodes
# then loop over number of bytes
# then loop over vector sizes
# then loop over number of trials


# pthread MCAPI messages
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_pthread -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# fork MCAPI messages
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_fork -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# pthread MCAPI messages non-blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_pthread -a -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# fork MCAPI messages non-blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_fork -a -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# pthread MCAPI packets
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_pthread -p -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# fork MCAPI packets
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_fork -p -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# pthread MCAPI packets non blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_pthread -p -a -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done

# fork MCAPI packets non blocking
for ((i=MIN_NODES; i<=MAX_NODES; i+=NODE_INCR)); do
  for ((j=MIN_DSIZE; j<=MAX_DSIZE; j+=DSIZE_INCR)); do
    for ((v=MIN_VECTOR_SIZE; v <= MAX_VECTOR_SIZE; v+=VECTOR_SIZE_INCR)); do
      for ((k=0; k<NUM_TRIALS; k+=1)); do
        BYTES=$j
        if [ $BYTES -eq 0 ]; then
          BYTES=$BASE_DSIZE
        fi
        ../src/tc_overlap_fork -p -a -i $i -b $BYTES -n $NUM_SENDS -d $v -f $FILENAME
      done
    done
  done
done


echo `date` >> $FILENAME


