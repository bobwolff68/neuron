cd ../../../media/entity_info_idl/
rtiddsgen -replace -namespace entityinfo.idl
cd ../idl
rtiddsgen -replace -namespace media.idl
cd ../../entities/test/entinfo_discovery/rp_standalone
./build.sh
cd ../h264source_standalone
./build.sh

cd ../
rm CMakeCache.txt
cmake -G "Unix Makefiles"
make clean
make

cp USER_QOS_PROFILES.xml ~/

