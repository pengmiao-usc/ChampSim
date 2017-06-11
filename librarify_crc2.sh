# ChampSim configuration
L1D_PREFETCHER=$1    # prefetcher/*.l1d_pref
L2C_PREFETCHER=$2    # prefetcher/*.l2c_pref
NUM_CORE=$3          # tested up to 4-core system

############## Some useful macros ###############
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

embed_newline()
{
   local p="$1"
   shift
   for i in "$@"
   do
      p="$p\n$i"         # Append
   done
   echo -e "$p"          # Use -e
}
#################################################

# Sanity check
if [ ! -f ./prefetcher/${L1D_PREFETCHER}.l1d_pref ] || [ ! -f ./prefetcher/${L2C_PREFETCHER}.l2c_pref ]; then
	echo "${BOLD}Possible L1D prefetcher: ${NORMAL}"
	LIST=$(ls prefetcher/*.l1d_pref | cut -d '/' -f2 | cut -d '.' -f1)
	p=$( embed_newline $LIST )
	echo "$p"

	echo
	echo "${BOLD}Possible L2C prefetcher: ${NORMAL}"
	LIST=$(ls prefetcher/*.l2c_pref | cut -d '/' -f2 | cut -d '.' -f1)
	p=$( embed_newline $LIST )
	echo "$p"
	exit
fi

# Check for multi-core
if [ "$NUM_CORE" != "1" ]
then
    echo "${BOLD}Building multi-core ChampSim...${NORMAL}"
    sed -i.bak 's/\<NUM_CPUS 1\>/NUM_CPUS '${NUM_CORE}'/g' inc/champsim.h
	sed -i.bak 's/\<DRAM_CHANNELS 1\>/DRAM_CHANNELS 2/g' inc/champsim.h
	sed -i.bak 's/\<DRAM_CHANNELS_LOG2 0\>/DRAM_CHANNELS_LOG2 1/g' inc/champsim.h
else
    echo "${BOLD}Building single-core ChampSim...${NORMAL}"
fi
echo

# Change prefetchers
cp prefetcher/${L1D_PREFETCHER}.l1d_pref prefetcher/l1d_prefetcher.cc
cp prefetcher/${L2C_PREFETCHER}.l2c_pref prefetcher/l2c_prefetcher.cc

# Librarify
cd lib
./librarify.sh
cd ..

# Sanity check
echo ""
if [ ! -f lib/champsim.a ]; then
    echo "${BOLD}ChampSim CRC-2 build FAILED!${NORMAL}"
    echo ""
    exit
fi

echo "${BOLD}ChampSim CRC-2 library is successfully built"
echo "L1D prefetcher: ${L1D_PREFETCHER}"
echo "L2C prefetcher: ${L2C_PREFETCHER}"
echo "Cores: ${NUM_CORE}"
LIBRARY_NAME=$4
echo "CRC-2 Library: lib/${LIBRARY_NAME}.a ${NORMAL}"
echo ""
cp lib/champsim.a ChampSim_CRC2/lib/${LIBRARY_NAME}.a

# Restore to the default configuration
sed -i.bak 's/\<NUM_CPUS '${NUM_CORE}'\>/NUM_CPUS 1/g' inc/champsim.h
sed -i.bak 's/\<DRAM_CHANNELS 2\>/DRAM_CHANNELS 1/g' inc/champsim.h
sed -i.bak 's/\<DRAM_CHANNELS_LOG2 1\>/DRAM_CHANNELS_LOG2 0/g' inc/champsim.h

cp prefetcher/no.l1d_pref prefetcher/l1d_prefetcher.cc
cp prefetcher/no.l2c_pref prefetcher/l2c_prefetcher.cc
