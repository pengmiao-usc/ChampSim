mkdir -p ChampSim_CRC2/lib
sed -i.bak 's/NO_CRC2_COMPILE/CRC2_COMPILE/g' inc/champsim.h

# Configuratoin 1: no prefetchers, single-core
sed -i.bak 's/NUM_CONFIG 1/NUM_CONFIG 1/g' inc/champsim.h
./librarify_crc2.sh no no 1 config1

# Configuratoin 2: L1D next-line prefetcher, L2C IP-based stride prefetcher, single-core
sed -i.bak 's/NUM_CONFIG 1/NUM_CONFIG 2/g' inc/champsim.h
./librarify_crc2.sh next_line ip_stride 1 config2

# Configuratoin 3: no prefetchers, multi-core
sed -i.bak 's/NUM_CONFIG 2/NUM_CONFIG 3/g' inc/champsim.h
./librarify_crc2.sh no no 4 config3

# Configuratoin 4: L1D next-line prefetcher, L2C IP-based stride prefetcher, single-core
sed -i.bak 's/NUM_CONFIG 3/NUM_CONFIG 4/g' inc/champsim.h
./librarify_crc2.sh next_line ip_stride 4 config4

# Configuratoin 5: no prefetchers, single-core, 8MB LLC
sed -i.bak 's/NUM_CONFIG 4/NUM_CONFIG 5/g' inc/champsim.h
sed -i.bak 's/LLC_SET NUM_CPUS/LLC_SET 4/g' inc/cache.h
./librarify_crc2.sh no no 1 config5

# Configuratoin 6: L1D next-line prefetcher, L2C IP-based stride prefetcher, single-core, 8MB LLC
sed -i.bak 's/NUM_CONFIG 5/NUM_CONFIG 6/g' inc/champsim.h
./librarify_crc2.sh next_line ip_stride 1 config6

sed -i.bak 's/LLC_SET 4/LLC_SET NUM_CPUS/g' inc/cache.h
sed -i.bak 's/NUM_CONFIG 6/NUM_CONFIG 1/g' inc/champsim.h
sed -i.bak 's/CRC2_COMPILE/NO_CRC2_COMPILE/g' inc/champsim.h

cp inc/champsim_crc2.h ChampSim_CRC2/inc/
