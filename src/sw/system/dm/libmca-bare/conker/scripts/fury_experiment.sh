# -*- bash -*-
test "$BASH" || eval 'exec bash $0 ${1+"$@"}'


function usage
{
    echo "usage: fury_experiment.sh -t <num_trials> -s <min_sends> -S <max_sends> -i <send_incr> -d <data_size> -f <filename>"
    echo ""
    echo "defaults:"
    echo "num_trials = " $NUM_TRIALS
    echo "min_sends = " $MIN_SENDS
    echo "max_sends = " $MAX_SENDS
    echo "send_incr = " $SEND_INCR
    echo "data_size = " $DSIZE
    echo "filename = " $FILENAME

    exit 1
}
    
NUM_TRIALS=3
MIN_SENDS=100
MAX_SENDS=1000
SEND_INCR=100
DSIZE=100
FILENAME=fury_experiment.log


while getopts ht:s:S:i:d:f: OPT
do
    case "$OPT" in
      h)  usage;;
      t)  NUM_TRIALS="$OPTARG";;
      s)  MIN_SENDS="$OPTARG";;
      S)  MAX_SENDS="$OPTARG";;
      i)  SEND_INCR="$OPTARG";;
      d)  DSIZE="$OPTARG";;
      f)  FILENAME="$OPTARG";;
      \?)		# unknown flag
      	  usage;;
    esac
done
shift `expr $OPTIND - 1`

rm -f $FILENAME

echo `date` > $FILENAME

echo num_trials: $NUM_TRIALS >> $FILENAME
echo min_sends: $MIN_SENDS >> $FILENAME  
echo max_sends: $MAX_SENDS >> $FILENAME
echo send_incr: $SEND_INCR >> $FILENAME
echo dsize: $DSIZE >> $FILENAME
echo filename: $FILENAME >> $FILENAME

cat fury_log_template.txt >> $FILENAME

# loop over number of sends
# then loop over number of trials


# pthread MCAPI messages
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_pthread -m -b $DSIZE -n $i -f $FILENAME
    done
  done

# fork MCAPI messages
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_fork -m -b $DSIZE -n $i -f $FILENAME
    done
  done

# pthread MCAPI messages non-blocking
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_pthread -m -a -b $DSIZE -n $i -f $FILENAME
    done
  done

# fork MCAPI messages non-blocking
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_fork -m -a -b $DSIZE -n $i -f $FILENAME
    done
  done

# pthread MCAPI packets
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_pthread -p -b $DSIZE -n $i -f $FILENAME
    done
  done

# fork MCAPI packets
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_fork -p -b $DSIZE -n $i -f $FILENAME
    done
  done

# pthread MCAPI packets non blocking
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_pthread -p -a -b $DSIZE -n $i -f $FILENAME
    done
  done

# fork MCAPI packets non blocking
  for ((i=MIN_SENDS; i<=MAX_SENDS; i+=SEND_INCR)); do
    for ((j=0; j<NUM_TRIALS; j+=1)); do
      ../src/tc_fury_fork -p -a -b $DSIZE -n $i -f $FILENAME
    done
  done


echo `date` >> $FILENAME
