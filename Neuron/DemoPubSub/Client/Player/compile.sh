gcc main.c decode.c display.c thread_shared_data.c ../x264_vfr_module/x264_vfr_module.c ../x264_vfr_module/x264_buffer_parser.c ../../DDS/*.c -g -I$OSPL_HOME/include/dcps/C/SAC -L$OSPL_HOME/lib -o xvdplay -ldcpssac -lavcodec -lavformat -lswscale `sdl-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0`

