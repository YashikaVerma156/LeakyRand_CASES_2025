This directory contains the code that is used to mimick I/O access and invalidations. A new ChampSim tracer code is written for this as a new field is added in the instruction format to signify when to start counting the number of instructions and cycles and when to stop.

1. To look for added code of IO\_invalidations program, use the tag. //@IO\_INVALIDATIONS
2. Use invalidation\_program.sh to run the experiments.
3. Results prepared and used are generated using the below scripts. Result files are pushed to the backup server yashikav@backup1.cse.iitk.ac.in at path /data/yashikav/IO\_calls\_invalidations\_results.
a. extracted\_result.txt
b. extracted\_result\_llc\_misses.txt
