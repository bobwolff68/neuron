# Opensplice version
# gcc main.c decode.c display.c thread_shared_data.c ../x264_vfr_module/x264_vfr_module.c ../x264_vfr_module/x264_buffer_parser.c ../../DDS/*.c -g -I$OSPL_HOME/include/dcps/C/SAC -L$OSPL_HOME/lib -o xvdplay -ldcpssac -lavcodec -lavformat -lswscale `sdl-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0`

# RTI Style

# Regenerate files from IDL only when needed....
# cd ../../DDS;$NDDSHOME/scripts/rtiddsgen NeuronDDS.idl -replace -language C;cd ../Client/Player

# Main compilation / linking of application in RTI style (64 bit)
#gcc main.c decode.c display.c thread_shared_data.c ../x264_vfr_module/x264_vfr_module.c ../x264_vfr_module/x264_buffer_parser.c ../../DDS/*.c -g -DRTI_STYLE -DRTI_UNIX -DRTI_LINUX -DRTI_64BIT -m64 -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/x64Linux2.6gcc4.1.1 -o xvdplay -lavformat -lavcodec -lswscale -lavutil -lnddscd -lnddscored `sdl-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0`

# Main compilation / linking of application in RTI style (32 bit)
gcc main.c decode.c display.c thread_shared_data.c ../x264_vfr_module/x264_vfr_module.c ../x264_vfr_module/x264_buffer_parser.c -g3 -o xvdplay -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -lavformat -lavcodec -lswscale -lavutil -lNeuronVideoPubSub -ldl -lnddsc -lnddscpp -lnddscore `sdl-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0`

#gcc main.c decode.c display.c thread_shared_data.c ../x264_vfr_module/x264_vfr_module.c ../x264_vfr_module/x264_buffer_parser.c ../../DDS/*.c -g -DRTI_STYLE -DRTI_UNIX -DRTI_LINUX -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -o xvdplay -lavformat -lavcodec -lswscale -lavutil -lnddscd -lnddscored `sdl-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0`

